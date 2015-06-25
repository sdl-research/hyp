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
/**

   \file

   applyYaml(yamlRootNode, &rootConfiguration) will initialize
   rootConfiguration, update its values as specified by yamlRootNode, and
   validate them. You don't even need to create a ConfigureYaml object yourself.

   (see docs/configuration-xmt-library.md to see how to specify
   rootConfiguration's initialization and transfer from yamlRootNode)

   TODO: allow configure() library to exclude certain options as being cmdline only?

   TODO: effectiveYAML: return a YAML config object that would give current values (with and without defaults)
*/

#ifndef CONFIGURE_JG2012615_HPP
#define CONFIGURE_JG2012615_HPP
#pragma once

#ifndef SDL_NFC_NORMALIZE_CONFIGURE
#define SDL_NFC_NORMALIZE_CONFIGURE 1
#endif

#ifndef SDL_ALLOW_NON_STRING_MAP_KEY_CONFIGURE
#define SDL_ALLOW_NON_STRING_MAP_KEY_CONFIGURE 1
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4146)
#endif
#include <yaml-cpp/yaml.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <sdl/Config/Config.hpp>
#include <sdl/Util/Warn.hpp>
#include <graehl/shared/configure.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <sdl/Util/Unordered.hpp>
#include <sdl/Util/FormattedOstream.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/LexicalCast.hpp>
#include <sdl/Util/Override.hpp>
#include <sdl/Util/Print.hpp>
#include <sdl/Util/Nfc.hpp>

namespace sdl {
namespace Config {

#if SDL_NFC_NORMALIZE_CONFIGURE
typedef Util::NfcStringRef YamlScalarRef;
#else
typedef std::string const& YamlScalarRef;
#endif

struct ConfigureYaml;

/** \return string quoted in help format - human readable, not parseable. */
inline std::string helpQuoteStringScalar(std::string const& s) {
  std::string r(1, '"');
  r.append(s).push_back('"');
  return r;
}

/** \return string quoted in YAML scalar string format. */
inline std::string yamlQuoteStringScalar(std::string const& s) {
  return sdl::lexical_cast<std::string>(ConfigNode(s));
}

// actions:
using configure::init_config;
using configure::store_config;
using configure::validate_config;
// construct w/ ostream:
using configure::show_effective_config;
using configure::help_config;
using configure::show_example_config;  // TODO: output YAML

// free fns:
using configure::configure_action;
using configure::validate_stored;

/** a configure library backend that maintains a stack of ConfigNode (aka YAML
   subtree) for store_action. Also provides YAML-like formatting for help and
   show_effective actions.
 */
struct ConfigureYaml : configure::configure_backend_base<ConfigureYaml> {
 private:
  typedef configure::configure_backend_base<ConfigureYaml> Base;
  unsigned endColumn, minUsageIndent;
  void defaults() {
    endColumn = 80;
    minUsageIndent = 16;
  }

 public:
  friend inline unsigned max_column(ConfigureYaml const& x) { return x.endColumn; }
  /** for store action, need Yaml ConfigNode. */
  ConfigureYaml(ConfigNode const& root, StringConsumer const& warn)  // = Util::logWarning("configure")
      : Base(warn),
        ctx(new Context()),
        root(root) {
    defaults();
  }
  /** for non-store actions only. */
  explicit ConfigureYaml(StringConsumer const& warn, int verbosity = 0) : Base(warn), ctx(), root() {
    verbose_max = verbosity;
    defaults();
  }

  void setRoot(ConfigNode const& rootNode) {
    ctx.reset(new Context());
    root = rootNode;
  }

  /// the rest of this class is configure-library implementation (not for the library user)
  static inline std::string quote(std::string const& s) { return yamlQuoteStringScalar(s); }

  /// the rest of this class is configure-library implementation (not for the library user)
  static inline std::string help_quote(std::string const& s) { return helpQuoteStringScalar(s); }

 private:
  enum { kWarnOnMissingRequired = false, kThrowOnMissingRequired = true };
  typedef configure::conf_expr_base Conf;
  typedef configure::conf_opt Opt;
  friend struct configure::configure_backend_base<ConfigureYaml>;

  static inline void print_name_val(std::ostream& o, std::string const& name, std::string const& val) {
    o << name << ": " << val;
  }

