








#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <utility>
#include <boost/range/algorithm/lower_bound.hpp>
#include <boost/unordered_set.hpp>
#include <boost/tuple/tuple.hpp>


#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <utility>
#include <boost/range/algorithm/lower_bound.hpp>
#include <boost/unordered_set.hpp>
#include <boost/tuple/tuple.hpp>









































namespace Hypergraph {

static const ArcId fakeArcId = (ArcId)-1;
/**



*/
template <class A>
struct CmpInputLabelWithSearchLabel {




  bool operator()(ArcId a, ArcId b) const {
    bool result;
    if (a == fakeArcId) {







    }
    return result;
  }




};

// used only in a test
template <class A>


  // binary search:
  return boost::lower_bound(arcIdsRange, fakeArcId, CmpInputLabelWithSearchLabel<A>(hg, sid, label));
}


template <class T>
class Registry {
 public:

  typedef std::set<T*, Util::LessByValue<T> > TSet;

  Registry() {}

  ~Registry() {

  }

  T* insert(T* pT) {

    const bool wasInserted = p.second;



    return *p.first;
  }



 private:

};

// only used in debug mode, for nicer output
template <class T>
class RegistryWithIds : public Registry<T> {
 public:
  typedef std::map<T*, std::size_t> TsToIdsMap;



  /**

  */
  std::size_t getId(T* t) {
    // T* uniquePtr = insert(t);





      return id;
    }
    return found->second;
  }

 private:


};

struct EarleyParserOptions {

  bool enablePhiRhoMatch;
  bool sigmaPreventsPhiRhoMatch;


};

// Does matching an eps (or sigma) arc prevent matching a phi or rho
// arc? (phi and rho mean "match otherwise") This should usually not
// be the case. E.g., we want to match 'x' or else, transition to
// failure state (phi); or (independent of that) transition to final
// state (eps).
// const boost::uint32_t kEarleyEpsPreventsPhiRhoMatch =   0x00000002;

template <class A>
class EarleyParser {

 public:
  typedef A Arc;
  typedef typename Arc::Weight Weight;







  typedef std::set<Item*> ItemsSet;



