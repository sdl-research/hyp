// Copyright 2014-2015 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/** \file

    compose an fst with a cfg or fst. (see fs/compose for fst with fst, lazily)
*/

#ifndef HYP__HYPERGRAPH_COMPOSE_HPP
#define HYP__HYPERGRAPH_COMPOSE_HPP
#pragma once

#include <utility>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <utility>
#include <boost/range/algorithm/lower_bound.hpp>
#include <boost/unordered_set.hpp>
#include <tuple>

#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <utility>
#include <boost/range/algorithm/lower_bound.hpp>
#include <boost/unordered_set.hpp>

#include <sdl/IVocabulary.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>

#include <sdl/Hypergraph/WeightsFwdDecls.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/Properties.hpp>
#include <sdl/Hypergraph/HypergraphWriter.hpp>
#include <sdl/Hypergraph/Empty.hpp>
#include <sdl/Hypergraph/SortArcs.hpp>
#include <sdl/Hypergraph/ArcParserFct.hpp>
#include <sdl/Hypergraph/Transform.hpp>
#include <sdl/Hypergraph/HypergraphCopyBasic.hpp>
#include <sdl/Hypergraph/fs/Compose.hpp>
#include <sdl/Hypergraph/FeatureWeightUtil.hpp>

#include <sdl/Util/Compare.hpp>
#include <sdl/Util/Input.hpp>


#include <sdl/Util/ThreadSpecific.hpp>
#include <sdl/Util/Hash.hpp>
#include <sdl/Util/Unordered.hpp>
#include <sdl/Util/NonNullPointee.hpp>
#include <sdl/Util/IsDebugBuild.hpp>
#include <sdl/Util/PrintRange.hpp>
#include <sdl/Hypergraph/Assert.hpp>
#include <sdl/Hypergraph/WeightUtil.hpp>

#ifndef SDL_HASH_COMPOSE_ITEM
#define SDL_HASH_COMPOSE_ITEM 0
// TODO: choose the best setting in a realistic context, maybe in conjunction
// with the cheaper ptr_vector<nullable<Item> > or shared_ptr<vector<Item> >
// since unordered_set copies its keys
#endif

#if SDL_HASH_COMPOSE_ITEM
#include <sdl/Util/OwnedRegistry.hpp>
#include <sdl/Hypergraph/LabelPair.hpp>
#include <sdl/Util/Map.hpp>
#include <graehl/shared/int_types.hpp>
#include <sdl/Util/PointerSet.hpp>
#endif

namespace sdl {
namespace Hypergraph {

static const ArcId fakeArcId = (ArcId)-1;
/**
   Used as a functor that compares arcs' input labels with a
   search label. Used in binary search, where we search for a label,
   where we pretend that label is on an arc with a fake ArcId.
*/
template <class A>
struct CmpInputLabelWithSearchLabel {

  CmpInputLabelWithSearchLabel(IHypergraph<A> const& fst, StateId s, Sym searchLabel = NoSymbol)
      : fst_(fst), tail_(s), searchLabel_(searchLabel) {}

  bool operator()(ArcId a, ArcId b) const {
    if (a == fakeArcId) {
      A* otherArc = fst_.outArc(tail_, b);
      return searchLabel_ < fst_.inputLabel((otherArc->tails_[1]));
    } else {
      A* otherArc = fst_.outArc(tail_, a);
      return searchLabel_ > fst_.inputLabel((otherArc->tails_[1]));
    }
  }

  IHypergraph<A> const& fst_;
  StateId tail_;
  Sym searchLabel_;
};

// TODO: try Index.hpp registry, which uses hashing.
template <class T>
class Registry {
  // TODO: use unordered_set
  typedef std::set<T*, Util::LessByValue<T>> TSet;
  TSet set_;

 public:
  ~Registry() {
    for (T* item : set_) {
      delete item;
    }
  }

  /// delete p if it was already in set; return version in set.
  T* insert(T* p) {
    std::pair<typename TSet::iterator, bool> iNew = set_.insert(p);
    if (iNew.second)
      return p;
    else {
      delete p;
      return *iNew.first;
    }
  }
};

// only used in debug mode, for nicer output
template <class T>
class RegistryWithIds : public Registry<T> {
  typedef std::map<T*, std::size_t> TsToIdsMap;
  std::size_t nextId_;
  TsToIdsMap ids_;

 public:
  RegistryWithIds() : nextId_(1) {}

  /**
     Must already be the unique pointer, as created by insert.
  */
  std::size_t getId(T* t) {
    // T* uniquePtr = insert(t);
    typename TsToIdsMap::const_iterator found = ids_.find(t);
    if (found == ids_.end()) {
      std::size_t id = nextId_;
      ids_[t] = id;
      ++nextId_;
      return id;
    }
    return found->second;
  }
};

struct EarleyParserOptions {
  bool enablePhiRhoMatch;
  bool sigmaPreventsPhiRhoMatch;

  explicit EarleyParserOptions() : enablePhiRhoMatch(true), sigmaPreventsPhiRhoMatch(false) {}
};


// Does matching an eps (or sigma) arc prevent matching a phi or rho
// arc? (phi and rho mean "match otherwise") This should usually not
// be the case. E.g., we want to match 'x' or else, transition to
// failure state (phi); or (independent of that) transition to final
// state (eps).
// const boost::uint32_t kEarleyEpsPreventsPhiRhoMatch =   0x00000002;

template <class A>
struct EarleyParser {
  typedef A Arc;
  typedef typename Arc::Weight Weight;
  struct Item {
    Item()
        : from(kNoState)
        , to(kNoState)
        , arc(NULL)
        , dotPos(0)
        , agendaWeight(HUGE_VAL)
        , chartWeight(HUGE_VAL)
        , lastWasPhiOrEps(false) {}

    Item(StateId from_, StateId to_, Arc* arc_, TailId dotPos_ = 0)
        : from(from_)
        , to(to_)
        , arc(arc_)
        , dotPos(dotPos_)
        , agendaWeight(HUGE_VAL)
        , chartWeight(HUGE_VAL)
        , lastWasPhiOrEps(false) {}