  unsigned print_usage(std::ostream& out, std::string const& usage, unsigned startColumn,
                       unsigned indentColumn) const {
    indentColumn += 4;
    if (indentColumn < minUsageIndent) indentColumn = minUsageIndent;
    std::string indent(indentColumn, ' ');
    if (startColumn) out << '\n';
    return Util::printFormatted(out, usage, indent, indent, endColumn);
  }

  static inline std::string option_name(configure::conf_expr_base const& conf) { return conf.name(); }

  typedef std::map<std::string, YAML::Node> ChildMap;
  typedef ChildMap::value_type ChildPair;
  typedef configure::conf_opt::allow_unrecognized_args UnrecognizedOpt;

  /** For tree_action nodes need keeping track of unrecognized options. We put
      these on a stack alongside the ConfigureYaml objects. For leaf vector/map
      nodes we also have an item on stack and may recurse to other tree or leaf
      actions, even though technically vector and map are leaves */
  struct Subtree {
    ConfigNode root;
    bool exists() const { return !is_null(root); }

    /** for missing/dup key errors. */
    StringConsumer warn;
    std::string pathname;  // TODO: build including sequence/open-map key in path (not using conf.path_name())
    std::string fullname(std::string const& key) const {
      return (pathname.empty() ? "" : (pathname + ".")) + key;
    }
    std::string rootstr() const { return (pathname.empty() ? "<ROOT>" : pathname) + ": " + to_string(root); }

    /** contained is a one-shot register set by "leaf" map and string for each
       of the values, which we cannot look up by a pathname in configure() space
       (the keys are arbitrary).
     */
    boost::optional<ConfigNode> contained;

    /// \return get root[key], !IsDefined() if not found. error if root is defined, non-null, non-map
    ConfigNode node(std::string const& key, bool required = false, bool requiredThrow = true) {
      if (!is_null(root)) {
        if (!root.IsMap())
          trouble("Key '" + key + "' missing - can't find in non-map YAML node: " + rootstr(), true);
        ChildMap::iterator i = unprocessedMapChildren.find(key);
        if (i != unprocessedMapChildren.end()) {
          ConfigNode r = i->second;
          unprocessedMapChildren.erase(i);
          return r;
        } else if (required)
          trouble("Key '" + key + "' missing from YAML node: " + rootstr(), requiredThrow);
      }
      return ConfigNode(YAML::NodeType::Undefined);
    }

    /** a yaml map's entries are stored here, but after we use each one we remove it.

        what remain at finish() might be typos, and get stored in unrecognizedOpt (or give an error if those
       aren't allowed)
    */
    ChildMap unprocessedMapChildren;

    UnrecognizedOpt unrecognizedOpt;

    /// any child of a non-required struct that has no yaml path for it gets
    /// ignored in the store phase (if you want an error for a missing node it
    /// needs to be .require()
    bool nothingToStore;

    Subtree(ConfigNode const& n, std::string const& pathname, StringConsumer const& warn, bool nothingToStore)
        : root(n)
        , pathname(pathname)
        , warn(warn)
        , unrecognizedOpt(false)
        , nothingToStore(nothingToStore)
        , finished(false) {
      if (n.IsMap()) {
        for (YAML::const_iterator i = n.begin(), e = n.end(); i != e; ++i) {
          YamlScalarRef key = i->first.Scalar();
          if (!Util::supplyMissing(unprocessedMapChildren, key, i->second))
            trouble(fullname(key) + " duplicated in YAML map; " + "was: " + (std::string const&)key + ": "
                        + to_string(i->second) + ", and now: " + to_string(i->second),
                    true);
        }
      }
    }

    void trouble(std::string const& msg, bool throwException = true) const {
      warn(msg);
      if (throwException) throw ConfigException(msg);
    }

    bool finished;

