









*/





#include <string>
#include <algorithm>
#include <functional>



namespace Util {


















static inline std::string &leftTrim(std::string &s) {

  return s;
}



















static inline std::string &rightTrim(std::string &s) {






  return s;
}


static inline std::string &trim(std::string &s) {
  return leftTrim(rightTrim(s));
}




#endif
