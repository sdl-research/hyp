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
#include <sstream>

#include <sdl/Hypergraph/HelperFunctions.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/ArcParserFct.hpp>

namespace sdl {
namespace Hypergraph {

template<class Arc>
IMutableHypergraph<Arc>* constructHypergraphFromString(std::string const& hgstr,
                                                       IVocabularyPtr const& pVoc) {
  IMutableHypergraph<Arc>* hg = new MutableHypergraph<Arc>();
  hg->setVocabulary(pVoc);
  std::stringstream ss(hgstr);
  parseText(ss, "<string-input>", hg);
  return hg;
}

}}