    Item(StateId from_, StateId to_, Arc* arc_, TailId dotPos_, bool lastWasPhiOrEps_)
        : from(from_)
        , to(to_)
        , arc(arc_)
        , dotPos(dotPos_)
        , agendaWeight(HUGE_VAL)
        , chartWeight(HUGE_VAL)
        , lastWasPhiOrEps(lastWasPhiOrEps_) {}

    bool isComplete() const { return dotPos == arc->tails_.size(); }

    bool operator<(Item const& other) const {
      if (from != other.from)
        return from < other.from;
      else if (to != other.to)
        return to < other.to;
      else if (arc != other.arc)
        return arc < other.arc;
      else if (dotPos != other.dotPos)
        return dotPos < other.dotPos;
      return lastWasPhiOrEps < other.lastWasPhiOrEps;
    }

    bool operator==(Item const& o) const {
      return from == o.from && to == o.to && arc == o.arc && dotPos == o.dotPos
             && lastWasPhiOrEps == o.lastWasPhiOrEps;
    }

    std::size_t hash() const {
      return Util::mixedbits((uint64)(dotPos + ((uint64)from << 2) + ((uint64)to << 21) + ((uint64)arc >> 5)));
    }
    friend inline std::size_t hash_value(Item const& self) { return self.hash(); }

    StateId from;
    StateId to;
    Arc* arc;
    TailId dotPos;
    Weight agendaWeight;
    Weight chartWeight;
    bool lastWasPhiOrEps;  // did we create this item by using a phi
    // transition? in that case, we can say we have
    // something lexical right before the dot (we
    // traversed an FST arc)
  };

// TODO: use unordered_set
#if SDL_HASH_COMPOSE_ITEM
  typedef unordered_set<Item*, Util::NonNullPointeeHash<Item>, Util::NonNullPointeeEqual> ItemsSet;
#else
  typedef std::set<Item*> ItemsSet;
#endif
  typedef uint64 ItemPriority;

  struct ItemPriorityMap {
    typedef Item* key_type;
    typedef ItemPriority value_type;
    typedef ItemPriority reference;
    typedef boost::readable_property_map_tag category;
    friend inline ItemPriority get(ItemPriorityMap const&, Item* key) {
      return (((ItemPriority)key->from << 32) + key->to);
    }
    ItemPriority const& operator[](Item* key) const { return get(*this, key); }
  };

  // at each dot position in a CFG rule item, a possible sequence of
  // FST arcs that matched there (it's sequence b/c of eps)
  typedef std::vector<std::vector<Arc*>> ArcVecPerDotPos;
  typedef shared_ptr<ArcVecPerDotPos> ArcVecPerDotPosPtr;


  /**
     Groups an item (i.e., CFG rule and dot position) with the
     FST arcs that have matched up to the dot position.
  */
  struct ItemAndMatchedArcs {
    ItemAndMatchedArcs(Item* i, ArcVecPerDotPosPtr s, StateId h, StateIdContainer const& t)
        : item(i), stateIds(s), head(h), tails(t) {
      hashCode = (std::size_t)(item - (Item*)0);
      boost::hash_combine(hashCode, h);
      boost::hash_range(hashCode, stateIds->begin(), stateIds->end());
      boost::hash_range(hashCode, tails.begin(), tails.end());
    }

    bool operator==(ItemAndMatchedArcs const& other) const {
      return other.item == item && other.head == head && *other.stateIds == *stateIds && other.tails == tails;
    }
    bool operator<(ItemAndMatchedArcs const& other) const {
      if (item != other.item)
        return item < other.item;
      else if (head != other.head)
        return head < other.head;
      else if (*stateIds != *other.stateIds)
        return *stateIds < *other.stateIds;
      else
        return tails < other.tails;
    }

    Item* item;
    const ArcVecPerDotPosPtr stateIds;
    std::size_t hashCode;
    StateId head;
    StateIdContainer tails;  // TODO: too expensive?
  };

  struct ItemAndMatchedArcsHashFct {
    std::size_t operator()(ItemAndMatchedArcs* obj) const { return obj->hashCode; }
  };

  typedef unordered_set<ItemAndMatchedArcs*, ItemAndMatchedArcsHashFct, Util::NonNullPointeeEqualExpensive>
      ItemAndMatchedArcsSet;
  IHypergraph<A> const& fst_;
  IHypergraph<A> const& cfg_;

 private:
  IMutableHypergraph<A>* result_;

  // map from head to matching items
  std::vector<std::map<StateId, ItemsSet>> itemsTo_;
  std::vector<std::map<StateId, ItemsSet>> completeItemsFrom_;
// TODO: ptr_vector to avoid expensive copies

#if SDL_IS_DEBUG_BUILD
  // For nicer debug output:
  typedef RegistryWithIds<Item> ItemRegistry;
#elif SDL_HASH_COMPOSE_ITEM
  typedef Util::OwnedRegistry<Item> ItemRegistry;
#else
  typedef Registry<Item> ItemRegistry;
#endif

  ItemRegistry registry_;
  // foreach position and CFG state
  std::vector<std::set<StateId>> alreadyPredicted_;

#if 1
  std::queue<Item*> agenda_;
#else
  /// work on leftmost spans first? (std::less) - see Hypergraph2/regtest-compose3.yml
  Util::priority_queue<std::vector<Item*>, 4, ItemPriorityMap, std::greater<ItemPriority>> agenda_;
#endif
  // TODO: maybe std::priority_queue w/ old comparison object (no ItemPriorityMap) was correct?

  /// queue (no priority): bad. less: bad. greater: bad. problem w/ HypCompose props?
  std::set<Item*> finalItems_;

  typedef std::pair<Item*, Item*> BackPointer;
  struct BackPointerHash  {
    static constexpr std::size_t ceil_log2_size = graehl::ceil_log2_const(sizeof(Item));
    std::size_t operator()(BackPointer bp) const {
      return (((std::size_t)bp.first << 12) ^ (std::size_t)bp.second ^ (std::size_t)bp.first) >> ceil_log2_size;
    }
  };

