#include <map>
#include <string>
#include <utility>
#include <stdexcept>

#include <sdl/Hypergraph/FeatureWeight.hpp>
#include <sdl/Util/LogHelper.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>  // crucial
#include <sdl/Util/Move.hpp>

namespace sdl {
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
void parseWeightString(std::string const& str, FeatureWeightTpl<FloatT, MapT, SumT>* weight) {
  namespace qi = boost::spirit::qi;
  std::pair<FloatT, boost::optional<std::map<typename MapT::key_type, FloatT> > > weightProxy;
  bool const ok = qi::phrase_parse(str.begin(), str.end(),
                             // val[key=val, key=val, ...] where the map part is optional:
                             qi::double_ >> ('[' >> ((qi::int_ >> '=' >> qi::double_) % ',') >> ']') | qi::eps,
                             qi::space, weightProxy);
  if (!ok)
    SDL_THROW_LOG(Hypergraph.FeatureWeightTpl, InvalidInputException, "Could not parse '" << str << "'");
  weight->setValue(weightProxy.first);
  if (weightProxy.second.is_initialized())
    Util::moveAssign(weight->featuresWrite(), *weightProxy.second);
}


}}
