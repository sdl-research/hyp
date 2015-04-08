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
// Include both .cpp and .hpp here:
#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/HelperFunctions.hpp>
#include <sdl/Hypergraph/src/HelperFunctions.cpp>

// Add all functions from HelperFunctions.hpp here:
#define INSTANTIATE_ARC_TYPES(ArcT) \
  template IMutableHypergraph<ArcT>* constructHypergraphFromString(std::string const&, IVocabularyPtr const&);

#include <sdl/Hypergraph/src/InstantiateArcTypes.ipp>