  //  typedef unordered_set<BackPointer, BackPointerHash, std::equal_to<BackPointer>> BackPointers;
  typedef std::set<BackPointer> BackPointers;
  typedef Util::pointer_unordered_map<Item, BackPointers> BackPointerMap;
  BackPointerMap backPointers_;

  typedef std::tuple<StateId, std::size_t, std::size_t> Triple;
  // TODO: use unordered_map or some custom open hash table
  // states (in the result machine) of (cfg-state, from, to) triples
  typedef std::map<Triple, StateId> TripleToResultStateMap;
  TripleToResultStateMap tripleToResultState_;


  typedef std::map<LabelPair, StateId> LabelPairToStateId;
  LabelPairToStateId resultLabelPairToStateId_;

  // A CFG arc "s <- / 0" that we add as backpointer for the start of
  // the CFG
  Arc* cfgPseudoStartArc_;

  EarleyParserOptions options_;
  IVocabularyPtr pVoc_;

 public:
  EarleyParser(IHypergraph<A> const& cfg, IHypergraph<A> const& fst, IMutableHypergraph<A>* resultCfg,
               EarleyParserOptions opts = EarleyParserOptions())
      : fst_(fst), cfg_(cfg), result_(resultCfg), cfgPseudoStartArc_((A*)NULL), options_(opts) {
    pVoc_ = fst.getVocabulary();
  }

  ~EarleyParser() { delete cfgPseudoStartArc_; }

  typedef TailId DotPos;


  /**
     \return Pointer to unique item
  */
  Item* createItem(StateId from, StateId to, Arc* arc, TailId dotPos = 0, bool lastWasPhiOrEps = false) {
    return registry_.insert(new Item(from, to, arc, dotPos, lastWasPhiOrEps));
  }

  std::ostream& writeItem(std::ostream& out, Item* item, IHypergraph<A> const& hg) {
    out << "[";
    if (Util::isDebugBuild()) {
      out << "#" << registry_.getId(item);
      out << ", ";
    }
    out << (item->from == kNoState ? -1 : item->from) << ".." << (item->to == kNoState ? -1 : item->to)
        << ", [";
    printArc(out, item->arc, &hg);
    out << "], dot=" << item->dotPos << ", lastEpsPhi=" << (item->lastWasPhiOrEps ? "y" : "n")
        << ", agendaw=" << item->agendaWeight << ", chartw=" << item->chartWeight << "]";
    return out;
  }


  // TODO: allow improvements only. Also, what if item had already been derived
  // and is in the chart now, with better weight? see BestPath 'rereach' options
  // for an example
  void pushAgendaItem(Item* item, Weight agendaWeight) {
    Hypergraph::removeFeatures(agendaWeight);
    if (isZero(item->agendaWeight)) {
      item->agendaWeight = agendaWeight;
      agenda_.push(item);
    } else {
      plusBy(agendaWeight, item->agendaWeight);
    }
  }

  void init() {
    StateId s = cfg_.final();
    StateId from = fst_.start();
    for (ArcId arcid : cfg_.inArcIds(s)) {
      Arc* arc = cfg_.inArc(s, arcid);
      // Item* item = createItem(from, from, arc, 0, arc->weight());
      Item* item = createItem(from, from, arc, 0);
      pushAgendaItem(item, arc->weight());
    }
    alreadyPredicted_.resize(from + 1);
    alreadyPredicted_[from].insert(s);
  }

  void predict(Item* item) {
    StateId s = item->arc->tails_[item->dotPos];

    if (s == cfg_.start()) {
      // automatically derive start state
      Item* newItem = createItem(item->from, item->to, item->arc, item->dotPos + 1);
      pushAgendaItem(newItem, item->chartWeight);

      // Backpointer is a pseudo item that derives the start state
      if (cfgPseudoStartArc_ == NULL) {
        cfgPseudoStartArc_ = new Arc();
        cfgPseudoStartArc_->setHead(cfg_.start());
      }
      Item* startItem = createItem(cfg_.start(), cfg_.start(), cfgPseudoStartArc_, 0);
      backPointers_[newItem].insert(BackPointer(item, startItem));
      return;
    }

    StateId from = item->to;
    if (alreadyPredicted_.size() > from && alreadyPredicted_[from].find(s) != alreadyPredicted_[from].end()) {
      return;
    }
    for (ArcId arcid : cfg_.inArcIds(s)) {
      Arc* arc = cfg_.inArc(s, arcid);
      StateId to = item->to;
      Item* newItem = createItem(from, to, arc, 0);
      pushAgendaItem(newItem, arc->weight());
    }
    if (alreadyPredicted_.size() <= from) {
      alreadyPredicted_.resize(from + 1);
    }
    alreadyPredicted_[from].insert(s);
  }

  /**
     Attempts to match eps at the current FST state.
  */
  void scanEps(Item* item) {
    // Only attach a found eps after scanning something else (low in
    // the tree), not after some nonterminal
    if (item->dotPos > 0) {
      StateId mostRecentlyCompleted = item->arc->getTail(item->dotPos - 1);
      bool mostRecentlyCompletedWasLexical = cfg_.outputLabel(mostRecentlyCompleted).isTerminal();
      if ((!mostRecentlyCompletedWasLexical && !item->lastWasPhiOrEps)
          && mostRecentlyCompleted != cfg_.start()) {
        return;
      }
    } else if (item->dotPos == 0 && item->arc->getTail(0) == cfg_.start()) {
      return;
    }
    // Don't attach eps right before a non-lexical nontermial (will
    // instead attach inse that nonterminal)
    if (!item->isComplete()) {
      StateId next = item->arc->tails_[item->dotPos];
      if (!cfg_.outputLabel(next).isTerminal()) {
        return;
      }
    }
    StateId s = item->to;
    for (ArcId arcid : fst_.outArcIds(s)) {
      Arc* fstArc = fst_.outArc(s, arcid);
      Sym label = fst_.inputLabel(fstArc->tails_[1]);
      if (label == EPSILON::ID) {
        Weight w = times(item->chartWeight, fstArc->weight());
        Item* newItem = createItem(item->from, fstArc->head_, item->arc, item->dotPos, true);
        // newItem->lastWasPhiOrEps = true;
        pushAgendaItem(newItem, w);
        backPointers_[newItem].insert(BackPointer(item, createItem(kNoState, kNoState, fstArc, 1)));
      } else
        return;  // all following labels are non-eps
    }
  }

