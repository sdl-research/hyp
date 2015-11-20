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
/** \df

    USAGE: e.g.
   (at top level, not namespace)

   #define INSTANTIATE_ARC_TYPES(arc) template void invert(IMutableHypergraph<ArcTpl<arc> > &h);
   #include <sdl/Hypergraph/src/InstantiateWeightTypes.ipp>
*/

#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Hypergraph/ExpectationWeight.hpp>
#include <sdl/Hypergraph/FeatureWeight.hpp>
#include <sdl/Hypergraph/TokenWeight.hpp>

namespace sdl {
namespace Hypergraph {

/**
   Any new Weight type must be added here. Hypergraph functions
   and classes will automatically be instantiated with this type, so
   that the code can be compiled into object files (explicit template
   instantiation, see http://stackoverflow.com/a/555349/60628):
 */
INSTANTIATE_WEIGHT_TYPES(ViterbiWeight)
INSTANTIATE_WEIGHT_TYPES(LogWeight)
INSTANTIATE_WEIGHT_TYPES(FeatureWeight)
#if SDL_FLOAT == 32
typedef FeatureWeightTpl<double, std::map<FeatureId, double>> DFeatureWeight;
INSTANTIATE_WEIGHT_TYPES(DFeatureWeight)
#endif
INSTANTIATE_WEIGHT_TYPES(ExpectationWeight)

#undef INSTANTIATE_WEIGHT_TYPES


}}
