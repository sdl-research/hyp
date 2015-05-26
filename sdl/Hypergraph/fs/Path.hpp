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

    an fst derivation.
*/

#ifndef PATH_JG2013_03_05_HPP
#define PATH_JG2013_03_05_HPP
#pragma once

#include <sdl/Hypergraph/WeightUtil.hpp>
#include <sdl/Hypergraph/IsFeatureWeight.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/Util/RemovePointer.hpp>
#include <sdl/Hypergraph/Label.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>

namespace sdl {
namespace Hypergraph {
namespace fs {

/**
   this is like an FstArc<WeightT, State> but with no actual State stored.
*/
template <class WeightT, class StateT = bool>
struct FstArcNoState {
  typedef WeightT Weight;
  typedef StateT State;
  LabelPair labelPair;
  Weight weight;
  IF_SDL_HYPERGRAPH_FS_ANNOTATIONS(Syms annotations;)
  template <class State2>
  void setDst(State2 const& dst_) {}
  State getDst() const { return State(); }

  FstArcNoState() {}

  template <class FstArc2>
  FstArcNoState(FstArc2 const& arc)
      : labelPair(arc.labelPair)
      , weight(arc.weight)
#if SDL_HYPERGRAPH_FS_ANNOTATIONS
      , annotations(arc.annotations)
#endif
  {
#if SDL_HYPERGRAPH_FS_ANNOTATIONS
    SDL_DEBUG_IF(!annotations.empty(), Hypergraph.fs.Path, "fs arc Annotations: " << annotations);
#endif
  }

  void printLabel(std::ostream& out) const {
    out << '(' << labelPair.first;
    if (labelPair.first != labelPair.second) out << ' ' << labelPair.second;
    out << ')';
  }
  void printWeight(std::ostream& out) const {
    if (weight != Weight::one()) out << "/" << weight;
  }
  void print(std::ostream& out) const {
    printLabel(out);
    printWeight(out);
  }
  friend inline std::ostream& operator<<(std::ostream& out, FstArcNoState const& self) {
    self.print(out);
    return out;
  }

  void printLabel(std::ostream& out, IVocabulary const* vocab) const {
    out << '(';
    out << vocab->str(labelPair.first);
    if (labelPair.first != labelPair.second) out << ' ' << vocab->str(labelPair.second);
    out << ')';
  }
  void print(std::ostream& out, IVocabulary const& vocab) const {
    printLabel(out, &vocab);
    printWeight(out);
  }
  void print(std::ostream& out, IVocabulary const* vocab) const {
    printLabel(out, vocab);
    printWeight(out);
  }
  void print(std::ostream& out, IVocabularyPtr const& pvocab) const { print(out, *pvocab); }
  friend inline void print(std::ostream& out, FstArcNoState const& self, IVocabularyPtr const& pvocab) {
    self.print(out, pvocab);
  }
  friend inline void print(std::ostream& out, FstArcNoState const& self, IVocabulary const* pvocab) {
    self.print(out, pvocab);
  }
};

/// when creating a HG or path-symbols-sequence, prepend these each time you
/// have an arc w/ a given source StateId. makes the most sense if input states
/// can't recur on a path (i.e. acyclic), for e.g. keeping constraint open/close
/// symbols
typedef std::vector<Syms> PrependForInputState;
typedef shared_ptr<PrependForInputState> PrependForInputStatePtr;

template <class InState>
struct PrependForInputStateFn {
  virtual void append(InState const& state, Syms& appendTo) = 0;
  virtual ~PrependForInputStateFn() {}
};

typedef PrependForInputStateFn<StateId> PrependForInputStateIdFn;

struct PrependForInputStateCached : PrependForInputStateIdFn {
  PrependForInputState cached_;
  virtual void append(StateId const& state, Syms& appendTo) OVERRIDE {
    if (state < cached_.size()) appendTo.append(cached_[state]);
  }
};

template <class Arc>
Arc* addAnnotatedArc(IMutableHypergraph<Arc>& outHg, StateId head, StateId tail, LabelPair const& labelPair,
                     typename Arc::Weight const& wt
#if SDL_HYPERGRAPH_FS_ANNOTATIONS
                     , Syms const* annotations, bool annotate
#endif
                     ) {
  Arc* add = new Arc(wt);
  add->head() = head;
  StateIdContainer& tails = add->tails();
#if SDL_HYPERGRAPH_FS_ANNOTATIONS
  SDL_DEBUG_IF(annotations, Hypergraph.fs.Path, "fs arc Annotations: " << *annotations);
  if (annotate) {
   bool const label = labelPair.first != EPSILON::ID || labelPair.second && labelPair.second != EPSILON::ID;
    TailId n = annotations->size(), labeli = 1 + n;
    tails.resize(labeli + label);
    Syms::const_iterator anno = annotations->begin();
    for (TailId i = 1; i <= n; ++i, ++anno) {
      tails[i] = outHg.addState(*anno);
      SDL_TRACE(Hypergraph.fs.Path.annotations, "annotation " << *anno << " -> tail #" << i);
    }
    if (label) tails[labeli] = outHg.addState(labelPair);
  } else
#endif
  {
    tails.resize(2);
    tails[1] = outHg.addState(labelPair);
  }
  tails[0] = tail;
  outHg.addArc(add);
  return add;
}

/**
   arcs of a path (and start state, although you can always get that with fst.startState())
*/
template <class FstArcT>
struct Path {
  typedef FstArcT FstArc;
  typedef typename FstArc::Weight Weight;
  typedef typename FstArc::State State;
  typedef typename Weight::FloatT Distance;
  typedef std::vector<FstArc> ArcsFinalToStart;
  Distance totalDistance;