  /**
     Attempts to match a label at the current FST state.
  */
  void scan(Item* item) {
    Sym searchLabel = cfg_.outputLabel(item->arc->tails_[item->dotPos]);
    if (searchLabel == EPSILON::ID) {  // the CFG has an eps
      Item* newItem = createItem(item->from, item->to, item->arc, item->dotPos + 1);
      pushAgendaItem(newItem, item->chartWeight);
      backPointers_[newItem].insert(BackPointer(item, createItem(kNoState, kNoState, cfgPseudoStartArc_, 0)));
      return;
    } else if (searchLabel == PHI::ID || searchLabel == RHO::ID || searchLabel == SIGMA::ID) {
      SDL_THROW_LOG(Hypergraph, InvalidInputException,
                    "Currently, special symbols <phi>, <rho>, <sigma> can only be used in "
                    "the 2nd argument to compose.");
    }
    StateId s = item->to;

    std::size_t numMatches = 0;

    // store first arc that imposes some match condition (i.e., not
    // eps or sigma)
    ArcId firstConditionalArcId = kNoArc;

    // Iterate through first arcs to see if we have unconditional
    // matches (i.e., eps or sigma)
    for (ArcId arcid : fst_.outArcIds(s)) {
      Arc* fstArc = fst_.outArc(s, arcid);
      Sym foundLabel = fst_.inputLabel(fstArc->tails_[1]);
      if (foundLabel == EPSILON::ID) {
        continue;  // scanEps is a separate function
      } else if (foundLabel == SIGMA::ID) {
        // match unconditionally and consume
        Item* newItem = createItem(item->from, fstArc->head_, item->arc, item->dotPos + 1);
        pushAgendaItem(newItem, times(item->chartWeight, fstArc->weight()));
        backPointers_[newItem].insert(BackPointer(item, createItem(kNoState, kNoState, fstArc, 1)));
        if (options_.sigmaPreventsPhiRhoMatch) ++numMatches;
      } else {
        firstConditionalArcId = arcid;
        break;
      }
    }

    // Search for search label using binary search
    ArcIdRange arcIdsRange = fst_.outArcIds(s);
    // binary search:
    ArcIdIterator arcEnd = boost::end(arcIdsRange);
    ArcIdIterator matchingArcIdsIter
        = boost::lower_bound(arcIdsRange, fakeArcId, CmpInputLabelWithSearchLabel<A>(fst_, s, searchLabel));
    for (; matchingArcIdsIter != arcEnd; ++matchingArcIdsIter, ++numMatches) {
      Arc* matchingArc = fst_.outArc(s, *matchingArcIdsIter);
      if (fst_.inputLabel(matchingArc->tails_[1]) != searchLabel) {
        break;
      }
      Item* newItem = createItem(item->from, matchingArc->head_, item->arc, item->dotPos + 1);
      pushAgendaItem(newItem, times(item->chartWeight, matchingArc->weight()));
      backPointers_[newItem].insert(BackPointer(item, createItem(kNoState, kNoState, matchingArc, 1)));
    }

    // Look for phi or rho
    if (numMatches == 0 && firstConditionalArcId != kNoArc && (options_.enablePhiRhoMatch)) {
      ArcIdRange arcIdsRange = fst_.outArcIds(s);
      for (ArcIdIterator aiter = ArcIdIterator(firstConditionalArcId), arcEnd = boost::end(arcIdsRange);
           aiter != arcEnd; ++aiter) {
        Arc* arc = fst_.outArc(s, *aiter);
        Item* fstArcItem = createItem(kNoState, kNoState, arc, 1);
        if (fst_.inputLabel(arc->tails_[1]) == PHI::ID) {
          // non-consuming
          Item* newItem = createItem(item->from, arc->head_, item->arc, item->dotPos, true);
          // newItem->lastWasPhiOrEps = true;
          pushAgendaItem(newItem, times(item->chartWeight, arc->weight()));
          backPointers_[newItem].insert(BackPointer(item, fstArcItem));
        } else if (fst_.inputLabel(arc->tails_[1]) == RHO::ID) {
          // consuming
          Item* newItem = createItem(item->from, arc->head_, item->arc, item->dotPos + 1);
          pushAgendaItem(newItem, times(item->chartWeight, arc->weight()));
          backPointers_[newItem].insert(BackPointer(item, fstArcItem));
        }
      }
    }
  }

  /**
     We have a complete item j..k and attach it to an
     incomplete one i..j.
  */
  void complete(Item* item) {
    if (itemsTo_.size() > item->from) {
      StateId head = item->arc->head_;
      for (Item* oldItem : itemsTo_[item->from][head]) {
        if (!oldItem->isComplete()) {
          assert(oldItem->arc->getTail(oldItem->dotPos) == head);
          Item* newItem = createItem(oldItem->from, item->to, oldItem->arc, oldItem->dotPos + 1);
          backPointers_[newItem].insert(BackPointer(oldItem, item));
          pushAgendaItem(newItem, times(item->chartWeight, oldItem->chartWeight));
        }
      }
    }
  }

  /**
     Part of the complete operation, where we have an
     incomplete item i..j and try to find a complete one j..k.
  */
  void findComplete(Item* item) {
    if (completeItemsFrom_.size() > item->to) {
      StateId nextTail = item->arc->tails_[item->dotPos];
      for (Item* completeItem : completeItemsFrom_[item->to][nextTail]) {
        if (completeItem->arc->head_ == nextTail) {
          Item* newItem = createItem(item->from, completeItem->to, item->arc, item->dotPos + 1);
          backPointers_[newItem].insert(BackPointer(item, completeItem));
          pushAgendaItem(newItem, times(item->chartWeight, completeItem->chartWeight));
        }
      }
    }
  }

