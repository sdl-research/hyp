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
#include <sdl/Util/IcuUtil.hpp>
#include <sdl/Util/Utf8.hpp>
#include <iostream>

#include <graehl/shared/warning_push.h>
GCC_DIAG_IGNORE(unused-but-set-variable)

#include <unicode/brkiter.h>
#include <unicode/chariter.h>
#include <unicode/schriter.h>
#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include <graehl/shared/warning_pop.h>

#include <sdl/Util/IcuCase.hpp>
#include <sdl/Util/IcuIsCase.hpp>

namespace sdl {
namespace Util {

SDL_NAME_ENUM(IcuCase);
SDL_NAME_ENUM(IcuIsCase);

bool TokenizerBaseImpl::findNextNonSpace() {
  using namespace icu;
  while (charHasNext()) {
    Unicode ch = charPeek();
    if (!u_isspace(ch) && fix_(ch)) return true;
    charAdvance();
  }
  return false;
}

String32 toString32(Slice const& word) {
  return String32(FromUtf8IterC(word.first), FromUtf8IterC(word.second));
}

String16 toString16(Slice const& word) {
  String16 ws;
  appendUtf16From8(ws, word.first, word.second);
  return ws;
}

String16 toString16(std::string const& utf8Str) {
  String16 ws;
  appendUtf16From8(ws, utf8Str.begin(), utf8Str.end());
  return ws;
}

String32 toString32(std::string const& utf8Str) {
  return String32(FromUtf8Iter(utf8Str.begin()), FromUtf8Iter(utf8Str.end()));
}

void toString16(std::string const& utf8Str, String16& to) {
  appendUtf16From8(to, utf8Str.begin(), utf8Str.end());
}

void toString32(std::string const& utf8Str, String32& to) {
  to.assign(FromUtf8Iter(utf8Str.begin()), FromUtf8Iter(utf8Str.end()));
}

void toString16(Slice const& utf8, String16& to) {
  appendUtf16From8(to, utf8.first, utf8.second);
}

void toString32(Slice const& utf8, String32& to) {
  to.assign(FromUtf8IterC(utf8.first), FromUtf8IterC(utf8.second));
}

bool isUtf8Whitespace(std::string const& utf8Str) {
  // TODO: test
  icu::CharacterIterator* charIter
      = new icu::StringCharacterIterator(icu::UnicodeString::fromUTF8(utf8Str.c_str()));
  bool result = true;
  for (; charIter->hasNext(); charIter->next32()) {
    if (!u_isspace(charIter->current32())) {
      result = false;
      break;
    }
  }
  delete charIter;
  return result;
}


}}
