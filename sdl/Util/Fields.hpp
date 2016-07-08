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

    single or double char split Field (no-copy substring)
*/

#ifndef FIELDS_JG_2013_08_28_HPP
#define FIELDS_JG_2013_08_28_HPP
#pragma once

#include <sdl/Util/Field.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/SmallVector.hpp>
#include <sdl/Util/Split.hpp>
#include <algorithm>
#include <utility>
#include <vector>

namespace sdl {
namespace Util {

/**
   (input) fields separated by single char (tab, for hadoop), or, if doubleDelim, two such chars

   boost split_iterator is similar, but i didn't see any efficient finder for
   the two-character (double-space) convention used by RuleFeatures.
*/
struct FieldGenerator {
  /// generator interface:
  typedef void Peekable;
  typedef Field value_type;
  typedef Field const& reference;
  typedef reference result_type;
  Field const& peek() const { return field_; }
  Field const& get() const { return field_; }
  void got() { pop(); }
  operator bool() const { return field_.first != end_; }
  void pop() {
    Pchar& p = field_.second;
    if (p == end_)
      field_.first = end_;
    else {
      if (doubleDelim_) ++p;
      field_.first = ++p;
      advanceSecond();
    }
  }
  Field operator()() {
    assert(*this);
    Field r((field_));
    pop();
    return r;
  }

  void setDelim(char delim = '\t', bool doubleDelim = false) {
    delim_ = delim;
    doubleDelim_ = doubleDelim;
  }

  /// constructors:
  void init(Pchar begin, Pchar end, char delim = '\t', bool doubleDelim = false) {
    end_ = end;
    delim_ = delim;
    doubleDelim_ = doubleDelim;
    field_.first = begin;
    field_.second = begin;
    advanceSecond();
  }

  FieldGenerator(Pchar begin, Pchar end, char delim = '\t', bool doubleDelim = false) {
    init(begin, end, delim, doubleDelim);
  }
  FieldGenerator(Field range, char delim = '\t', bool doubleDelim = false) {
    init(range.first, range.second, delim, doubleDelim);
  }
  enum { singleSpace, doubleSpace };
  /**
     warning: don't give a temporary string expression to a FieldGenerator (no
     deep copy is made)
  */
  FieldGenerator(std::string const& str, char delim = '\t', bool doubleDelim = false) {
    Pchar data = str.data();
    init(data, data + str.size(), delim, doubleDelim);
  }
  FieldGenerator(char const* cstr, char delim = '\t', bool doubleDelim = false) {
    init(cstr, cstr + std::strlen(cstr), delim, doubleDelim);
  }

  Fields& store(Fields& fields) {
    while (*this) {
      fields.push_back(field_);
      pop();
    }
    return fields;
  }

 protected:
  void advanceSecond() {
    Pchar& p = field_.second;
    if (doubleDelim_) {
      // POST: at end or at start of delim
      for (; p != end_; ++p) {
        if (*p == delim_) {
          if (++p == end_) return;
          if (*p == delim_) {
            --p;
            return;
          }
        }
      }
    } else
      for (; p != end_ && *p != delim_; ++p)
        ;
  }

  char delim_;
  bool doubleDelim_;
  Field field_;
  Pchar end_;
};

inline void stringsToFields(Strings const& strings, Fields& fields) {
  fields.resize(strings.size());
  Fields::iterator o = fields.begin();
  for (Strings::const_iterator i = strings.begin(), e = strings.end(); i != e; ++i, ++o) *o = *i;
}

inline void splitToFields(std::string const& str, Fields& fields, char sep) {
  FieldGenerator fg(str, sep);
  while (fg) {
    fields.push_back(fg.get());
    fg.got();
  }
}


}}

#endif
