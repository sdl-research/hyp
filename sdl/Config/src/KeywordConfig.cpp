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
#include <sdl/Config/KeywordConfig.hpp>
#include <sdl/Util/PrintRange.hpp>

namespace sdl {
namespace Config {

void CategoryTypeFactories::list(HelpOut& out) const {
  for (Factories::const_iterator i = factories.begin(), e = factories.end(); i != e; ++i)
    out << i->first << '\n';
}

void CategoryTypeFactories::list(HelpOut& out, Category const& category) const {
  checkCategory(category);
  for (Factories::const_iterator i = factories.begin(), e = factories.end(); i != e; ++i)
    if (i->first.first == category) out << i->first << '\n';
}

bool CategoryTypeFactories::describe(HelpOut& out, CategoryType const& categoryType) const {
  // TODO: return not just factory but also YamlConfigurable default config object for usage, or usage string
  VoidFactory f;
  if (getFactory(categoryType, f)) {
    out << categoryType << ":\n" << f.description << '\n';
    return true;
  } else
    return false;
}

bool CategoryTypeFactories::describeAnyCategory(HelpOut& out, std::string const& type) const {
  bool any = false;
  CategoryType ctype;
  ctype.second = type;
  std::string r;
  for (Categories::const_iterator i = categories.begin(), e = categories.end(); i != e; ++i) {
    ctype.first = *i;
    if (describe(out, ctype)) any = true;
  }
  return any;
}

void CategoryTypeFactories::checkCategory(Category const& category) const {
  if (!Util::contains(categories, category))
    SDL_THROW_LOG(Config.KeywordConfig, ConfigException, "category " << category << " doesn't exist - try "
                                                                     << printer(categories));
}

VoidFactory const& CategoryTypeFactories::getFactoryRequired(CategoryType const& categoryType) const {
  checkCategory(categoryType.first);
  Factories::const_iterator i = factories.find(categoryType);
  if (i == factories.end())
    SDL_THROW_LOG(Config, ConfigException, "No factory for " << categoryType.first << ' ' << categoryType.second);
  return i->second;
}


/**
   \return new instance else null

   \param[out] name name of instance if not null

   categoryName is 'category name: ' key with value node (that contains a 'type: type' entry we strip)

*/
VoidPtr CategoryTypeFactories::makeInstance(std::string const& categoryName, ConfigNode const& node,
                                            std::string& name) const {
  if (!node.IsMap())
    SDL_THROW_LOG(Config, ConfigException, "config:\n"
                                               << node << "\nwasn't a map for " << categoryName << ' ' << name);
  CategoryType categoryType;
  bool haveType = false;
  if (Util::split2(categoryName, categoryType.first, name)) {
    ConfigNode withoutType;
    for (YAML::const_iterator i = node.begin(), e = node.end(); i != e; ++i) {
      std::string const& key = i->first.Scalar();
      ConfigNode const& val = i->second;
      if (key == kType) {
        categoryType.second = val.Scalar();
        haveType = true;
      } else
        Config::addKeyVal(withoutType, key, val);
    }
    if (!haveType || categoryType.second.empty())
      SDL_THROW_LOG(Config, ConfigException, "missing 'type: ...' for " << categoryName << ' ' << name);
    try {
      return getFactoryRequired(categoryType)(name, withoutType);
    } catch (std::exception& e) {
      SDL_THROW_LOG(Configure, ConfigException, "'" << categoryType.first << " " << name << "': " << e.what());
    }
  }
  return VoidPtr();
}


}}
