// Copyright 2014 SDL plc
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <map>
#include <string>
#include <utility>
#include <stdexcept>

#include <sdl/Hypergraph/FeatureWeight.hpp>
#include <sdl/Util/LogHelper.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>  // crucial

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
  bool ok = qi::phrase_parse(str.begin(), str.end(),
                             // val[key=val, key=val, ...] where the map part is optional:
                             qi::double_ >> ('[' >> ((qi::int_ >> '=' >> qi::double_) % ',') >> ']') | qi::eps,
                             qi::space, weightProxy);
  if (!ok) {
    SDL_THROW_LOG(Hypergraph.FeatureWeightTpl, std::runtime_error, "Could not parse '" << str << "'");
  }
  weight->setValue(weightProxy.first);
  if (weightProxy.second.is_initialized()) {  // it's optional, so it might not be there
    weight->featuresWrite().swap(*weightProxy.second);
  }
}


}}
