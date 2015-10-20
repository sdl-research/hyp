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

   .
*/

#ifndef SUBSTITUTE_JG_2014_07_28_HPP
#define SUBSTITUTE_JG_2014_07_28_HPP
#pragma once

#include <sdl/Syms.hpp>

namespace sdl {

typedef unsigned ConstraintIndex;
enum { kNullConstraintIndex = (ConstraintIndex)-1 };

enum Replacing
{
  kNoReplacement = 0,
  kDecoderReplacement = 1,
  kFinalReplacement = 2,
  kFinalElseDecoderReplacement = 3,
  kDecoderElseFinalReplacement = 4
};

struct Substitute {
  /// guaranteed single-thread safe only (valid until next call)
  virtual Syms const& substitute(ConstraintIndex index) = 0;
  virtual ~Substitute() {}
  virtual ConstraintIndex size() const = 0;
};


Replacing const kDecodersReplacing = kDecoderElseFinalReplacement;

}

#endif