    /**
       may throw. call just before destructor (this used to be done in
       destructor, but this caused any exception e.g. in string->leaf to
       complain about things that would have been recognized had we continued
       walking the configure() tree without exception
    */
    void finish() {
      if (finished) return;
      finished = true;
      const bool allow = unrecognizedOpt.enable, warn = unrecognizedOpt.warn,
                 store = unrecognizedOpt.unrecognized_storage;
      if (allow && !warn && !store) return;
      for (ChildMap::const_iterator i = unprocessedMapChildren.begin(), e = unprocessedMapChildren.end();
           i != e; ++i) {
        ChildPair const& keyNode = *i;
        std::string const& key = keyNode.first;
        if (warn || !allow)
          trouble(key + ": " + to_string(keyNode.second) + " - '" + fullname(key)
                      + "' is an unrecognized option (check spelling ? )",
                  !allow);
        if (store) (*unrecognizedOpt.unrecognized_storage)[key] = to_string(keyNode.second);
      }
    }
    friend inline std::ostream& operator<<(std::ostream& out, Subtree const& self) {
      self.print(out);
      return out;
    }
    void print(std::ostream& out) const {
      YAML::Emitter em;
      em << root;
      out << em.c_str();
    }
    std::string str() const {
      YAML::Emitter em;
      em << root;
      return em.c_str();
    }
  };

  /** Context keeps track of where we are in the input Yaml tree while storing
      values into our configured objects. Actions other than store don'\t
      reference the input Yaml at all. */
  struct Context : boost::noncopyable {
    bool nothingToStore() const { return stack.empty() ? false : top().nothingToStore; }
    void setNothingToStore() { top().nothingToStore = true; }

    /// null unless we're about to store a map or sequence element
    boost::optional<ConfigNode> collectionElementConfig;
    /** set a one-shot register (collectionElementConfig) that modifies the next
       node() call since we already retrived the subconfig (lookup depends on
     * seq or map). */
    void pushCollectionYaml(ConfigNode const& subconfig) {
      assert(!collectionElementConfig);
      collectionElementConfig = subconfig;
    }

    typedef boost::ptr_vector<Subtree> Stack;
    Stack stack;

    bool ignore() {
      if (nothingToStore()) {
        collectionElementConfig.reset();
        return true;
      } else
        return false;
    }

    /**
       \return last collection value if parent was a map/sequence/set, else look
       up 'key' from previous top-of-stack node. return is !IsDefined() if not
       present
    */
    ConfigNode node(std::string const& key, bool required = false, bool requiredThrow = true) {
      if (collectionElementConfig) {
        ConfigNode r = *collectionElementConfig;
        collectionElementConfig.reset();
        return r;
      } else
        return top().node(key, required, requiredThrow);
    }
    Subtree& top() {
      assert(!stack.empty());
      return stack.back();
    }
    Subtree const& top() const {
      assert(!stack.empty());
      return stack.back();
    }
    void push(Subtree* sub) {
      if (!stack.empty() && top().root.is(sub->root))
        throw ConfigException("infinite loop in yaml subtree " + top().rootstr());
      stack.push_back(sub);
    }
    void pop() {
      top().finish();
      stack.pop_back();
    }
    std::string str() const {
      if (stack.empty()) return std::string();
      return stack.back().str();
    }
    friend std::string to_string(Context const& x) { return x.str(); }
    template <class O>
    void print(O& o) const {
      o << to_string(*this);
    }
    template <class Ch, class Tr>
    friend std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& o, Context const& self) {
      self.print(o);
      return o;
    }
  };

  typedef shared_ptr<Context> ContextPtr;
  ContextPtr ctx;

  ConfigNode root;

  void trouble(std::string const& name, std::string const& is, std::string const& reason = "configure error",
               std::string const& detail = "", bool throwException = false) const {
    std::string detailSuffix;
    if (!detail.empty()) detailSuffix = " (" + detail + ")";
    trouble(reason + " for option '" + name + "' of type " + is + detailSuffix, throwException);
  }
  void fatal(std::string const& name, std::string const& is, std::string const& reason = "configure error",
             std::string const& detail = "") const {
    trouble(name, is, reason, detail, true);
  }
  void trouble(std::string const& msg, bool throwException) const {
    warn(msg);
    if (throwException) throw ConfigException(msg);
  }

  template <class Val>
  void requireTrouble(Val* pval, Conf const& conf, bool throwException) const {
    trouble("Missing configuration for option '" + conf.path_name() + "' of type " + conf.opt->get_is(pval),
            throwException);
  }

 public:
  /** called by configure::conf_expr. */
  template <class Val>
  bool init_tree(configure::help_config help, Val* pval, Conf const& conf) const {
    return Base::init_tree(help, pval, conf);
  }