 private:
  ArcsFinalToStart arcs;  // in reverse order (first element .dst is a final state)
  FstArc startArc;  // only used to hold startArc.getDst()
 public:
  /// set from outside for constraints in TrainableCapitalizerModule
  shared_ptr<PrependForInputStateFn<State> > prependForInputState;

  /**
     add to the end of arcs (which is in reverse order) path arc fstArc.
  */
  template <class FstArc2>
  void prepend(FstArc2 const& fstArc) {
    SDL_DEBUG(Hypergraph.fs.Path, "prepend best-path fs arc: " << fstArc);
    arcs.push_back(fstArc);
  }

  Weight weight() {
    Weight r;
    computeWeight(r);
    return r;
  }
  void computeWeight(Weight& weight) {
    if (isDistanceWeight(&weight))
      weight.value_ = totalDistance;
    else if (*this) {
      weight = Weight::one();
      forall (FstArc const& arc, arcs) { timesBy(arc.weight, weight); }
    } else
      weight = Weight::zero();
  }


  void clear() { totalDistance = std::numeric_limits<Distance>::infinity(); }
  Path() {
    arcs.reserve(100);
    clear();
  }
  operator bool() const {
    return totalDistance != std::numeric_limits<Distance>::infinity();  // we check weight since arcs may be
    // empty if start==final and you'll
    // still have a valid path
  }
  template <class State2>
  void startState(State2 const& state) {
    startArc.setDst(state);
  }
  State startState() const { return startArc.getDst(); }
  State finalState() const { return arcs.empty() ? startState() : arcs.front().getDst(); }
  typedef typename ArcsFinalToStart::const_reverse_iterator iterator;
  typedef iterator const_iterator;
  iterator begin() const { return arcs.rbegin(); }
  iterator end() const { return arcs.rend(); }
  void print(std::ostream& out, IVocabulary const& vocab) const { print(out); }
  void print(std::ostream& out, IVocabularyPtr const& pvocab) const { print(out, *pvocab); }
  template <class Out>
  void print(Out& o) const {
    o << " {best path (" << arcs.size() << ") arcs, distance=" << totalDistance << "}";
  }
  template <class Ch, class Tr>
  friend std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& o, Path const& self) {
    self.print(o);
    return o;
  }

  /**
     usage:

     #include <Hypergraph/GetString.hpp>
     cout << textFromSyms(path.string(), pVocab)
  */
  Syms syms(bool skipEpsilon = true, LabelType inputOrOutput = kOutput) {
    Syms labels;
    appendSyms(labels, skipEpsilon, inputOrOutput);
    return labels;
  }
  void appendSyms(Syms& labels, bool skipEpsilon = true, LabelType inputOrOutput = kOutput) const {
    forall (FstArc const& arc, arcs) {
      Hypergraph::appendSyms(labels, arc.labelPair, skipEpsilon, inputOrOutput);
    }
  }

