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

    characters we escape inside single or double quotes for hypergraph format.
*/

#ifndef QUOTE_LW20111219_HPP
#define QUOTE_LW20111219_HPP
#pragma once

#include <sdl/Util/EscapeChars.hpp>

namespace sdl {
namespace Util {

struct DoubleQuoteEsc : public EscapeChars {
  DoubleQuoteEsc() {
    add()('\a', "\\a")('\b', "\\b")('\f', "\\f")('\n', "\\n")('\r', "\\r")('\t', "\\t")('\v', "\\v")(
        '\\', "\\\\")('"', "\\\"");
  }
};

struct SingleQuoteEsc : public EscapeChars {
  SingleQuoteEsc() { add()('\'', "\\'")('\\', "\\\\"); }
};


}}

#endif
