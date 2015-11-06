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

    no-copy string slice/reference/view

    no-copy string slices with convenience access (numeric conversion, trim
    whitespace, etc)

    TODO: use boost::string_ref or std::experimental::string_span instead in C++11
*/

#ifndef FIELD_JG_2015_06_18_HPP
#define FIELD_JG_2015_06_18_HPP
#pragma once

#include <sdl/Array.hpp>
#include <sdl/Types.hpp>
#include <sdl/Util/IsChar.hpp>
#include <sdl/Util/StringBuilder.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <graehl/shared/atoi_fast.hpp>
#include <boost/functional/hash.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/regex_fwd.hpp>
#include <vector>
#include <string>
#include <sdl/gsl.hpp>

namespace boost {
template <class BidiIterator>
struct sub_match;
}

namespace sdl {
namespace Util {


/**
   a slice into an underlying utf8 array/vector/string - contents may only be
   used while that underlying storage is valid. POD and with much higher
   performance conversion to number than sdl::lexical_cast or stringstream or
   sscanf

   since Slice=std::pair is officially POD in C++11 (and defacto since forever) we
   inherit from std::pair<Pchar, Pchar>

   TODO: move to Util/Field.hpp

   TODO: use boost::string_ref or std::experimental::string_span instead in C++11
*/
struct Field : Slice {
  Field() {}
  Field(Pchar begin, Pchar end) : Slice(begin, end) {}

  Field(Slice const& o) : Slice(o) {}
  Field(Field const& o) : Slice(o) {}
  Field(char const* word) : Slice(word, word + std::strlen(word)) {}
  Field(std::string const& word) : Slice(arrayBegin(word), arrayEnd(word)) {}
  Field(std::vector<char> const& word) : Slice(arrayBegin(word), arrayEnd(word)) {}
  Field(boost::iterator_range<std::string::const_iterator> const& word)
      : Slice(arrayBegin(word), arrayEnd(word)) {}
  template <class Iter>
  Field(boost::sub_match<Iter> const& word)
      : Slice(&*word.first, &*word.second) {}
  Field(std::string::const_iterator begin, std::string::const_iterator end) : Slice(&*begin, &*end) {}
  typedef Pchar const_iterator;
  typedef Pchar iterator;
  typedef char& reference;
  typedef char const& const_reference;
  typedef char value_type;

  Pchar begin() const { return first; }
  Pchar end() const { return second; }

  char operator[](std::size_t i) const { return first[i]; }

  void operator=(std::string const& str) {
    first = &*str.begin();
    second = &*str.end();
  }

  void operator=(std::vector<char> const& builder) {
    std::size_t sz = builder.size();
    if (!sz)
      first = second = 0;
    else {
      first = &*builder.begin();
      second = first + builder.size();
    }
  }

  void operator=(char const* str) {
    first = str;
    second = str + std::strlen(str);
  }

  template <class Range>
  bool operator==(Range const& str) const {
    return size() == str.size() && std::equal(begin(), end(), str.begin());
  }

  bool operator==(char c) const { return size() == 1 && *first == c; }

  template <class Range>
  bool operator!=(Range const& str) const {
    return !operator==(str);
  }

  std::string str() const { return std::string(first, second); }

  std::string& str(std::string& store) const {
    store.assign(first, second);
    return store;
  }

  bool empty() const { return first == second; }

  std::size_t size() const { return second - first; }
  /**
     \return addr of first char c in field, else 0
  */
  Pchar find_first(char c) const {
    for (Pchar i = first; i < second; ++i)
      if (*i == c) return i;
    return 0;
  }

  void print(std::vector<char>& out) const { out.insert(out.end(), first, second); }