  StateId addLexicalState(Sym ilabel, Sym olabel) { return addLexicalState(LabelPair(ilabel, olabel)); }

  StateId addLexicalState(LabelPair label);


  StateId createTerminalState(BackPointer bp);

  void backtrace(std::string name, Item* item, std::size_t recursion) {
    std::stringstream blanks;
    for (std::size_t i = 0; i < recursion; ++i) {
      blanks << ".";
    }
    std::cerr << blanks.str() << name;
    const bool itemMeansEpsScan = item->arc == NULL;
    if (itemMeansEpsScan) {
      std::cerr << "eps\n";
      ;
    }
    typename BackPointerMap::const_iterator found = backPointers_.find(item);
    if (found != backPointers_.end()) {
      for (BackPointer bp : found->second) {
        backtrace("(A) ", bp.first, recursion + 1);
        if (bp.second != NULL) {
          backtrace("(B) ", bp.second, recursion + 1);
        }
      }
    }
  }

  void backtrace(Item* item) { backtrace("", item, 0); }

  void backtrace2(Item* item, std::size_t recursion) {
    typename BackPointerMap::const_iterator found = backPointers_.find(item);
    if (found != backPointers_.end()) {
      for (BackPointer bp : found->second) {
        std::cerr << registry_.getId(item) << "(\"" << registry_.getId(item) << "\")";
        std::cerr << " <- ";
        std::cerr << registry_.getId(bp.first) << "(\"" << registry_.getId(bp.first) << "\") ";
        if (bp.second != NULL) {
          std::cerr << registry_.getId(bp.second) << "(\"" << registry_.getId(bp.second) << "\")";
        }
        std::cerr << '\n';
        backtrace2(bp.first, recursion + 1);
        if (bp.second != NULL) {
          backtrace2(bp.second, recursion + 1);
        }
      }
    }
  }

  void backtrace2(Item* item) {
    std::cerr << "FINAL <- " << registry_.getId(item) << "(\"" << registry_.getId(item) << "\")\n";
    backtrace2(item, 0);
  }

  StateId getResultCfgState(StateId inputCfgState, std::size_t from, std::size_t to) {
    TripleToResultStateMap::const_iterator found = tripleToResultState_.find(Triple(inputCfgState, from, to));
    StateId resultId = kNoState;
    if (found == tripleToResultState_.end()) {
      resultId = result_->addState(cfg_.outputLabel(inputCfgState), cfg_.outputLabel(inputCfgState));
      tripleToResultState_[Triple(inputCfgState, from, to)] = resultId;
    } else {
      resultId = found->second;
    }
    return resultId;
  }

  void createResultArcs1(ItemAndMatchedArcs* itemAndMatchedArcs, StateId head, ArcVecPerDotPosPtr matchedArcs,
                         StateIdContainer const& tails, Weight w, ItemAndMatchedArcsSet* alreadyExpanded);

  void createResultArcs(Item* item, StateId head, ItemAndMatchedArcsSet* alreadyExpanded);

  /**
     Creates result CFG by following the back pointers of the
     final items.
  */
  void createResultCfg() {
    SDL_TRACE(Hypergraph.Compose, "createResultCfg");
    ItemAndMatchedArcsSet alreadyExpanded;
    for (Item* item : finalItems_) {
      StateId head = getResultCfgState(item->arc->head_, item->from, item->to);
      result_->setFinal(head);
      createResultArcs(item, head, &alreadyExpanded);
    }
    for (ItemAndMatchedArcs* p : alreadyExpanded) {
      delete p;
    }
  }

  IMutableHypergraph<A>* parse() {
    buildChart();
    createResultCfg();
    return result_;
  }

  void addConsequentsToAgenda(Item* item) {
    scanEps(item);
    if (item->isComplete()) {
      complete(item);
    } else {
      StateId nextTail = item->arc->tails_[item->dotPos];
      if (cfg_.outputLabel(nextTail).isTerminal()) {
        scan(item);
      } else {
        predict(item);
        findComplete(item);
      }
    }
  }

