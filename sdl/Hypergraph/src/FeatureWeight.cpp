// Copyright 2014 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <string>

#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Hypergraph/FeatureWeight.hpp>
#include <sdl/Hypergraph/src/FeatureWeight.ipp>

namespace sdl {
namespace Hypergraph {

// Explicit template instantiation of the parseWeightString function
// for FeatureWeight (i.e., FeatureWeightTpl templated on common
// types):
template
void parseWeightString(std::string const& str, FeatureWeight* weight);

// Define parseWeightString for the other float type
#if SDL_FLOAT == 32
template
void parseWeightString(std::string const& str,
                       FeatureWeightTpl<double, std::map<FeatureId, double> >* weight);
#else
template
void parseWeightString(std::string const& str,
                       FeatureWeightTpl<float, std::map<FeatureId, float> >* weight);
#endif


}}