  friend inline std::ostream& operator<<(std::ostream& out, Field const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const { out.write(first, size()); }

  template <class Real>
  Real toReal(bool requireComplete = true) const {
    using namespace graehl;
    StrCursor str(first, second);
    double r = scan_real<double>(str);
    // could scan_real<Real> but prefer more intermediate precision
    if (requireComplete && str)
      SDL_THROW_LOG(RuleFeatures.Field, ConfigException, "'" << *this << "' was not (just) a double");
    return (Real)r;
  }

  double toDouble(bool requireComplete = true) const { return toReal<double>(requireComplete); }

  float toFloat(bool requireComplete = true) const { return toReal<float>(requireComplete); }

  template <class Int>
  int toSignedInt(bool requireComplete = true) const {
    return requireComplete ? graehl::atoi_fast_complete<Int>(first, second)
                           : graehl::atoi_fast<Int>(first, second);
  }

  int toInt(bool requireComplete = true) const { return toSignedInt<int>(requireComplete); }

  template <class Unsigned>
  Unsigned toUnsignedInt(bool requireComplete = true) const {
    return requireComplete ? graehl::atou_fast_complete<Unsigned>(*this, first, second)
                           : graehl::atou_fast<Unsigned>(first, second);
  }

  unsigned toUnsigned(bool requireComplete = true) const { return toUnsignedInt<unsigned>(true); }

  std::size_t toSize(bool requireComplete = true) const { return toUnsignedInt<std::size_t>(true); }

  double& toNumber(double* num, bool requireComplete = true) const {
    return * num = toDouble(requireComplete);
  }
  float& toNumber(float* num, bool requireComplete = true) const { return * num = toFloat(requireComplete); }
  unsigned& toNumber(unsigned* num, bool requireComplete = true) const {
    return * num = toUnsigned(requireComplete);
  }
  int& toNumber(int* num, bool requireComplete = true) const { return * num = toInt(requireComplete); }
  std::size_t& toNumber(std::size_t* num, bool requireComplete = true) const {
    return * num = toSize(requireComplete);
  }
  void resize(std::size_t size) { second = first + size; }

  /**
     \return field starts with str.
  */
  bool startsWith(std::string const& str) {
    return size() >= str.size() && std::equal(str.begin(), str.end(), first);
  }

  friend inline std::size_t hash_value(Field const& field) {
    return boost::hash_range(field.first, field.second);
  }

  template <class IsChar>
  void trimRight(IsChar const& is = IsChar()) {
    Pchar last = second;
    while (--last >= first && is(*last))
      ;
    second = ++last;
  }

  template <class IsChar>
  void trimLeft(IsChar const& is = IsChar()) {
    while (first < second && is(*first)) ++first;
  }

  template <class IsChar>
  void trim(IsChar const& is = IsChar()) {
    trimRight(is);
    trimLeft(is);
  }

  void trimRight() { trimRight(IsSpace()); }

  void trimLeft() { trimLeft(IsSpace()); }

  void trim() { trim(IsSpace()); }

  /// return first part (before sep), modifying this to point to part after,
  /// else empty Field() if separator wasn't found
  Field popBefore(char sep) {
    Pchar f = first;
    for (Pchar i = first; i < second; ++i)
      if (*i == sep) {
        first = i + 1;
        return Field(f, i);
      }
    return Field();
  }

  /// return first part (before sep), modifying this to point to part after,
  /// else empty Field() if separator wasn't found
  Field popBefore(Slice sep) {
    Pchar f = first;
    Pchar i = std::search(first, second, sep.first, sep.second);
    if (i == second)
      return Field();
    else {
      first = i + (sep.second - sep.first);
      return Field(f, i);
    }
  }
  //operator cstring_span<>() const { return cstring_span<>(begin(), end()); }
};

typedef boost::hash<Field> FieldHash;

typedef std::vector<Field> Fields;  // Util::small_vector<Field, 2, std::size_t>

inline std::string str(boost::iterator_range<std::string::const_iterator> const& range) {
  return std::string(range.begin(), range.end());
}
}

inline Util::Field& field(Slice& slice) {
  return reinterpret_cast<Util::Field&>(slice);
}

inline Util::Field const& field(Slice const& slice) {
  return reinterpret_cast<Util::Field const&>(slice);
}

inline Util::Field field(boost::iterator_range<std::string::const_iterator> const& range) {
  return Util::Field(range);
}


}

#endif