  /** called by configure::conf_expr. */
  template <class Val>
  bool init_tree(configure::store_config, Val*, Conf const& conf) const {
    assert(conf.depth || root);
    ConfigNode const& yaml = conf.depth ? ctx->node(conf.name()) : root;
    bool const shouldStore = conf.depth ? yaml.IsDefined() : true;
    SDL_TRACE(ConfigureYaml, "store=" << shouldStore << " init_tree " << conf.path_name() << ": "
                                      << Util::print(yaml, Config::kShallow));
    ctx->push(new Subtree(yaml, conf.path_name(), warn, !shouldStore));
    return shouldStore;
  }

  /** called by configure::conf_expr. */
  template <class Val>
  void tree_action(configure::store_config, Val* v, Conf const& conf) const {
    Subtree& top = ctx->top();
    Opt const& opt = *conf.opt;
    top.unrecognizedOpt = opt.get_unrecognized();
    const bool require = opt.is_required(), requireThrow = opt.is_required_err();
    if (top.exists())
      conf.warn_if_deprecated();
    else if (require && !top.nothingToStore)
      requireTrouble(v, conf, requireThrow);
    ctx->pop();
  }

  /** called by configure::conf_expr. */
  template <class Val>
  void sequence_action(configure::store_config, Val* pval, Conf const& conf) const {
    // if (ctx->nothingToStore()) return;
    Opt const& opt = *conf.opt;
    std::string const& name = conf.name();
    std::string const& pathname = conf.path_name();
    ConfigNode const& n = ctx->node(name, opt.is_required(), opt.is_required_err());
    SDL_TRACE(ConfigureYaml, "store sequence " << pathname << ": " << Util::print(n, Config::kShallow));
    if (!n.IsDefined()) return;
    if (n.IsNull()) return;  // null value should not be acceptable, but:
    // user may prefer to have shorthand feature-weights: (end of line/indent) meaning empty seq
    if (n.IsScalar()) {  // special case: allow single scalar value as singleton list
      conf.warn_if_deprecated();
      pval->clear();
      typename Val::value_type& subval = configure::append_default(*pval);
      ctx->pushCollectionYaml(n);
      configure::configure_store_init_from_base(*this, &subval, conf);
    } else if (!n.IsNull() && !n.IsSequence())
      throw ConfigException(pathname + " expected a YAML sequence");
    else {
      // TODO: handle map with int keys ?
      conf.warn_if_deprecated();
      pval->clear();
      unsigned key = 0;
      for (YAML::const_iterator i = n.begin(), e = n.end(); i != e; ++i) {
        typename Val::value_type& subval = configure::append_default(*pval);
        configure::conf_expr_base subconf(conf, graehl::to_string(key++));
        ctx->pushCollectionYaml(*i);
        configure::configure_store_init_from_base(*this, &subval, subconf);
      }
    }
  }

  /** called by configure::conf_expr. */
  template <class Val>
  void set_action(configure::store_config, Val* pval, Conf const& conf) const {
    // if (ctx->nothingToStore()) return;
    Opt const& opt = *conf.opt;
    std::string const& name = conf.name();
    std::string const& pathname = conf.path_name();
    ConfigNode const& n = ctx->node(name, opt.is_required(), opt.is_required_err());
    SDL_TRACE(ConfigureYaml, "store set " << pathname << ": " << Util::print(n, Config::kShallow));
    if (!n.IsDefined()) return;
    if (n.IsNull()) return;  // null value should not be acceptable, but:
    // user may prefer to have shorthand feature-weights: (end of line/indent) meaning empty set
    if (!n.IsSequence()) throw ConfigException(pathname + " expected a YAML sequence");
    conf.warn_if_deprecated();
    // TODO: handle map with int keys ?
    pval->clear();
    unsigned key = 0;
    for (YAML::const_iterator i = n.begin(), e = n.end(); i != e; ++i) {
      configure::defaulted<typename Val::value_type> subval;
      configure::conf_expr_base subconf(conf, graehl::to_string(key++));
      ctx->pushCollectionYaml(*i);
      configure::configure_store_init_from_base(*this, &subval.val, subconf);
      pval->insert(subval.val);
    }
  }

