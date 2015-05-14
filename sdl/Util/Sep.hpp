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

 simple avoid-space-before-first-element separator.
*/

#ifndef SEP_JG_2015_05_16_HPP
#define SEP_JG_2015_05_16_HPP
#pragma once

namespace sdl { namespace Util {

struct Sep {
  char const* s;
  mutable bool squelch;

  // mutable since if ostream << Sep isn't const, then boost lazy test writer stream compile fails
  Sep(char const* s = " ") : s(s), squelch(true) {}
  operator char const*() const {
    if (squelch) {
      squelch = false;
      return "";
    } else
      return s;
  }
  template <class O>
  void print(O& o) const {
    if (squelch)
      squelch = false;
    else
      o << s;
  }
  friend inline std::ostream& operator<<(std::ostream& out, Sep const& self) {
    self.print(out);
    return out;
  }
};

}}

#endif
