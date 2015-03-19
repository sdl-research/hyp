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
