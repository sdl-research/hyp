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
#include <sdl/Util/NormalizeUtf8.hpp>

namespace sdl {
namespace Util {

void NormalizeUtf8::normalize(std::string& str, Constraints& c) const {
  normalizeNfc(str, c);
  if (constraintsIndexUnicodes(c) && FixUnicode::removeControlChars)
    SDL_THROW_LOG(NormalizeUtf8, ConfigException,
                  "remove-control-characters can't be true when using constraints (yet)");
  FixUnicode::normalize(str, true);
  SDL_TRACE(NormalizeUtf8, "windows-1252-replaced: '" << str << "' #bytes=" << str.size()
                                                      << " #unicode=" << utf8length(str));
}

void NormalizeUtf8::normalize(std::string& str) const {
  FixedUtf8 fixed(str, *this);  // does FixUnicode
  if (fixed.modified()) {
    if (nfc) {
      str.clear();
      NfcOptions::normalize(fixed, str);
    } else {
      fixed.moveTo(str);
      NfcOptions::maybeWarn(str);
    }
  } else {
    NfcOptions::normalize(str);
  }
}

void NormalizeUtf8::normalizeNfc(std::string& str, Constraints& c) const {
  SDL_TRACE(NormalizeUtf8, "unnormalized: '" << str << "' #bytes=" << str.size()
                                             << " #unicode=" << utf8length(str));
  if (str.empty()) return;
  FixedUtf8 fixed(str);
  fixed.moveTo(str);
  SDL_TRACE(NormalizeUtf8, "fixed-utf8: '" << str << "' #bytes=" << str.size() << " #unicode=" << utf8length(str));
  NfcOptions::normalize(str, c);
  SDL_TRACE(NormalizeUtf8, "nfc-normalized: '" << str << "' #bytes=" << str.size()
                                               << " #unicode=" << utf8length(str));
}

void NormalizeUtf8::normalizeNfc(std::string& str) const {
  SDL_TRACE(NormalizeUtf8, "unnormalized: '" << str << "' #bytes=" << str.size()
                                             << " #unicode=" << utf8length(str));
  if (str.empty()) return;
  FixedUtf8 fixed(str);
  fixed.moveTo(str);
  SDL_TRACE(NormalizeUtf8, "fixed-utf8: '" << str << "' #bytes=" << str.size() << " #unicode=" << utf8length(str));
  NfcOptions::normalize(str);
  SDL_TRACE(NormalizeUtf8, "nfc-normalized: '" << str << "' #bytes=" << str.size()
                                               << " #unicode=" << utf8length(str));
}

void NormalizeUtf8::normalize(std::string const& str, std::string& out) const {
  assert(out.empty());
  if (FixUnicode::maybeNormalize(str, out))
    NfcOptions::normalize(out);
  else
    NfcOptions::normalize(str, out);
}


}}
