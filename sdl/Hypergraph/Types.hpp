








#include <utility>
#include <cstddef>
#include <vector>
#include <limits>


























#include <boost/iterator/counting_iterator.hpp>






#include <boost/integer_traits.hpp>









#include <boost/tuple/tuple.hpp>







namespace Hypergraph {



























typedef std::vector<StateId> StateString;

typedef std::vector<StateArc> DerivationYield;



typedef boost::counting_iterator<StateId> StateIdIterator;
typedef boost::counting_iterator<ArcId> ArcIdIterator;
typedef boost::counting_iterator<TailId> TailIdIterator;

// similar use w/ boost range lib as iterator_range













static const ArcId kNoArc = boost::integer_traits<ArcId>::const_max;
static const StateId kNoState = boost::integer_traits<StateId>::const_max;

inline StateId maxState(StateId a, StateId b) {
  if (a == kNoState) return b;
  if (b == kNoState) return a;





}  // Hypergraph

namespace Constants {
using Hypergraph::kNoArc;
using Hypergraph::kNoState;






#endif
