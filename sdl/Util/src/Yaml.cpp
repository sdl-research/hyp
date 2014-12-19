#include <map>
#include <cassert>

#include <sdl/Util/Yaml.hpp>
#include <sdl/Util/LogHelper.hpp>

namespace sdl {
namespace Util {

/**
   Not very efficient if the YAML nodes contain maps;
   implemented for unit tests.
*/
bool yamlNodesEqualByValue(YAML::Node const& n1, YAML::Node const& n2) {
  if (n1.Type() != n2.Type()) {
    SDL_DEBUG(Yaml.yamlNodesEqualByValue, "node types differ: '" << n1 << "' vs '" << n2 << "'");
    return false;
  }

  if (n1.IsMap()) {
    if (n1.size() != n2.size()) return false;
    YAML::const_iterator it1 = n1.begin();
    YAML::const_iterator it2 = n2.begin();
    typedef std::map<std::string, YAML::Node> Map;
    Map sorted1, sorted2;
    for (; it1 != n1.end(); ++it1, ++it2) {
      // YAML map keys are unique
      assert(sorted1.find(it1->first.Scalar()) == sorted1.end());
      assert(sorted2.find(it2->first.Scalar()) == sorted2.end());
      sorted1[it1->first.Scalar()] = it1->second;
      sorted2[it2->first.Scalar()] = it2->second;
    }
    Map::const_iterator it1b = sorted1.begin();
    Map::const_iterator it2b = sorted2.begin();
    assert(sorted1.size() == sorted2.size());
    for (; it1b != sorted1.end(); ++it1b, ++it2b) {
      if (it1b->first != it2b->first || !yamlNodesEqualByValue(it1b->second, it2b->second)) { return false; }
    }
  } else if (n1.IsSequence()) {
    if (n1.size() != n2.size()) return false;
    YAML::const_iterator it1 = n1.begin();
    YAML::const_iterator it2 = n2.begin();
    for (; it1 != n1.end(); ++it1, ++it2) {
      if (!yamlNodesEqualByValue(*it1, *it2)) { return false; }
    }
  } else if (n1.IsScalar()) {
    if (n1.Scalar() != n2.Scalar()) {
      SDL_DEBUG(Yaml.yamlNodesEqualByValue, "node scalar values differ: '" << n1 << "' vs '" << n2 << "'");
      return false;
    }
  } else { SDL_THROW_LOG(Yaml, InvalidInputException, "Unknown node type: " << n1.Type()); }
  return true;
}


}}