  void buildChart() {
    using namespace Util;
    SDL_TRACE(Hypergraph.Compose, "buildChart");
    init();
    while (!agenda_.empty()) {
      Item* item = agenda_.front();
      agenda_.pop();
      Weight agendaWeight = item->agendaWeight;
      setZero(item->agendaWeight);
      Weight oldChartWeight = item->chartWeight;
      plusBy(agendaWeight, item->chartWeight);
      // Enter into chart, unless already there
      if (isZero(oldChartWeight)) {
        const bool isComplete = item->isComplete();
        const bool isFinalReached = isComplete && cfg_.final() == item->arc->head_
                                    && fst_.start() == item->from && fst_.final() == item->to;
        if (isComplete) {
          atExpand(completeItemsFrom_, item->from)[item->arc->head_].insert(item);
        } else {
          atExpand(itemsTo_, item->to)[item->arc->getTail((TailId)item->dotPos)].insert(item);
        }
        if (isFinalReached) {
          finalItems_.insert(item);
        }
      }
      if (item->chartWeight != oldChartWeight) {
        addConsequentsToAgenda(item);
      }
    }
  }
};

/// Functions that create the result Hypergraph from the chart:

/**
   Adds lexical state in the result CFG (unique per label pair)
*/
template <class A>
StateId EarleyParser<A>::addLexicalState(LabelPair label) {
  StateId* s;
  if (Util::update(resultLabelPairToStateId_, label, s))
    return * s = result_->addState(label);
  else
    return *s;
}

/**
   Creates lexical state (one that carries a lexical label) in
   the result CFG.
*/
template <class A>
StateId EarleyParser<A>::createTerminalState(BackPointer bp) {

  // CFG has an eps at that position
  if (bp.second->arc == cfgPseudoStartArc_) {
    StateId cfgLabelState = bp.first->arc->getTail(bp.first->dotPos);
    Sym cfgOutputLabel = cfg_.outputLabel(cfgLabelState);
    assert(cfgOutputLabel == EPSILON::ID);
    return addLexicalState(cfgOutputLabel, cfgOutputLabel);
  }

  StateId s;
  StateId fstLabelState = bp.second->arc->tails_[1];
  Sym fstInputLabel = fst_.inputLabel(fstLabelState);
  const bool isNonconsuming = fstInputLabel == EPSILON::ID || fstInputLabel == PHI::ID;
  if (isNonconsuming) {
    Sym fstOutputLabel = fst_.outputLabel(fstLabelState);
    s = addLexicalState(EPSILON::ID, fstOutputLabel);
  } else {
    StateId cfgLabelState = bp.first->arc->getTail(bp.first->dotPos);
    // Sym cfgOutputLabel =
    assert(cfg_.outputLabel(cfgLabelState) != EPSILON::ID);  // dealt with above

    Sym cfgInputLabel = cfg_.inputLabel(cfgLabelState);
    Sym fstInputLabel = fst_.inputLabel(fstLabelState);
    Sym fstOutputLabel = fst_.outputLabel(fstLabelState);
    // rewrite rho/sigma input to the actual matched CFG label
    if ((fstInputLabel == RHO::ID && fstOutputLabel == RHO::ID)
        || (fstInputLabel == SIGMA::ID && fstOutputLabel == SIGMA::ID)) {
      s = addLexicalState(cfgInputLabel, cfgInputLabel);
    } else {
      s = addLexicalState(cfgInputLabel, fstOutputLabel);
    }
  }
  return s;
}

template <class Arc>
void EarleyParser<Arc>::createResultArcs1(ItemAndMatchedArcs* itemAndMatchedArcs, StateId head,
                                          ArcVecPerDotPosPtr matchedArcs, StateIdContainer const& tails,
                                          Weight w, ItemAndMatchedArcsSet* alreadyExpanded) {
  SDL_TRACE(Hypergraph.Compose, "createResultArcs1 for head=" << head
                                    << ", tails=" << printer(tails)
                                    << ", matchedArcs.size()=" << itemAndMatchedArcs->stateIds->size());
  using namespace Util;
  typename ItemAndMatchedArcsSet::const_iterator foundItem = alreadyExpanded->find(itemAndMatchedArcs);
  if (foundItem != alreadyExpanded->end()) {
    SDL_TRACE(Hypergraph.Compose, "Ignore: arc already created");
    delete itemAndMatchedArcs;
    return;
  }

  Item* item = itemAndMatchedArcs->item;
  alreadyExpanded->insert(itemAndMatchedArcs);

  typename BackPointerMap::const_iterator found = backPointers_.find(item);
  if (found != backPointers_.end()) {
    for (BackPointer bp : backPointers_[item]) {
      Item* complete = bp.second;
      const bool isLexical = (complete->from == kNoState);
      StateId rightmost;
      Weight fstWeight = Weight::one();
      if (isLexical) {
        rightmost = createTerminalState(bp);
        fstWeight = complete->arc->weight();
      } else {
        rightmost = getResultCfgState(complete->arc->head_, complete->from, complete->to);
      }
      StateIdContainer tails2(tails.begin(), tails.end());
      tails2.push_back(rightmost);
      Weight prod = times(w, fstWeight);
      ArcVecPerDotPosPtr matchedArcs2(
          new std::vector<std::vector<Arc*>>(matchedArcs->begin(), matchedArcs->end()));
      if (isLexical) {  // store the lexical FST arcs that matched the item
        atExpand(*matchedArcs2, item->dotPos).push_back(complete->arc);
      }
      ItemAndMatchedArcs* irts = new ItemAndMatchedArcs(bp.first, matchedArcs2, head, tails2);
      createResultArcs1(irts, head, matchedArcs2, tails2, prod, alreadyExpanded);
      createResultArcs(bp.second, rightmost, alreadyExpanded);
    }
  } else {  // no backpointer
    if (item->to != kNoState) {
      if (tails.empty())
        result_->setStart(head);
      else
        result_->addArc(
            new Arc(head, StateIdContainer(tails.rbegin(), tails.rend()), times(w, item->arc->weight())));
    }
  }
}

template <class Arc>
void EarleyParser<Arc>::createResultArcs(Item* item, StateId head, ItemAndMatchedArcsSet* alreadyExpanded) {
  StateIdContainer tails;
  ArcVecPerDotPosPtr matchedArcs(new std::vector<std::vector<Arc*>>());
  ItemAndMatchedArcs* irts = new ItemAndMatchedArcs(item, matchedArcs, head, tails);
  return createResultArcs1(irts, head, matchedArcs, tails, Weight::one(), alreadyExpanded);
}

struct ComposeOptions : fs::FstComposeOptions {
  explicit ComposeOptions() {}
  static char const* type() { return "Compose"; }
  static char const* caption() {
    return "CFG*FST or FST*FST compose (for better speed, prune before composing)";
  }
  template <class Configure>
  void configure(Configure const& config) {
    fs::FstComposeOptions::configure(config);
    config.is(type());
    config(caption());
  }
};

namespace {
const Properties kComposeFstRequiredProperties = kFsm | kStoreOutArcs | kSortedOutArcs;
const Properties kComposeCfgRequiredProperties = kStoreInArcs;  // TODO: need out-arcs?
}

template <class A>
void composeImpl(IHypergraph<A> const& cfg, IHypergraph<A> const& fst, IMutableHypergraph<A>* resultCfg,
                 ComposeOptions opts = ComposeOptions()) {
  ASSERT_VALID_HG(cfg);
  ASSERT_VALID_HG(fst);
  bool const ce = empty(cfg);  // actually checks for empty set - linear time
  if (ce || fst.prunedEmpty()) {  // empty(fst)
    SDL_DEBUG(Hypergraph.Compose, "compose(cfg, fst): " << (ce ? "cfg" : "fst")
                                                        << " is empty so result is empty\n");
    resultCfg->setEmpty();
    return;
  }

  if (!fst.isFsm()) {
    SDL_THROW_LOG(Hypergraph, InvalidInputException, "the 2nd of the hypergraphs must be finite-state");
  }

  if (!fst.hasSortedArcs()) {
    SDL_THROW_LOG(Hypergraph, InvalidInputException,
                  "Please sort the outgoing arcs of the FST by input label (with sortArcs)");
  }

  if (!cfg.storesInArcs()) {
    SDL_THROW_LOG(Hypergraph, InvalidInputException, "CFG must store incoming arcs");
  }

  IVocabularyPtr const& pVoc = cfg.getVocabulary();
  if (pVoc != fst.getVocabulary()) {
    SDL_THROW_LOG(Hypergraph, InvalidInputException, "Compose: hypergraphs must have same vocabulary");
  }
  resultCfg->setVocabulary(pVoc);

  SDL_DEBUG(Hypergraph.Compose, "Compose: cfg = \n" << cfg);
  SDL_DEBUG(Hypergraph.Compose, "Compose: fst = \n" << fst);

  EarleyParserOptions earleyOpts;
  EarleyParser<A> p(cfg, fst, resultCfg, earleyOpts);
  p.parse();
  SDL_DEBUG(Hypergraph.Compose, "Compose: cfg*fst = \n" << *resultCfg);

  resultCfg->setEmptyIfNoArcs();
  ASSERT_VALID_HG(*resultCfg);
}

// HgMaybeConst: means maybe you can modify if props are wrong, maybe
// you have to make a copy. both are IHypergraph<A> &
template <class HgMaybeConstCfg, class HgMaybeConstFst, class A>
void compose(HgMaybeConstCfg& cfgIn, HgMaybeConstFst& fstIn, IMutableHypergraph<A>* resultCfg,
             ComposeOptions opts = ComposeOptions(),
             OnMissingProperties onMissing = kModifyOrCopyEnsuringProperties,
             bool const useFstComposeIfPossible = false) {
  typedef IHypergraph<A> H;
  typedef shared_ptr<H const> HP;

  if (!fstIn.isFsm())
    SDL_THROW_LOG(Hypergraph.Compose, NonFsmHypergraphException,
                  "the 2nd of the hypergraphs must be finite-state");
  SDL_DEBUG(Hypergraph.Compose, "compose cfg props pre: " << printProperties(cfgIn));
  typename HgMaybeConstCfg::ConstImmutablePtr pCfgCopy
      = ensureProperties(cfgIn, kComposeCfgRequiredProperties, 0, 0, onMissing);
  SDL_DEBUG(Hypergraph.Compose, "compose cfg props post: " << printProperties(*pCfgCopy));

  SDL_DEBUG(Hypergraph.Compose, "compose fst props pre: " << printProperties(fstIn));

  Properties fstProp = fstIn.properties();
  Properties reqFstProp = kComposeFstRequiredProperties;

  if (useFstComposeIfPossible && (fstProp & kStoreFirstTailOutArcs)) {
    // store first-tail out arc per state or store all arcs out per state.
    reqFstProp |= kStoreFirstTailOutArcs;
    reqFstProp &= ~kStoreOutArcs;
  }

  typename HgMaybeConstFst::ConstImmutablePtr pFstCopy = ensureProperties(fstIn, reqFstProp, 0, 0, onMissing);
  SDL_DEBUG(Hypergraph.Compose, "compose fst props post: " << printProperties(*pFstCopy));

  if (useFstComposeIfPossible && pFstCopy->isMutable() && pCfgCopy->isFsmLike())
    fs::compose(*pCfgCopy, mutableHg(*pFstCopy), resultCfg, opts);
  else
    composeImpl(*pCfgCopy, *pFstCopy, resultCfg, opts);
  SDL_DEBUG(Hypergraph.Compose, "composed: props=" << resultCfg->properties() << "\n" << *resultCfg);
}

template <class HgMaybeConstCfg, class HgMaybeConstFst, class A>
void intersect(HgMaybeConstCfg& hgCfg, HgMaybeConstFst& hgFst, IMutableHypergraph<A>* pHgResult,
               OnMissingProperties onMissing = kModifyOrCopyEnsuringProperties) {
  if (hgCfg.hasOutputLabels() || hgFst.hasOutputLabels()) {
    SDL_THROW_LOG(Hypergraph, InvalidInputException, "intersection is for non-transducing hypergraphs/fsts");
  }
  Hypergraph::compose(hgCfg, hgFst, pHgResult, ComposeOptions(), onMissing);
}

template <class Arc>
struct ComposeTransform;

struct ComposeTransformOptions : ComposeOptions, TransformOptionsBase {
  template <class Arc>
  struct TransformFor {
    typedef ComposeTransform<Arc> type;
  };

