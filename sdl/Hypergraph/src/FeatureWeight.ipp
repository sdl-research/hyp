#include <map>
#include <string>
#include <utility>
#include <stdexcept>




#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>  // crucial


namespace Hypergraph {

// It is somewhat important that this function is in .cpp file and not
// in header because it uses boost::spirit, which takes slightly
// longer to compile. (A simple boost::spirit toy example takes 6
// seconds to compile.)
//
// Useful boost::spirit resource:
// http://boost-spirit.com/home/articles/qi-example/parsing-a-list-of-key-value-pairs-using-spirit-qi
//
template <class FloatT, class MapT, class SumT>

  namespace qi = boost::spirit::qi;


                             // val[key=val, key=val, ...] where the map part is optional:
                             qi::double_ >> ('[' >> ((qi::int_ >> '=' >> qi::double_) % ',') >> ']') | qi::eps,

  if (!ok) {

  }
  weight->setValue(weightProxy.first);
  if (weightProxy.second.is_initialized()) {  // it's optional, so it might not be there

  }
}



