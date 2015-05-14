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

    more efficient lexical_cast of substring (without copy).
*/


#ifndef CASTSUBSTRING_JG2013117_HPP
#define CASTSUBSTRING_JG2013117_HPP
#pragma once

#include <sdl/LexicalCast.hpp>
#include <boost/range/iterator_range.hpp>


namespace sdl {
namespace Util {

/**
   \return lexical_cast<Value> of string(charsBegin, charsEnd)
*/
template <class Value, class CharIterator>
Value castSubstring(CharIterator charsBegin, CharIterator charsEnd) {
  return sdl::lexical_cast<Value>(boost::make_iterator_range(charsBegin, charsEnd));
}

/**
   store into value lexical_cast<Value> of string(charsBegin, charsEnd)
*/
template <class Value, class CharIterator>
void castSubstringTo(Value& value, CharIterator charsBegin, CharIterator charsEnd) {
  value = sdl::lexical_cast<Value>(boost::make_iterator_range(charsBegin, charsEnd));
}

/**
   \return lexical_cast<Value> of str.substr(iBegin, iEnd)
*/
template <class Value>
Value castSubstring(std::string const& str, std::string::size_type iBegin, std::string::size_type iEnd) {
  std::string::const_iterator s = str.begin();
  return sdl::lexical_cast<Value>(boost::make_iterator_range(s + iBegin, s + iEnd));
}

/**
   store in value lexical_cast<Value> of str.substr(iBegin, iEnd)
*/
template <class Value>
void castSubstringTo(Value& value, std::string const& str, std::string::size_type iBegin,
                     std::string::size_type iEnd) {
  std::string::const_iterator s = str.begin();
  value = sdl::lexical_cast<Value>(boost::make_iterator_range(s + iBegin, s + iEnd));
}

/**
   \return lexical_cast<Value> of str.substr(iBegin, (end))
*/
template <class Value>
Value castSubstring(std::string const& str, std::string::size_type iBegin) {
  return sdl::lexical_cast<Value>(boost::make_iterator_range(str.begin() + iBegin, str.end()));
}

/**
   store in value lexical_cast<Value> of str.substr(iBegin, (end))
*/
template <class Value>
void castSubstringTo(Value& value, std::string const& str, std::string::size_type iBegin) {
  value = sdl::lexical_cast<Value>(boost::make_iterator_range(str.begin() + iBegin, str.end()));
}


}}

#endif