  /** called by configure::conf_expr. */
  template <class Val>
  void map_action(configure::store_config, std::map<std::string, Val>* pval, Conf const& conf) const {
    // if (ctx->nothingToStore()) return;
    Opt const& opt = *conf.opt;
    std::string const& name = conf.name();
    std::string const& pathname = conf.path_name();
    ConfigNode const& n = ctx->node(name, opt.is_required(), opt.is_required_err());
    SDL_TRACE(ConfigureYaml, "store map " << pathname << ": " << Util::print(n, Config::kShallow));
    if (!n.IsDefined()) return;
    if (n.IsNull()) return;  // null value should not be acceptable, but:
    // 1. yaml-cpp parser mis-parses {} as null
    // 2. user may prefer to have shorthand feature-weights: (end of line/indent) meaning empty map
    if (!n.IsMap()) throw ConfigException(pathname + " expected a YAML map: " + to_string(*ctx));
    conf.warn_if_deprecated();
    pval->clear();
    for (YAML::const_iterator i = n.begin(), e = n.end(); i != e; ++i) {
      YamlScalarRef key = i->first.Scalar();
      Val& subval = configure::append_default(*pval, (std::string const&)key);
      configure::conf_expr_base subconf(conf, key);
      ctx->pushCollectionYaml(i->second);
      configure::configure_store_init_from_base(*this, &subval, subconf);
    }
  }

  /** called by configure::conf_expr.

     // e.g. ReweightOptions uses map<FeatureId, FeatureValue>

   */
  template <class Val>
  void map_action(configure::store_config, Val* pval, Conf const& conf) const {
    // if (ctx->nothingToStore()) return;
    Opt const& opt = *conf.opt;
    std::string const& name = conf.name();
    std::string const& pathname = conf.path_name();
    ConfigNode const& n = ctx->node(name, opt.is_required(), opt.is_required_err());
    SDL_TRACE(ConfigureYaml, "store map " << pathname << ": " << Util::print(n, Config::kShallow));
    if (!n.IsDefined()) return;
    if (n.IsNull()) return;  // null value should not be acceptable, but:
    // 1. yaml-cpp parser mis-parses {} as null
    // 2. user may prefer to have shorthand feature-weights: (end of line/indent) meaning empty map
    if (!n.IsMap()) throw ConfigException(pathname + " expected a YAML map: " + to_string(*ctx));
    conf.warn_if_deprecated();
    pval->clear();
    for (YAML::const_iterator i = n.begin(), e = n.end(); i != e; ++i) {
      YamlScalarRef keystr = i->first.Scalar();
      typename Val::key_type key;
      graehl::string_to((std::string const&)keystr, key);
      typename Val::mapped_type& subval = configure::append_default(*pval, key);
      configure::conf_expr_base subconf(conf, keystr);
      ctx->pushCollectionYaml(i->second);
      configure::configure_store_init_from_base(*this, &subval, subconf);
    }
  }

  /** called by configure::conf_expr. */
  template <class Val>
  void leaf_action(configure::store_config, Val* pval, Conf const& conf) const {
    if (ctx->ignore()) return;
    Opt const& opt = *conf.opt;
    std::string const& name = conf.name();
    std::string const& pathname = conf.path_name();

    ConfigNode const& n = ctx->node(name, opt.is_required(), opt.is_required_err());
    SDL_TRACE(ConfigureYaml, "store leaf " << pathname << ": " << Util::print(n, Config::kShallow));
    if (n.IsDefined()) {
      conf.warn_if_deprecated();
      if (n.IsNull()) {
        if (opt.is_implicit()) try {
            opt.apply_implicit_value(pval, pathname, warn);
          } catch (std::exception& e) {
            trouble(pathname, opt.get_is(pval),
                    "implicit (null) value error (find a programmer; this is a bug)", e.what(), "");
          }
        else
          fatal(pathname, opt.get_is(pval), "null value (for non-implicit/flag)", "");
      } else {
        try {
          if (!n.IsScalar()) throw ConfigException(pathname + " expected a YAML scalar");
          YamlScalarRef scalar = n.Scalar();
          conf.opt->apply_string_value(scalar, pval, pathname, warn);
        } catch (std::exception& e) {
          fatal(pathname, opt.get_is(pval), "Error storing value from yaml config: '" + to_string(n) + "'",
                e.what());
        }
      }
    } else {
      if (opt.is_required())
        trouble(pathname, opt.get_is(pval), "missing required value", "", opt.is_required_err());
    }
    // TODO: check and store allow_undefined (iterate over existing keys rather than looking up only specified
    // ones)
  }

