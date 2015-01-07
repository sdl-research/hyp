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
/** \file

    USAGE: e.g.
    (at top level, not namespace)

    #define INSTANTIATE_ARC_TYPES(arc) template void invert(IMutableHypergraph<arc> &h);
    #include <sdl/Hypergraph/src/InstantiateArcTypes.ipp>

    note: lack of include guard is essential.
*/

#define INSTANTIATE_WEIGHT_TYPES(x) INSTANTIATE_ARC_TYPES( ArcTpl< x > ) INSTANTIATE_ARC_TYPES( ArcWithDataTpl< x > )
#include <sdl/Hypergraph/src/InstantiateWeightTypes.ipp>
#undef INSTANTIATE_ARC_TYPES
