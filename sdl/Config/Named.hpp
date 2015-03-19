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