  /**  Since we only overrode for store_action, this macro forwards for all other actions (since we hid the
   * base-class functions of the same name). */
  FORWARD_BASE_CONFIGURE_ACTIONS(Base)
  // TODO: separate backend for building actual YAML tree ?
};

/** perform config action by binding yamlRoot into a ConfigureYaml, and calling
it on pRootVal. (yamlRoot is only relevant for store_action). */
template <class Action, class Val>
void configureYaml(Action action, ConfigNode const& yamlRoot, Val* pRootVal,
                   std::string const& component = "configure") {
  StringConsumer warn = Util::logWarning(component);
  ConfigureYaml cy(yamlRoot, warn);
  configure::configure_action(cy, action, pRootVal, warn);
}

/** print help for pRootVal and all its configurable sub-ojects. vaguely YAML-like output */
template <class Val>
void showHelp(std::ostream& o, Val* pRootVal, std::string const& component = "configure", int verbosity = 0) {
  // configure::help(o, pRootVal, Util::logWarning(component));
  StringConsumer warn = Util::logWarning(component);
  ConfigureYaml cy(warn);
  cy.verbose_max = verbosity;
  configure::configure_action(cy, configure::help_config(o), pRootVal, warn);
}

/** print example configuration (with placeholder values). YAML-like output. */
template <class Val>
void showExample(std::ostream& o, Val* pRootVal, std::string const& component = "configure", bool init = true) {
  StringConsumer warn = Util::logWarning(component);
  if (init) configure::configure_init(pRootVal, warn);
  configure::show_example(pRootVal, warn);
}

template <class Configurable>
inline std::ostream& help(std::ostream& out, std::string const& component = "configure", int verbosity = 0) {
  Configurable c;  // needed to show self_init default
  showHelp(out, &c, component, verbosity);
  return out;
}

template <class Configurable>
inline std::ostream& example(std::ostream& out, std::string const& component = "configure", int verbosity = 0) {
  Configurable c;  // needed to show self_init default
  showExample(out, &c, component, verbosity);
  return out;
}

/** print effective configuration. YAML-like output. ideally this can be parsed
    as a YAML tree and passed to applyYaml to initialize another Val in exactly
    the same way.

    call after you've initialized values via applyYaml or otherwise */
template <class Val>
void showEffective(std::ostream& o, Val const* pRootVal, std::string const& component = "configure") {
  //  configure::show_effective(o, pRootVal, Util::logWarning(component));
  StringConsumer warn = Util::logWarning("effective." + component);
  ConfigureYaml cy(warn);
  configure::configure_action(cy, configure::show_effective_config(o), const_cast<Val*>(pRootVal), warn);
}

/** string return instead of printing with showEffective. */
template <class Val>
std::string getEffective(Val* pRootVal, std::string const& component = "configure") {
  std::ostringstream o;
  showEffective(o, pRootVal, component);
  return o.str();
}

/** TODO: return a YAML tree instead of printing with showEffective. */
template <class Val>
ConfigNode effectiveYaml(Val* pRootVal, std::string const& component = "configure") {
  return ConfigNode();  // TODO
}

/** TODO: return a YAML tree instead of printing with showExample. */
template <class Val>
ConfigNode exampleYaml(Val* pRootVal, std::string const& component = "configure") {
  return ConfigNode();  // TODO
}
}

