

#include <iostream>
#include <cstddef> // size_t
#include <stdexcept>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include <boost/any.hpp>







#include <process.h>
#define getpid _getpid





#endif












namespace Util {





































MemoryInfo::MemoryInfo(const MemoryInfo&) {

}

/**

*/
MemoryInfo::MemoryInfo() {

}



#ifdef _WIN32
std::size_t MemoryInfo::getSize() {

            "MemoryInfo::getSize() not yet supported on Windows.");

}
#elif __APPLE__
std::size_t MemoryInfo::getSize() {

            "MemoryInfo::getSize() not yet supported on Apple.");
  return 0;
}
#else
std::size_t MemoryInfo::getSize() {
  // unsigned long long memoryUsage;
  std::string line;




  std::getline(memoryFileStream, line);
  const std::string val0 = getColumn(line, 22); // virtual memory size column
  if (val0.empty()) { // TODO@MD: Fix for non-linux systems
    return 0;
  }

  return val;
}
#endif

double MemoryInfo::getSizeInMB() {

}

double MemoryInfo::getSizeInGB() {

}

/**


*/
std::string MemoryInfo::getColumn(const std::string& s, unsigned columnNumber) {
  std::string::size_type start = 0;
  while (columnNumber-- > 0) {
    start = s.find(' ', start) + 1;
  }
  return s.substr(start, s.find(' ', start + 1) - start);
}



