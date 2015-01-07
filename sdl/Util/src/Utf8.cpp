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
/** \file

    Reminder: here's UTF8:

    sig bits | first | last | bytes | byte sequence pattern
    7 U+0000 U+007F 1 0xxxxxxx
    11 U+0080 U+07FF 2 110xxxxx 10xxxxxx
    16 U+0800 U+FFFF 3 1110xxxx 10xxxxxx 10xxxxxx
    21 U+10000 U+1FFFFF 4 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

    // and theoretically the unused (so far) code points:

    26 U+200000 U+3FFFFFF 5 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    31 U+4000000 U+7FFFFFFF 6 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
*/

#include <sdl/Util/String32.hpp>
#include <sdl/Util/Utf8.hpp>
#include <utility>
#include <unicode/uchar.h> //u_iscntrl u_isspace

namespace sdl {
namespace Util {

std::string fixedUtf8(std::string const& string) {
  std::string fixedStr;
  fixUtf8To(string, fixedStr);
  return fixedStr;
}

std::string & fixUtf8(std::string &string) {
  if (validUtf8(string)) return string;
  std::string fixedStr;
  fixUtf8To(string, fixedStr);
  std::swap(fixedStr, string);
  return string;
}

Unicode gWindows1252ToUnicode[kNWindows1252Unicode] = {
  0x20ac, // 0x80 -> Euro Sign
  0x0081,
  0x201a, // 0x82 -> Single Low-9 Quotation Mark
  0x0192, // 0x83 -> Latin Small Letter F With Hook
  0x201e, // 0x84 -> Double Low-9 Quotation Mark
  0x2026, // 0x85 -> Horizontal Ellipsis
  0x2020, // 0x86 -> Dagger
  0x2021, // 0x87 -> Double Dagger
  0x02c6, // 0x88 -> Modifier Letter Circumflex Accent
  0x2030, // 0x89 -> Per Mille Sign
  0x0160, // 0x8a -> Latin Capital Letter S With Caron
  0x2039, // 0x8b -> Single Left-Pointing Angle Quotation Mark
  0x0152, // 0x8c -> Latin Capital Ligature Oe
  0x008d,
  0x017d, // 0x8e -> Latin Capital Letter Z With Caron
  0x008f,
  0x0090,
  0x2018, // 0x91 -> Left Single Quotation Mark
  0x2019, // 0x92 -> Right Single Quotation Mark
  0x201c, // 0x93 -> Left Double Quotation Mark
  0x201d, // 0x94 -> Right Double Quotation Mark
  0x2022, // 0x95 -> Bullet
  0x2013, // 0x96 -> En Dash
  0x2014, // 0x97 -> Em Dash
  0x02dc, // 0x98 -> Small Tilde
  0x2122, // 0x99 -> Trade Mark Sign
  0x0161, // 0x9a -> Latin Small Letter S With Caron
  0x203a, // 0x9b -> Single Right-Pointing Angle Quotation Mark
  0x0153, // 0x9c -> Latin Small Ligature Oe
  0x009d,
  0x017e, // 0x9e -> Latin Small Letter Z With Caron
  0x0178 // 0x9f -> Latin Capital Letter Y With Diaeresis
};


}}
