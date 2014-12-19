









#include <boost/cstdint.hpp>




namespace Hypergraph {




















// An FSM is a special case of a CFG, i.e., it is a left-recursive,
// binary CFG, where the 2nd tail of each arc is lexical.


// Does hypergraph store incoming arcs per state?


// Does hypergraph store outgoing arcs per state?
// (Only one of kStoreFirstTailOutArcs or kStoreOutArcs
// can be effective for now. May want to make them
// independent/orthogonal.)













  would have a new kStoreJustFirstTailOutArcs for 3 and kStoreFirstTailOutArcs would be for 2.





// Does Hypergraph have one or more states that have an output label
// (i.e., it's a transducer)?


// TODO: rename kSortedOutArcs to kOutArcsSortedByInputLabel, and add
// kOutArcsSortedByOutputLabel (somtimes you want to sort by output label)
// TODO: This property is currently not automatically updated as arcs
// are added.


/// triggers hg.addState(...) to use a single state per lexical in/out
/// label leaf. property may be set even though there are some
/// redundant same-LabelPair leaf states.

























































}



















  }
};




























#endif
