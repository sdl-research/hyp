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
#ifndef SDL_CONFIGURE__CONFIG_HPP
#define SDL_CONFIGURE__CONFIG_HPP
#pragma once

/** \file

    YAML based implementation of configure library (see docs/)..
*/

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4146)
#endif
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/impl.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <sdl/Util/StableVector.hpp>
#include <sdl/Config-fwd.hpp>
#include <sdl/Path.hpp>
#include <iostream>
#include <sdl/StringConsumer.hpp>
#include <sdl/Util/OnceFlag.hpp>
#include <sdl/Util/LogHelper.hpp>

namespace YAML {
template <>
struct convert<Node> {
  static Node const& encode(Node const& rhs) { return rhs; }
  static bool decode(Node const& node, Node& rhs) {
    rhs = node;
    return true;
  }
};
}

// since ConfigNode is a typedef, it would only be found by ADL in the yaml namespace
inline bool is_null(sdl::ConfigNode const& node) {
  return !node.IsDefined() || node.IsNull();
}

namespace sdl {

inline ConfigNode child(ConfigNode const& parent, std::string const& childname) {
  assert(!is_null(parent));
  ConfigNode const& child(parent[childname]);
  if (is_null(child))
    SDL_THROW_LOG(Config, ConfigException, "child node " << childname << " not present in yaml");
  return child;
}

namespace Config {

typedef Util::StableStrings OptPath;


template <class Key, class Val>
inline void addKeyVal(ConfigNode& node, Key const& key, Val const& val) {
  node.force_insert(key, val);
}

/**
   \return a copy of a single YAML node, recursively if deep==true. does not copy tags.
*/
ConfigNode copy(ConfigNode const& in, bool deep = false);

/**
   \return except for 'removeKey: ...' if any, a copy of a single YAML node, recursively if deep==true. does
   not copy tags.
*/
ConfigNode copyRemovingKey(ConfigNode const& in, std::string const& removeKey, bool deep = false);

inline ConfigNode withoutTypeOrCategory(ConfigNode const& in) {
  return copyRemovingKey(copyRemovingKey(in, "type"), "category");
}

/**
   \return copy(in, true) - may maintain backreferences if using YAML version 0.5 or later
*/
ConfigNode clone(ConfigNode const& in);

/**
   \return a new tree copied from in (expanding any shared subtrees - will lose any existing backreferences)
*/
ConfigNode copyToTree(ConfigNode const& in);

typedef std::map<std::string, std::string> UnrecognizedOptions;

// these couldn't be in Config-fwd because ConfigNode is a lightweight value type

struct ConfigNodeFormat {
  ConfigNodeFormat(ConfigNodeFormat const& base, unsigned maxDepth = (unsigned)-1)
      : indentSpace(base.indentSpace)
      , longMap(base.longMap)
      , multiLine(base.multiLine)
      , maxDepth(base.maxDepth) {}
  explicit ConfigNodeFormat(std::string const& indentSpace = "", bool longMap = false, bool multiLine = true,
                            unsigned maxDepth = (unsigned)-1)
      : indentSpace(indentSpace), longMap(longMap), multiLine(multiLine), maxDepth(maxDepth) {}
  unsigned maxDepth;
  std::string indentSpace;
  bool longMap, multiLine;
};

/** print a YAML ConfigNode in our preferred format. */
void print(ConfigNode const& in, std::string const& indent, std::ostream& os, bool longMap = false,
          bool newLine = true, unsigned maxDepth = (unsigned)-1);

/**
   usage: os << printer(node, format).
*/
inline void print(std::ostream& os, ConfigNode const& node, ConfigNodeFormat const& format) {
  print(node, format.indentSpace, os, format.longMap, format.multiLine, format.maxDepth);
}

#if SDL_ENCRYPT
// give uniterpreted yaml tree from encrypted file
ConfigNode loadRawEncryptedConfig(Path const& path);
#endif

namespace {
ConfigNodeFormat const oneline("", true, false);
ConfigNodeFormat const multiline(" ", false, true);
ConfigNodeFormat const kMultilineBraces(" ", true, true);
ConfigNodeFormat const kShallow("", true, false, 2);
}

// gives uninterpreted yaml tree from file
ConfigNode loadRawConfig(Path const& path);

/// fully expanded starting from relative or absolute path 'basis', and interpreted via conventions specified
/// in ../docs/xmt-configuration-with-yaml.pdf
ConfigNode expandConfig(ConfigNode const& unexpanded, Path const& filePath, OptPath const& forPath = OptPath());

// Loads and expands the config.
ConfigNode loadConfig(Path const& path, OptPath const& forPath = OptPath());


/**
   sequence of YAML map/sequence keys
*/
typedef std::vector<std::string> ConfigPath;

/**
   \return yamlPathDotSeparated split on '.'
*/
ConfigPath parsePath(std::string const& yamlPathDotSeparated);

/**
   \return a yaml tree directly parsed from yamlString, without any of the transformations that happen in
   expandConfig or loadConfig.
*/
inline ConfigNode parseConfig(std::string const& yamlString) {
  ConfigNode r;
  string_to_impl(yamlString, r);
  return r;
}

ConfigNode overrideConfig(ConfigNode const& root, ConfigPath const& replaceAt,
                          ConfigNode const& substituteAtPath);

/**
   \return tree replaced by substituteAtPath at path replaceAt (doesn't modify input)

   \param root base for overriding (possibly null/empty)

   \param yamlPathDotSeparated string, split on ., giving map or sequence keys from root.

   \param substituteAtPath - tree used as value at yamlPathDotSeparated

   creates map nodes wherever null or nothing was defined before, but won't replace a non-map node with a map
   node (will throw ConfigException in that case).
*/
inline ConfigNode overrideConfig(ConfigNode const& root, std::string const& yamlPathDotSeparated,
                                 std::string const& substituteYamlString) {
  return overrideConfig(root, parsePath(yamlPathDotSeparated), parseConfig(substituteYamlString));
}

}  // end Config ns

/** usage: struct MyConfigurable : sdl::YamlConfigure<MyConfigurable, YamlConfigurable>

    (YamlConfigure is in ConfigYaml.hpp)

    which will implement YamlConfigurable (by CRTP) */
class YamlConfigurable {
 public:
  std::string category_, type_, name_;
  virtual ~YamlConfigurable() {}
  virtual std::string logname() const { return type_; }

