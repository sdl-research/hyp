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

    fwd decl of sdl::ConfigNode and other Config/ConfigureYaml.hpp
*/

#ifndef SDL_CONFIG_FWD_HPP
#define SDL_CONFIG_FWD_HPP
#pragma once

#include <sdl/SharedPtr.hpp>
#include <iostream>
#include <map>
#include <string>

/**
   these to/from string methods need to be found by ADL on YAML::Node.
*/
namespace YAML {

class Node;
/** Parseable YAML string. In same namespace as Node so you can unqualified from any namespace.

    \param in YAML node

    \return string 'in'

      This method prints YAML nodes in a more readable format.
      - Sequences are always printed within [] and on a single line.
      - If called with longMap & multiLine set to true, YAML Maps are scoped in { }.
        example:
         {
           foo: {
             bar: buz,
             hello: world!,
           },
           test-path: [../a.in, b.in, c.in]
         }
   *
      - If longMap is set to false & multiLine to true the { are not printed and the output is more concise
   and readable.
        example:
         foo:
           bar: buz
           hello: world!
        test-path: [../a.in, b.in, c.in]
   *
      - If called with longMap set to true & multiLine to false, the entire YAML node is printed on a single
   line,
        e.g:
         { foo: { bar: buz, hello: world! }, test-path: [../a.in, b.in, c.in] }
   *
      - If called with longMap & multiLine set to false, longMap gets reset to true as this is invalid config.
   and the
        output is same as above.
 */
std::string to_string(Node const& in, bool longMap = true, bool newLine = false,
                      std::string const& indent = "");

/**
   dest=parse(str) - parses str as yaml Node (without the Config.hpp transformations).
*/
void string_to_impl(std::string const& str, Node& dest);
}

namespace sdl {

typedef YAML::Node
    ConfigNode;  // these should be passed by value. they're handles. but is there copy on write?
class YamlConfigurable;
typedef shared_ptr<YamlConfigurable> YamlConfigurablePtr;

/// for printing effective values in logging stream output macros without converting to string first
struct YamlConfigurableEffective {
  YamlConfigurable const& configurable;
  int verbosity;
  YamlConfigurableEffective(YamlConfigurable const& configurable, int verbosity = 0)
      : configurable(configurable), verbosity(verbosity) {}
  YamlConfigurableEffective(YamlConfigurable const* configurable, int verbosity = 0)
      : configurable(*configurable), verbosity(verbosity) {}
  friend inline std::ostream& operator<<(std::ostream& out, YamlConfigurableEffective const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const;
};


}

#endif
