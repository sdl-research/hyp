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

    abstraction: you have a yaml config node/file that's a map. keys are:

    keyword name:
    type: t
    ...

    we call appropriate factories for each keyword.

    actually, yaml config file / basis expansion translates this to:

    name:
    category: keyword
    type: t
    ...

    the result is a set of variables of dynamic type (unlike a regular single
    configurable object, which predeclares a list of names, or else stores things
    as a map

    alternatively, we could create a single Dynamic<TypeTag> type that reads the
    category/type fields and initializes itself (boost::any style). TypeTag might
    be needed to get thread-local access to context; otherwise we could omit it.

    difficulty is that we haven't defined a fully type-erased configure-backend
    type object (and we're not really declarative / reflective, with an explicit
    tree of C++ typeinfos for dispatch). we did some work toward this with the
    configure::configurable interface and configure::configure_any-we just want a
    factory-like equivalent (that doesn't require an already constructed object to
    put in the configure_any)
*/

#ifndef KEYWORDCONFIG_JG_2013_08_22_HPP
#define KEYWORDCONFIG_JG_2013_08_22_HPP
#pragma once

#include <sdl/Config/Config.hpp>
#include <sdl/Config/ConfigureYaml.hpp>
#include <sdl/Config/Named.hpp>
#include <sdl/Util/Delete.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Map.hpp>
#include <sdl/Util/String.hpp>
#include <sdl/Util/VoidIf.hpp>
#include <sdl/Config-fwd.hpp>
#include <sdl/SharedPtr.hpp>
#include <boost/any.hpp>

namespace sdl {
namespace Config {


/// TODO: type safety improvement - boost::any containing shared_ptr<Interace *>
/// depending on Category? or dynamic_pointer_cast (common base)
typedef shared_ptr<void> VoidPtr;

// TODO: use YamlConfigurable from Config-already has name / category / type
struct ConfigurableFactory {
  typedef VoidPtr result_type;
  virtual VoidPtr fromConfig(std::string const& name, ConfigNode const& cfg) = 0;
  virtual ~ConfigurableFactory() {}
};

/// typename, yaml config -> shared_ptr<something> (dynamic cast to actual known type for category keyword)
typedef function<VoidPtr(std::string const&, ConfigNode const&)> VoidFactoryFn;

struct VoidFactory : VoidFactoryFn {
  std::string description;

  VoidFactory() {}

  template <class Fn>
  VoidFactory(Fn const& fn, std::string const& description) : VoidFactoryFn(fn), description(description) {}

  VoidFactory(VoidFactory const& o) : VoidFactoryFn(o), description(o.description) {}
};

typedef std::string Category;
typedef std::string Type;
typedef std::string InstanceName;

// TODO: category could be members of boost::mpl list, and looked up through mpl map - then we wouldn't need
// VoidPtr

typedef std::pair<Category, Type> CategoryType;

template <class Config>
inline CategoryType categoryType(Config const& c = Config()) {
  return CategoryType(c.category(), c.type());
}

inline std::ostream& operator<<(std::ostream& out, CategoryType const& ct) {
  return out << ct.first << " " << ct.second;
}

inline std::string str(CategoryType const& ct) {
  return ct.first + "." + ct.second;
}

template <class Impl>
struct DescribeFromConfig {
  std::string description;

  DescribeFromConfig(std::string const& type) {
    std::ostringstream out;
    Config::help<Impl>(out, type);
    description = out.str();
  }

  operator std::string const&() const { return description; }
};

/**
   Impl should have string Impl::Config with static string
   category()/type()/usage() and member setName(string) - e.g. subclass Nameless
   or Named
*/
template <class Impl, class Enable = void>
struct NewFromConfig {
  VoidPtr operator()(std::string const& name, ConfigNode const& cfg) {
    Util::AutoDelete<Impl> pimpl(new Impl);
    Impl& impl = *pimpl;
    impl.setName(name);
    applyYaml(cfg, &impl, str(categoryType(impl)) + "." + name);
    return VoidPtr(pimpl.release());
  }
};


/// alternative: Options with Transform satisfying above 'Impl' requirements and
/// w/ constructor taking shared_ptr<Options const>
template <class Options>
struct NewFromConfig<Options, typename Util::VoidIf<typename Options::Transform>::type> {
  VoidPtr operator()(std::string const& name, ConfigNode const& cfg) {
    shared_ptr<Options> popt(new Options);
    Options& opt = *popt;
    std::string logname(opt.type());
    logname.push_back('.');
    logname += name;
    applyYaml(cfg, &opt, logname);
    typedef typename Options::Transform Impl;
    Util::AutoDelete<Impl> pimpl(new Impl(popt));
    pimpl->setName(name);
    return VoidPtr(pimpl.release());
  }
};

static std::string const kType = "type";

struct CategoryTypeFactories {
  typedef std::map<CategoryType, VoidFactory> Factories;
  typedef std::set<Category> Categories;
  Factories factories;
  Categories categories;

  /**
     \return factory else throw ConfigException
  */
  VoidFactory const& getFactoryRequired(CategoryType const& categoryType) const;

  bool getFactory(CategoryType const& categoryType, VoidFactory& factory) const {
    Factories::const_iterator i = factories.find(categoryType);
    if (i == factories.end())
      return false;
    else {
      factory = i->second;
      return true;
    }
  }

  /**
     \return new instance else null

     \param[out] name name of instance if not null

     categoryName is 'category name: ' key with value node (that contains a 'type: type' entry we strip)

  */
  VoidPtr makeInstance(std::string const& categoryName, ConfigNode const& node, std::string& name) const;

  template <class FactoryFn>
  void setFactory(Category const& category, Type const& type, FactoryFn const& factory,
                  std::string const& description) {
    factories[CategoryType(category, type)] = VoidFactory(factory, description);
    categories.insert(category);
  }

  template <class Impl>
  void setNewFactory(Category const& category, Type const& type) {
    DescribeFromConfig<Impl> describe((type));
    setFactory(category, type, NewFromConfig<Impl>(), describe.description);
  }

  typedef std::ostream HelpOut;
  void list(HelpOut&) const;
  void list(HelpOut&, Category const& category) const;

  /// return whether categoryType is registered, and if so, print description to out
  bool describe(HelpOut& out, CategoryType const& categoryType) const;

  /// \return whether any describe(out, (category, type)) for all category
  bool describeAnyCategory(HelpOut& out, std::string const& type) const;

  void checkCategory(Category const& category) const;
};

template <class Impl>
inline char const* categoryFor() {
  return ((Impl const*)0)->category();
}

/**
   the (untyped) variables
*/
struct Environment {
  typedef std::map<InstanceName, VoidPtr> Variables;
  Variables variables;

  template <class Impl>
  shared_ptr<Impl> getPtr(InstanceName const& name) {
    VoidPtr r;
    Util::maybeGet(variables, name, r);
    return static_pointer_cast<Impl>(r);
  }

  template <class Impl>
  shared_ptr<Impl> getRequiredPtr(InstanceName const& name) const {
    VoidPtr r;
    Util::maybeGet(variables, name, r);
    if (!r)
      SDL_THROW_LOG(Config.Keyword.Environment, ConfigException, "no " << categoryFor<Impl>() << " instance '"
                                                                       << name << "'");
    return static_pointer_cast<Impl>(r);
  }

  template <class Impl>
  void getPtr(InstanceName const& name, shared_ptr<Impl>& p) {
    VoidPtr r;
    Util::maybeGet(variables, name, r);
    p = static_pointer_cast<Impl>(r);
  }

  template <class Impl>
  void getRequiredPtr(InstanceName const& name, shared_ptr<Impl>& p) const {
    VoidPtr r;
    Util::maybeGet(variables, name, r);
    if (!r)
      SDL_THROW_LOG(Config.Keyword.Environment, ConfigException, "no " << categoryFor<Impl>() << " instance '"
                                                                       << name << "'");
    p = static_pointer_cast<Impl>(r);
  }

  template <class Impl>
  Impl& getRequired(InstanceName const& name) const {
    Variables::const_iterator i = variables.find(name);
    if (i == variables.end())
      SDL_THROW_LOG(Config.Keyword.Environment, ConfigException, "no " << categoryFor<Impl>() << " instance '"
                                                                       << name << "'");
    return *(Impl*)i->second;
  }
  /**
     store in unrecognized (if not null) all the unknown-keyword and non-keyword keys.
  */
  void configurePartial(ConfigNode const& root, ConfigNode* unrecognized, CategoryTypeFactories const& factories) {
    std::string categoryName, instanceName;
    for (YAML::const_iterator i = root.begin(), e = root.end(); i != e; ++i) {
      std::string const& key = i->first.Scalar();
      ConfigNode const& val = i->second;
      if (Util::split2(key, categoryName, instanceName)) {
        VoidPtr instance = factories.makeInstance(key, val, instanceName);
        if (instance)
          variables[instanceName] = instance;
        else
          SDL_THROW_LOG(Config, ConfigException, "couldn't create " << key << " from config:\n" << val);
      } else if (unrecognized)
        Config::addKeyVal(*unrecognized, key, val);
      else
        SDL_THROW_LOG(Config, ConfigException, "unrecognized " << key << ":\n" << val);
    }
  }
};


}}

#endif
