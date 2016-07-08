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

    spirit adds ~10 sec to compile. TODO: use (faster compile, similar runtime)
    custom parsing code.

    http://boost-spirit.com/home/articles/qi-example/parsing-a-list-of-key-value-pairs-using-spirit-qi
*/

#include <sdl/Hypergraph/FeatureWeight.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <boost/fusion/include/std_pair.hpp>  // crucial
#include <boost/spirit/include/qi.hpp>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <utility>

namespace sdl {
namespace Hypergraph {

template <class FloatT, class MapT, class SumT>
void FeatureWeightTpl<FloatT, MapT, SumT>::set(std::string const& str) {
  namespace qi = boost::spirit::qi;
  std::pair<FloatT, boost::optional<std::map<typename MapT::key_type, FloatT>>> weightProxy;
  weightProxy.first = 0;
  // TODO: avoid proxy (copy)
  bool const ok
      = qi::phrase_parse(str.begin(), str.end(),
                         // val[key=val, key=val, ...] where the map part is optional:
                         qi::double_ >> ('[' >> ((qi::int_ >> '=' >> qi::double_) % ',') >> ']') | qi::eps,
                         qi::space, weightProxy);
  if (!ok)
    SDL_THROW_LOG(Hypergraph.FeatureWeightTpl, InvalidInputException, "Could not parse '" << str << "'");
  this->value_ = weightProxy.first;
  if (weightProxy.second.is_initialized()) {
    assert(&this->featuresWrite() != &*weightProxy.second);
    this->featuresWrite() = std::move(*weightProxy.second);
  }
}


}}
