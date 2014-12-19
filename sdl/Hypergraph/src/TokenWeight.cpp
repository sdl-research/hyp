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

#include <sdl/IVocabulary.hpp>
#include <sdl/Hypergraph/TokenWeight.hpp>
#include <sdl/Util/PrintRange.hpp>

namespace sdl {
namespace Hypergraph {

void Token::append(Token const& other) {
  symIdsVec_.reserve(this->size() + other.size());
  forall (Sym symid, other) {
    symIdsVec_.push_back(symid);
  }
  if (other.properties() != Token::kUnspecified) {
    setExtendableRight(other.isExtendableRight());
    setMustExtendRight(other.mustExtendRight());
  }
  if (other.getEndState() != kNoState)
    setEndState(other.getEndState());
}

void Token::print(std::ostream& out, IVocabularyPtr pVoc) const {
  if (start() == kNoState)
    out << "?";
  else
    out << start();
  out << "-";
  if (getEndState() == kNoState)
    out << "?";
  else
    out << getEndState();
  out << ",";
  Util::Sep space(" ");
  forall (Sym symId, *this) {
    out << space;
    if (pVoc) {
      out << pVoc->str(symId);
    }
    else {
      out << symId;
    }
  }
  if (!this->empty()) {
    out << ",";
  }
  out << "props:" << this->properties();
}


}}