template <class Val>
struct PrintEffective {
  Val const& val;
  std::string component;
  PrintEffective(Val const& val, char const* component) : val(val), component(component) {}
  PrintEffective(Val const& val, std::string const& component) : val(val), component(component) {}
  friend inline std::ostream& operator<<(std::ostream& out, PrintEffective const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const { Config::showEffective(out, &val, component); }
};

template <class Val>
PrintEffective<Val> printEffective(Val const& val, char const* component) {
  return PrintEffective<Val>(val, component);
}

template <class Val>
PrintEffective<Val> printEffective(Val const& val) {
  return PrintEffective<Val>(val, "configure");
}

template <class Val>
PrintEffective<Val> printEffective(Val const& val, std::string const& component) {
  return PrintEffective<Val>(val, component);
}

struct PrintEffectiveYamlConfigurable {
  YamlConfigurable const& val;
  int verbosity;
  explicit PrintEffectiveYamlConfigurable(YamlConfigurable const& val, int verbosity = 0)
      : val(val), verbosity(verbosity) {}
  friend inline std::ostream& operator<<(std::ostream& out, PrintEffectiveYamlConfigurable const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const { val.showEffective(out, verbosity); }
};

inline PrintEffectiveYamlConfigurable printEffective(YamlConfigurable const& val, char const* = 0) {
  return PrintEffectiveYamlConfigurable(val);
}

inline PrintEffectiveYamlConfigurable printEffective(YamlConfigurable const& val, std::string const&) {
  return PrintEffectiveYamlConfigurable(val);
}

namespace Config {

#include <graehl/shared/warning_push.h>
GCC_DIAG_IGNORE(uninitialized)

/** applyYaml(yamlRootNode, &rootConfiguration) will initialize
    rootConfiguration, update its values as specified by yamlRootNode, and
    validate them. */
template <class Val>
void applyYaml(ConfigNode const& yamlRoot, Val* pRootVal, char const* component = "configure",
               bool init = true, bool validate = true, bool verbose = true, bool trace = true) {
  StringConsumer warn = Util::logWarning(component);
  LOG_TRACE_NAMESTR(kLogPrefix + component, "applyYaml node:\n" << Util::print(yamlRoot, Config::oneline));
  if (init) configure::configure_init(pRootVal, warn);

  if (trace) {
    // TODO: fix gcc warning: ‘<anonymous>’ may be used uninitialized in this function:
    LOG_TRACE_NAMESTR(kLogPrefix + component, "before store for " << component << ":\n"
                                                                  << printEffective(*pRootVal, component)
                                                                  << "\n");
  }

  configureYaml(configure::store_config(), yamlRoot, pRootVal, component);
  if (trace) {
    LOG_TRACE_NAMESTR(kLogPrefix + component,
                      "after store for " << component << ":\n" << printEffective(*pRootVal, component) << "\n");
  }

  if (validate) configure::validate_stored(pRootVal, warn);
  if (verbose)
    LOG_INFO_NAMESTR(kLogPrefix + component, "effective (YAML applied) configuration for "
                                                 << component << ":\n" << printEffective(*pRootVal, component)
                                                 << "\n");
}

template <class Val>
void applyYaml(ConfigNode const& yamlRoot, Val* pRootVal, std::string const& component, bool init = true,
               bool validate = true, bool verbose = true) {
  applyYaml(yamlRoot, pRootVal, component.c_str(), init, validate, verbose);
}

#include <graehl/shared/warning_pop.h>

}  // ns

/** usage: struct MyModule : YamlConfigurable<MyModule, ModuleConfig>

    or Base = ResourceConfig

    (note: both ModuleConfig and ResourceConfig are YamlConfigurable)
*/
template <class Impl, class Base = YamlConfigurable>
struct YamlConfigure : Base {
 private:
  Impl* pimpl() { return static_cast<Impl*>(this); }
  Impl* pimpl() const { return const_cast<Impl*>(static_cast<Impl const*>(this)); }
  StringConsumer log() const { return Base::log(); }

 public:
  virtual void help(std::ostream& out, int verbosity) const OVERRIDE {
    Config::ConfigureYaml cy(log(), verbosity);
    Config::configure_action(cy, Config::help_config(out), pimpl(), log());
  }
  virtual void showExample(std::ostream& out, int verbosity) const OVERRIDE {
    Config::ConfigureYaml cy(log(), verbosity);
    Config::configure_action(cy, Config::show_example_config(out), pimpl(), log());
  }
  virtual void showEffective(std::ostream& out, int verbosity) const OVERRIDE {
    Config::ConfigureYaml cy(log(), verbosity);
    Config::configure_action(cy, Config::show_effective_config(out), pimpl(), log());
  }

  virtual void init() OVERRIDE { configure::configure_init(pimpl(), log()); }
  virtual void validateStored() OVERRIDE { Config::validate_stored(pimpl(), log()); }
  virtual void store(ConfigNode const& configNode) OVERRIDE {
    Config::ConfigureYaml cy(configNode, log());
    Config::configure_action(cy, Config::store_config(), pimpl(), log());
  }
};


}

#endif