  friend inline std::ostream& operator<<(std::ostream& out, YamlConfigurable const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const;

  /** \returns void fn(string), where fn logs string to logname(). */
  StringConsumer log() const;

  /** the below call configure library actions on this' implementation type (via
      YamlConfigurable<MyConfigurable> which knows MyConfigurable's type) . */

  virtual void init() = 0;
  virtual void store(ConfigNode const& configNode) = 0;
  virtual void validateStored() = 0;  // not named validate to avoid ambiguity with subclass validate

  /** init, store, validate.

      \param logEffective - show effective config as a single LOG_INFO message

      \param effectiveVerbosity (if logEffective)
  */
  void apply(ConfigNode const& configNode, bool logEffective = false, int effectiveVerbosity = 0);

  /** print help.

      \param verbosity: 0 should give options significant to end user. 1 would
      show some that are fine defaulted and intended to be hidden. n also shows
      all options that had .verbose(m) with m<=n
  */
  virtual void help(std::ostream& out, int verbosity = 0) const = 0;
  /** show dummy configuration. */
  virtual void showExample(std::ostream& out, int verbosity = 0) const = 0;
  /** print configuration reflecting current settings.

      see effective() which returns a YAML ConfigNode - printing that should
      give similar results. */
  virtual void showEffective(std::ostream& out, int verbosity = 0) const = 0;
  /** showEffective to string. */
  std::string getEffective(int verbosity = 0) const;

  /** TODO: return a YAML config tree that is as close as possible to yielding *this when you store(it). */
  virtual ConfigNode effective() const;

  /** type/name/category accessors for SDL.cpp top-level YAML configuration. */
  std::string const& yamlType() const { return type_; }
  std::string const& getName() const { return name_; }
  char const* getCategory() const { return category_.c_str(); }

  void setType(std::string const& type) { type_ = type; }
  void setCategory(std::string const& category) { category_ = category; }
  void setName(std::string const& name) { name_ = name; }
  template <class Config>
  void configure(Config& config) {}

  virtual void logEffective(int verbosity = 0) const;

  void logEffectiveOnce(int verbosity = 0) const {
    if (logEffectiveOnce_.first()) logEffective(verbosity);
  }

 protected:
  mutable Util::OnceFlagAtomic logEffectiveOnce_;
};


}

#endif