               EarleyParserOptions opts = EarleyParserOptions())


  }





  struct Item {










    bool operator<(const Item& other) const {
      if (from != other.from)
        return from < other.from;
      else if (to != other.to)
        return to < other.to;
      else if (arc != other.arc)
        return arc < other.arc;
      else if (dotPos != other.dotPos)
        return dotPos < other.dotPos;













    StateId from;
    StateId to;
    Arc* arc;

    Weight agendaWeight;
    Weight chartWeight;
    bool lastWasPhiOrEps;  // did we create this item by using a phi
    // transition? in that case, we can say we have
    // something lexical right before the dot (we
    // traversed an FST arc)
  };  // end Item

  struct ItemPriorityFct {
    bool operator()(Item* a, Item* b) const {
      if (a->from != b->from)
        return a->from < b->from;
      else
        return a->to < a->to;
    }
  };

  /**

  */


  }

  typedef std::pair<Item*, Item*> BackPointer;


    out << "[";
    if (Util::isDebugBuild()) {

      out << ", ";
    }


    writeArc(out, *item->arc, hg);


    return out;
  }

  template <class T>
  struct DbgItem {
    DbgItem(Item* i, EarleyParser<T>& ep) : i(i), ep(ep) {}

    friend std::ostream& operator<<(std::ostream& out, DbgItem const& di) {
      // by convention:
      const bool isFstEpsItem = (di.i->from == kNoState);

    }

    Item* i;
    EarleyParser<T>& ep;
  };

  typedef DbgItem<Arc> dbgItem;




  void pushAgendaItem(Item* item, Weight agendaWeight) {
    Hypergraph::removeFeatures(agendaWeight);
    if (item->agendaWeight == Weight::zero()) {
      item->agendaWeight = agendaWeight;





    }
  }

  void init() {





      Item* item = createItem(from, from, arc, 0);

    }


  }

  void predict(Item* item) {

    StateId sid = item->arc->getTail(item->dotPos);


      // automatically derive start state
      Item* newItem = createItem(item->from, item->to, item->arc, item->dotPos + 1);
      pushAgendaItem(newItem, item->chartWeight);

      // Backpointer is a pseudo item that derives the start state



      }


      return;
    }



      return;
    }



      Item* newItem = createItem(from, to, arc, 0);

    }




  }

  /**

  */
  void scanEps(Item* item) {
    // Only attach a found eps after scanning something else (low in
    // the tree), not after some nonterminal
    if (item->dotPos > 0) {
      StateId mostRecentlyCompleted = item->arc->getTail(item->dotPos - 1);

      if ((!mostRecentlyCompletedWasLexical && !item->lastWasPhiOrEps)

        return;
      }



    // Don't attach eps right before a non-lexical nontermial (will
    // instead attach inside that nonterminal)
    if (!item->isComplete()) {
      StateId next = item->arc->getTail(item->dotPos);



    }
    StateId sid = item->to;



      if (label == EPSILON::ID) {


        // newItem->lastWasPhiOrEps = true;
        pushAgendaItem(newItem, w);



        return;  // all following labels are non-eps
    }
  }

  /**

  */
  void scan(Item* item) {


    if (searchLabel == EPSILON::ID) {  // the CFG has an eps

      pushAgendaItem(newItem, item->chartWeight);



      return;


                    "Currently, special symbols <phi>, <rho>, <sigma> can only be used in "
                    "the 2nd argument to compose.");
    }
    StateId sid = item->to;

    std::size_t numMatches = 0;

    // store first arc that imposes some match condition (i.e., not
    // eps or sigma)


    // Iterate through first arcs to see if we have unconditional
    // matches (i.e., eps or sigma)



      if (foundLabel == EPSILON::ID) {
        continue;  // scanEps is a separate function

        // match unconditionally and consume






        firstConditionalArcId = aid;
        break;
      }
    }

    // Search for search label using binary search

    // binary search:
    ArcIdIterator arcEnd = boost::end(arcIdsRange);


    for (; matchingArcIdsIter != arcEnd; ++matchingArcIdsIter, ++numMatches) {








    }

    // Look for phi or rho


      for (ArcIdIterator aiter = ArcIdIterator(firstConditionalArcId), arcEnd = boost::end(arcIdsRange);
           aiter != arcEnd; ++aiter) {



          // non-consuming

          // newItem->lastWasPhiOrEps = true;



          // consuming



        }
      }
    }
  }

  /**


  */
  void complete(Item* item) {




        if (!oldItem->isComplete()) {
          assert(oldItem->arc->getTail(oldItem->dotPos) == head);


          pushAgendaItem(newItem, times(item->chartWeight, oldItem->chartWeight));
        }
      }
    }
  }

  /**


  */
  void findComplete(Item* item) {

      StateId nextTail = item->arc->getTail(item->dotPos);




          pushAgendaItem(newItem, times(item->chartWeight, completeItem->chartWeight));
        }
      }
    }
  }



  StateId createTerminalState(BackPointer bp);

  void backtrace(std::string name, Item* item, std::size_t recursion) {
    std::stringstream blanks;



    std::cerr << blanks.str() << name;
    const bool itemMeansEpsScan = item->arc == NULL;
    if (itemMeansEpsScan) {








        backtrace("(A) ", bp.first, recursion + 1);



      }
    }
  }



  void backtrace2(Item* item, std::size_t recursion) {




        std::cerr << " <- ";

        if (bp.second != NULL) {

        }

        backtrace2(bp.first, recursion + 1);



      }
    }
  }

  void backtrace2(Item* item) {

    backtrace2(item, 0);
  }

  StateId getResultCfgState(StateId inputCfgSid, std::size_t from, std::size_t to) {









    return resultId;
  }

  // at each dot position in a CFG rule item, a possible sequence of

  typedef std::vector<std::vector<Arc*> > ArcVecPerDotPos;


  /**


  */
  struct ItemAndMatchedArcs {
    ItemAndMatchedArcs(Item* i, ArcVecPerDotPosPtr s, StateId h, StateIdContainer const& t)
        : item(i), stateIds(s), head(h), tails(t) {

      boost::hash_combine(hashCode, h);

      boost::hash_range(hashCode, tails.begin(), tails.end());
    }

    bool operator==(const ItemAndMatchedArcs& other) const {
      return other.item == item && other.head == head && *other.stateIds == *stateIds && other.tails == tails;
    }
    bool operator<(const ItemAndMatchedArcs& other) const {
      if (item != other.item)
        return item < other.item;
      else if (head != other.head)
        return head < other.head;
      else if (*stateIds != *other.stateIds)
        return *stateIds < *other.stateIds;


    }

    Item* item;
    const ArcVecPerDotPosPtr stateIds;
    std::size_t hashCode;
    StateId head;
    StateIdContainer tails;  // TODO: too expensive?
  };

  struct ItemAndMatchedArcsHashFct {

  };









  /**


  */
  void createResultCfg() {

    ItemAndMatchedArcsSet alreadyExpanded;




      createResultArcs(item, head, &alreadyExpanded);
    }

  }

  IMutableHypergraph<A>* parse() {
    buildChart();
    createResultCfg();

  }

  void addConsequentsToAgenda(Item* item) {
    scanEps(item);



      StateId nextTail = item->arc->getTail(item->dotPos);



        predict(item);
        findComplete(item);
      }
    }
  }

  void buildChart() {


    init();



      Weight agendaWeight = item->agendaWeight;
      item->agendaWeight = Weight::zero();
      Weight oldChartWeight = item->chartWeight;

      // Enter into chart, unless already there
      if (oldChartWeight == Weight::zero()) {
        const bool isComplete = item->isComplete();






        }






      }
    }
  }

 public:



 private:








  // For nicer debug output:





