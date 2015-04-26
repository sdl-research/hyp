// Copyright 2014 SDL plc
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

    allow only some words

    e.g. word-list blacklists (avoid pre-translating into a known foreign source word),
    skip mostly-numeric (for transliteration)
*/

#ifndef WORDFILTER_JG_2013_12_16_HPP
#define WORDFILTER_JG_2013_12_16_HPP
#pragma once

#include <sdl/Types.hpp>
#include <sdl/Util/Utf8.hpp>
#include <sdl/Util/Override.hpp>

namespace sdl {
namespace Util {

// TODO: if profiling shows it would help, make a WordFilter non-virtual base
// class, then CRTP IWordFilter generator (so you can compose
// filters w/o virtual call overhead e.g. in WordFilters)

/// optional base class for non-IWordFilter
struct WordFilterBase {
  void initProcess() {}

  /// try to set to alwaysAllow if possible
  void setAlwaysAllow() {}
  void setNeverAllow() {}

  /**
     \return whether allow*(...) must always return true.
  */
  bool alwaysAllow() const { return false; }
  /**
     \return whether allow*(...) must never return true.
  */
  bool neverAllow() const { return false; }
};

struct IWordFilter {
  /// should be called once after configure (we could make IWordFilter Evictable
  /// but I don't see a need yet)
  virtual void initProcess() {}

  /// try to set to alwaysAllow if possible
  virtual void setAlwaysAllow() {}
  virtual void setNeverAllow() {}

  // Evictable / configurable?
  virtual ~IWordFilter() {}
  bool operator()(std::string const& str) const { return allowString(str); }
  bool operator()(Unicodes const& str) const { return allowUnicodes(str); }

  /**
     \return whether allow*(...) must always return true.
  */
  virtual bool alwaysAllow() const { return false; }
  /**
     \return whether allow*(...) must never return true.
  */
  virtual bool neverAllow() const { return false; }

  /**
     must override one or both of:
  */
  virtual bool allowString(std::string const& word) const { return allowUnicodes(ToUnicodes(word)); }
  virtual bool allowUnicodes(Unicodes const& word) const { return allowString(FromUnicodes(word)); }
};

struct AllWords : IWordFilter {
  virtual bool alwaysAllow() const OVERRIDE { return true; }
  bool allowString(std::string const&) const OVERRIDE { return true; }
  bool allowUnicodes(Unicodes const&) const OVERRIDE { return true; }
};

struct AsciiWords : IWordFilter {
  enum { kAsciiMax = 127 };
  bool allowString(std::string const& word) const OVERRIDE {
    for (std::string::const_iterator i = word.begin(), e = word.end(); i != e; ++i)
      if ((unsigned char)*i > kAsciiMax) return false;
    return true;
  }
  bool allowUnicodes(Unicodes const& word) const OVERRIDE {
    for (Unicodes::const_iterator i = word.begin(), e = word.end(); i != e; ++i)
      if (*i > kAsciiMax) return false;
    return true;
  }
};

/// Impl is an IWordFilter with an allowString(word) but possibly without inheriting form IWordFilter
template <class Impl>
struct WordFilter : Impl, IWordFilter {
  Impl& impl() { return *this; }
  Impl const& impl() const { return *this; }

  void initProcess() OVERRIDE { impl().initProcess(); }
  void setAlwaysAllow() OVERRIDE { impl().setAlwaysAllow(); }
  void setNeverAllow() OVERRIDE { impl().setNeverAllow(); }
  bool alwaysAllow() const OVERRIDE { return impl().alwaysAllow(); }
  bool neverAllow() const OVERRIDE { return impl().neverAllow(); }
  bool allowString(std::string const& word) const OVERRIDE { return impl().allowString(word); }
  virtual bool allowUnicodes(Unicodes const& word) const OVERRIDE {
    return impl().allowString(FromUnicodes(word));
  }
};

template <class Impl>
struct WordFilterUnicodes : Impl, IWordFilter {
  Impl& impl() { return *this; }
  Impl const& impl() const { return *this; }

  void initProcess() OVERRIDE { impl().initProcess(); }
  void setAlwaysAllow() OVERRIDE { impl().setAlwaysAllow(); }
  void setNeverAllow() OVERRIDE { impl().setNeverAllow(); }
  bool alwaysAllow() const OVERRIDE { return impl().alwaysAllow(); }
  bool neverAllow() const OVERRIDE { return impl().neverAllow(); }
  bool allowString(std::string const& word) const OVERRIDE {
    return impl().allowUnicodes(Util::ToUnicodes(word));
  }
  virtual bool allowUnicodes(Unicodes const& word) const OVERRIDE { return impl().allowUnicodes(word); }
};

template <class Impl>
struct WordFilterStringUnicodes : Impl, IWordFilter {
  Impl& impl() { return *this; }
  Impl const& impl() const { return *this; }

  void initProcess() OVERRIDE { impl().initProcess(); }
  void setAlwaysAllow() OVERRIDE { impl().setAlwaysAllow(); }
  void setNeverAllow() OVERRIDE { impl().setNeverAllow(); }
  bool alwaysAllow() const OVERRIDE { return impl().alwaysAllow(); }
  bool neverAllow() const OVERRIDE { return impl().neverAllow(); }
  bool allowString(std::string const& word) const OVERRIDE { return impl().allowString(word); }
  virtual bool allowUnicodes(Unicodes const& word) const OVERRIDE { return impl().allowUnicodes(word); }
};


}}

#endif