  bool addFstOption;  // so that StatisticalTokenizer can reuse Compose without having an "fst" option.

  ComposeTransformOptions() : addFstOption(true) {}

  template <class Config>
  void configure(Config& config) {
    ComposeOptions::configure(config);
    if (addFstOption) config("fst", &fst)("name of fst hypergraph resource (VHG or FHG)").require();
  }

  bool configureVocabulary() const { return false; }

  std::string fst;

  void validate() { fs::FstComposeOptions::validate(); }
};

// TODO: support 3 arc types: input (cfg), match (fst), output(cfg),
// using a configurable fn object. fs compose supports this already but
// it's not exposed to module.
template <class FstArc>
struct ComposeTransform : TransformBase<Transform::Inout>, ComposeTransformOptions {

  typedef TransformBase<Transform::Inout> Base;

  ComposeTransform(ComposeTransformOptions const& opt = ComposeTransformOptions())
      : ComposeTransformOptions(opt) {}

  typedef IMutableHypergraph<FstArc> FST;
  typedef shared_ptr<FST> FstPtr;
  typedef Util::ThreadSpecific<FstPtr> FstPtrForThread;

  mutable FstPtrForThread ppFst;

  FstPtr& pFst() const { return ppFst.get(); }

  mutable Util::ThreadSpecificBool resourceNeedsCheck_;

  template <class ResourceManager>
  void loadResources(ResourceManager&) {
    if (!fst.empty()) fstname = fst;
  }

