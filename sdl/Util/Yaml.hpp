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

    deep-compare yaml nodes.
*/

#ifndef SDL_UTIL_YAML_HPP
#define SDL_UTIL_YAML_HPP
#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4146)
#endif

#include <yaml-cpp/yaml.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace sdl {
namespace Util {

bool yamlNodesEqualByValue(YAML::Node const& n1, YAML::Node const& n2);


}}

#endif