#endif


  // foreach position and CFG state






  typedef std::map<Item*, std::set<BackPointer> > BackPointerMap;



  // states (in the result machine) of (cfg-state, from, to) triples
  typedef std::map<boost::tuple<StateId, std::size_t, std::size_t>, StateId> TripleToResultStateMap;





  // A CFG arc "s <- / 0" that we add as backpointer for the start of
  // the CFG




};  // end class

/// Functions that create the result Hypergraph from the chart:

/**

*/
template <class A>





    return sid;
  }
  return found->second;
}

/**


*/
template <class A>
StateId EarleyParser<A>::createTerminalState(BackPointer bp) {

  // CFG has an eps at that position

    StateId cfgLabelState = bp.first->arc->getTail(bp.first->dotPos);

    assert(cfgOutputLabel == EPSILON::ID);
    return addLexicalState(cfgOutputLabel, cfgOutputLabel);
  }

  StateId sid;



  if (isNonconsuming) {



    StateId cfgLabelState = bp.first->arc->getTail(bp.first->dotPos);






    // rewrite rho/sigma input to the actual matched CFG label


      sid = addLexicalState(cfgInputLabel, cfgInputLabel);



  }
  return sid;
}

template <class Arc>






                                << ", matchedArcs.size()=" << itemAndMatchedArcs->stateIds->size());


  if (foundItem != alreadyExpanded->end()) {

    delete itemAndMatchedArcs;
    return;
  }

  Item* item = itemAndMatchedArcs->item;
  alreadyExpanded->insert(itemAndMatchedArcs);




      if (Util::isDebugBuild()) {


      }
      Item* complete = bp.second;

      StateId rightmost;

      if (isLexical) {
        rightmost = createTerminalState(bp);





      tails2.push_back(rightmost);





      }

      createResultArcs1(irts, head, matchedArcs2, tails2, prod, alreadyExpanded);
      createResultArcs(bp.second, rightmost, alreadyExpanded);
    }


      if (tails.empty())

      else

    }
  }
}

template <class Arc>


  ArcVecPerDotPosPtr matchedArcs(new std::vector<std::vector<Arc*> >());
  ItemAndMatchedArcs* irts = new ItemAndMatchedArcs(item, matchedArcs, head, tails);

}

///

struct ComposeOptions {
  explicit ComposeOptions() {}



    // currently no options here

};


const Properties kComposeFstRequiredProperties = kFsm | kStoreOutArcs | kSortedOutArcs;
const Properties kComposeCfgRequiredProperties = kStoreInArcs;










template <class A>

                 ComposeOptions opts = ComposeOptions()) {
  ASSERT_VALID_HG(cfg);





    resultCfg->setEmpty();
    return;
  }



  }




  }








  }





  EarleyParserOptions earleyOpts;

  p.parse();


  resultCfg->setEmptyIfNoArcs();
  ASSERT_VALID_HG(*resultCfg);
}

// HgMaybeConst: means maybe you can modify if props are wrong, maybe
// you have to make a copy. both are IHypergraph<A> &



             OnMissingProperties onMissing = kModifyOrCopyEnsuringProperties) {
  typedef IHypergraph<A> H;
















    reqFstProp |= kStoreFirstTailOutArcs;
    reqFstProp &= ~kStoreOutArcs;







}



               OnMissingProperties onMissing = kModifyOrCopyEnsuringProperties) {


  }

}





  template <class Arc>
  struct TransformFor {























// TODO: support 3 arc types: input (cfg), match (fst), output(cfg),
// using a configurable fn object. fs compose supports this already but
// it's not exposed to module.






































































































































































#endif
