#include <map>
#include <cassert>





namespace Util {

/**


*/
bool yamlNodesEqualByValue(YAML::Node const& n1, YAML::Node const& n2) {
  if (n1.Type() != n2.Type()) {

    return false;
  }

  if (n1.IsMap()) {

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

    }


    YAML::const_iterator it1 = n1.begin();
    YAML::const_iterator it2 = n2.begin();
    for (; it1 != n1.end(); ++it1, ++it2) {

    }

    if (n1.Scalar() != n2.Scalar()) {

      return false;
    }

  return true;
}