  template <class ResourceManager>
  void loadResourcesThread(ResourceManager& mgr) const {
    if (!fst.empty()) {
      mgr.getResource(fst, pFst());
      resourceNeedsCheck_.set(true);
    }
  }

  bool haveFst() const { return pFst().get(); }

  void loadFst(std::string const& filename, IVocabularyPtr const& vocab) const {
    SDL_DEBUG(Compose, "loadFst into vocabulary @" << vocab.get());
    if (pFst())
      SDL_WARN(Hypergraph.Compose,
               "loading fst over existing one - not thread safe");  // could be made threadsafe if we make a
    // local copy of pFst first in inout, then
    // pass that to all subrs.
    typedef MutableHypergraph<FstArc> Hg;
    Hg* phg = new Hg(kFsmOutProperties | kCanonicalLex);
    phg->setVocabulary(vocab);
    Util::Input in(filename);
    if (!in) SDL_THROW_LOG(Hypergraph.Compose, ConfigException, "can't read fst file " << filename);
    readHypergraph(*in, *phg, filename);
    vocab->freeze();
    sortArcs(phg);
    pFst().reset(phg);
    checkFst();
  }

  /**
     not safe to sort an hg resource since there may be other users.
  */
  void checkFst(bool resource = false) const {
    FST* fst = pFst().get();
    if (!fst)
      SDL_THROW_LOG(Hypergraph.Compose, ConfigException,
                    "No fst - supply fst resource name, or call loadFst");
    if (!fst->isFsm())
      SDL_THROW_LOG(Hypergraph.Compose, ConfigException,
                    "Hypergraph resources for Compose.fst must be finite state (two tails, start state, "
                    "second tail has labeled leaf state");
    if (!fst->hasSortedArcs()) {
      if (resource || !fst->isMutable())
        SDL_THROW_LOG(Hypergraph.Compose, ConfigException,
                      "Hypergraph resources for Compose.fst must have sort-arcs: true");
      else
        sortArcs(fst);
    }
  }

  std::string fstname;

  void setFst(shared_ptr<FST> const& pFst_) const {
    pFst() = pFst_;
    checkFst();
  }
  void setFst(FST& fst) const { setFst(ptrNoDelete(fst)); }
  shared_ptr<FST> getFst() const { return pFst(); }

  void inoutCfg(IHypergraph<FstArc>& hg, IMutableHypergraph<FstArc>* pResultHg) const {
    FST& fst = *pFst();
    if (resourceNeedsCheck_.get()) {
      fst.maybeInitProcessAndThread();
      checkFst(true);
      resourceNeedsCheck_.set(false);
    }
    forcePropertiesIfMutable(hg, kComposeCfgRequiredProperties);
    composeImpl(hg, fst, pResultHg, *this);
  }

  IVocabularyPtr checkVocab(IHypergraph<FstArc> const& hg) const {
    IVocabularyPtr const& r = inputVocabMatches(hg);
    FST* fst = pFst().get();
    SDL_DEBUG(Compose, "transform input hg vocabulary @ " << r.get() << " should match fst vocabulary @"
                                                          << (fst ? fst->getVocabulary().get() : NULL));
    if (fst && r.get() != fst->getVocabulary().get()) {
      SDL_THROW_LOG(Hypergraph.Compose, ConfigException,
                    "fst '" << fstname << "' has different vocabulary '"
                            << vocabName(fst->getVocabulary().get()) << "' from input vocabulary '"
                            << vocabName(r.get()) << "' - reconfigure so 'input-vocab' agrees with "
                                                     "'hg-resource.vocabulary:' or 'compose.vocabulary:'.");
    }
    return r;
  }

  /**
     *pResultHg = hg * fst.
   */
  template <class Arc>
  void inout(IHypergraph<Arc> const& hgIn, IMutableHypergraph<Arc>* pResultHg) const {
    FST* fst = pFst().get();
    if (!fst) SDL_THROW_LOG(Hypergraph.Compose, ConfigException, "no fst loaded to compose input with");
    pResultHg->setVocabulary(checkVocab(hgIn));

    IHypergraph<Arc>& hg = const_cast<IHypergraph<Arc>&>(hgIn);
    if (fstCompose) {
      if (!hg.isMutable())
        SDL_WARN(Hypergraph.Compose,
                 "fst*fst composition not possible as configured; first fst is not mutable. falling back to "
                 "cfg*fst");
      else if (!hg.isFsmLike())
        SDL_WARN(Hypergraph.Compose,
                 "fst*fst composition not possible as configured; first is a cfg - falling back to cfg*fst");
      else {
        SDL_DEBUG(Hypergraph.Compose, "input is mutable fst - using fst*fst composition");
        SDL_DEBUG(Hypergraph.Compose, hg);
        fs::compose((IMutableHypergraph<Arc>&)(hg), *fst, pResultHg, (fs::FstComposeOptions const&)*this);
        return;
      }
    }
    inoutCfg(hg, pResultHg);
  }

  template <class Arc, class Arc2>
  void inoutCfg(IHypergraph<Arc> const& hg, IMutableHypergraph<Arc2>* presult_hg) {
    SDL_THROW_LOG(Hypergraph.Compose, ConfigException, "weight type must be same for cfg*fst compose");
  }

  IVocabularyPtr preferredInputVocabulary() {
    FST* fst = pFst().get();
    if (fst)
      return fst->getVocabulary();
    else {
      SDL_WARN(Hypergraph.Compose.preferredInputVocabulary,
               "fst: resource wasn't loaded yet; no preferred vocabulary");
      return IVocabularyPtr();
    }
  }

  static Properties inAddProps() { return kStoreFirstTailOutArcs; }
};

template <class Arc>
void compose(IMutableHypergraph<Arc>& hg1, IMutableHypergraph<Arc>& hg2, IMutableHypergraph<Arc>* resultCfg,
             ComposeTransformOptions const& composeOpt) {
  ComposeTransform<Arc> tr(composeOpt);
  tr.setFst(hg2);
  tr.inout(hg1, resultCfg);
}


}}

#endif