  /**
     insert new states as needed to place a copy of this path from start to
     final. if either state is kNoState, we use an existing start or final state
     (creating a new one if needed)

     note that you'll probably want to set the vocabulary (yourself, since path doesn't ahve a vocabulary)

     \return # of non-epsilon output symbols

     \param createAnnotatedGraph - create graph arcs, prefaced by input annotations and omit EPSILON:EPSILON
     input/output
  */
  template <class Arc>
  std::size_t addToHypergraph(IMutableHypergraph<Arc>& outHg, StateId start = kNoState,
                              StateId final = kNoState, bool removeEpsilon = true, bool projectOutput = false,
                             bool createAnnotatedGraph = true) {
    if (!*this) return 0;
    typedef typename Arc::Weight ArcWeight;
    if (final == kNoState) final = ensureFinal(outHg);
    if (arcs.empty() && start == kNoState) {
      outHg.setStart(final);
      return 0;
    }
    if (start == kNoState) start = ensureStart(outHg);
    if (arcs.empty()) {
      outHg.addArcFsa(start, final, EPSILON::ID, ArcWeight(totalDistance));
      return 0;
    }
    StateId tail = start;
    iterator i = begin(), first = i, lasti = end() - 1;
    Weight const* startEpsWeights = 0;
    Weight multipleExtraWeight;
    Weight* addEpsTo = 0;
    std::size_t nNonEpsilonOutput = 0;
    for (;; ++i) {
     bool const last = i == lasti;
#if SDL_HYPERGRAPH_FS_ANNOTATIONS
      Syms const& annotations = i->annotations;
      TailId const nAnnotations = createAnnotatedGraph ? annotations.size() : 0;
#else
      TailId const nAnnotations = 0;
#endif
      LabelPair label = i->labelPair;
     bool const inputEpsilon = label.first == EPSILON::ID;
     bool const outputEpsilon = label.second == EPSILON::ID || inputEpsilon && !label.second;
      nNonEpsilonOutput += outputEpsilon;
     bool const nolabel = outputEpsilon && (projectOutput || inputEpsilon);
      if (!nAnnotations && removeEpsilon && nolabel) {
        if (addEpsTo)
          timesBy(i->weight, *addEpsTo);
        else if (!startEpsWeights)
          startEpsWeights = &i->weight;
        else {
          multipleExtraWeight = times(*startEpsWeights, i->weight);
          startEpsWeights = &multipleExtraWeight;
        }
      } else {
        StateId head = last ? final : outHg.addState();
        if (projectOutput) setInputAsOutput(label);
        addEpsTo = &addAnnotatedArc(outHg, head, tail, label, i->weight
#if SDL_HYPERGRAPH_FS_ANNOTATIONS
                                    ,
                                    &annotations, createAnnotatedGraph
#endif
                                    )->weight();
        tail = head;
        if (startEpsWeights) {
          timesBy(*startEpsWeights, *addEpsTo);
          startEpsWeights = 0;
        }
      }
      if (last) break;
    }
    if (startEpsWeights) {
      assert(!addEpsTo);
      if (createAnnotatedGraph)
        outHg.addArcGraph(start, final, *startEpsWeights);
      else
        outHg.addArcFsa(start, final, EPSILON::ID, *startEpsWeights);
    }
    return nNonEpsilonOutput;
  }
};

/**
   this is like a template typedef (default ctor is the only one).
*/
template <class WeightT>
struct PathNoState : Path<FstArcNoState<WeightT> > {};

/**
   these are convenience 'template typedefs' from the perspective of a user who
   has an IHypergraph<Arc> that happens to be an fst.

   alternatively you can just use PathNoState<Weight> if you only care about
   input/output/weight and not states.
*/
template <class HgOrPtr>
struct FstArcForHg {
  typedef typename Util::RemoveConstPointer<HgOrPtr>::type Hg;
  typedef typename Hg::FstArcT type;
};

template <class HgOrPtr>
struct PathForHg {
  typedef typename FstArcForHg<HgOrPtr>::type FstArc;
  typedef Path<FstArc> type;
};

template <class Arc>
struct PathForArc {
  typedef typename Arc::Weight Weight;
  typedef Path<FstArc<Weight, StateId> > type;
};


}}}

#endif
