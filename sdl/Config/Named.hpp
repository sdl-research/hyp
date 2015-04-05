// Copyright 2014-2015 SDL plc
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

   mixin for setName for KeywordConfig-able classes.
*/

#ifndef NAMED_JG_2013_12_05_HPP
#define NAMED_JG_2013_12_05_HPP
#pragma once

#include <string>

namespace sdl {
namespace Config {

struct UnspecifiedCategoryAndType {
  static char const* category() { return "?"; }
  static char const* type() { return "?"; }
  static std::string usage() { return "?"; }
};

struct Nameless : UnspecifiedCategoryAndType {
  void setName(std::string const&) {}
  std::string name() const { return "?"; }
  static char const* type() { return "nameless"; }
};

struct Named : UnspecifiedCategoryAndType {
  std::string name_;
  void setName(std::string const& name) { name_ = name; }
  std::string name() const { return name_; }
  char const* nameC() const { return name_.c_str(); }
  static char const* type() { return "named"; }
};

struct INamed {
  INamed() : name_("_") {}
  virtual void setName(std::vector<char> const& name) { name_.assign(name.begin(), name.end()); }
  virtual void setName(std::string const& name) { name_ = name; }
  virtual std::string name() const { return name_; }
  virtual char const* nameC() const { return name_.c_str(); }
  virtual std::string usage() const { return "?"; }
  virtual char const* category() const { return "INamed"; }
  static char const* type() { return "INamed"; }

 protected:
  std::string name_;
};


}}

#endif
