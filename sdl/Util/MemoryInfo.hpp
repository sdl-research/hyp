/** \file

    Usage:

    MemoryInfo::instance().getSize();
    MemoryInfo::instance().getSizeInMB();
    MemoryInfo::instance().getSizeInGB();

    This is useful for debug output; it can also be set to throw an
    exception if a memory limit is reached.

    \author Markus Dreyer
*/

#ifndef SDL_UTIL_MEMORYINFO
#define SDL_UTIL_MEMORYINFO
#pragma once

#include <string>

namespace sdl {
namespace Util {

double physicalMemoryBytes();
double physicalMemoryGB();


class MemoryInfo {
 public:

  // Singleton
  MemoryInfo();
  MemoryInfo(const MemoryInfo&);

  static MemoryInfo instance_;
  static MemoryInfo& instance() { return instance_; };

  std::size_t getSize(); // TODO: change type?
  double getSizeInMB();
  double getSizeInGB();

 private:
  enum { buflen = 64 };

  char memoryFilename[buflen];
  std::string getColumn(const std::string& s, unsigned columnNumber);

};

}}

#endif
