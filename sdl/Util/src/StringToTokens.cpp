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
#include <sdl/Util/StringToTokens.hpp>

namespace sdl { namespace Util {

void utf8ToCharPos(Pchar i, Pchar end, IAcceptString const& accept, NormalizeUtf8 const& fix, bool normalizeConsecutiveWhitespace)
{
  utf8ToCharPosImpl(i, end, accept, fix, normalizeConsecutiveWhitespace);
}

void utf8ToWordSpan(Pchar i, Pchar end, IAcceptString const& accept, NormalizeUtf8 const& fix)
{
  utf8ToWordSpanImpl(i, end, accept, fix);
}

void utf8ToWholeSpan(Pchar i, Pchar end, IAcceptString const& accept, NormalizeUtf8 const& fix)
{
  utf8ToWholeSpanImpl(i, end, accept, fix);
}

}}
