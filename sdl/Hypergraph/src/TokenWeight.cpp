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
#include <sdl/IVocabulary.hpp>
#include <sdl/Hypergraph/TokenWeight.hpp>
#include <sdl/Util/PrintRange.hpp>

namespace sdl {
namespace Hypergraph {

void Token::print(std::ostream& out, IVocabulary const* voc) const {
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
  if (!syms_.empty()) {
    sdl::print(out, syms_, voc);
    out << ",";
  }
  if (props_ == kUnspecified)
    out << '*';
  else {
    //    if (props_ & kBlockLeft) out << '{';
    if (props_ & kMustExtendLeft) out << '<';
    if (props_ & kExtendableLeft) out << '[';
    if (props_ & kExtendableRight) out << ']';
    if (props_ & kMustExtendRight) out << '>';
    if (props_ & kBlockRight) out << '}';
  }
}


}}
