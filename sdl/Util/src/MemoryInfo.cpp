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
#include <sdl/Exception.hpp>
#include <sdl/Util/Sprintf.hpp>
#include <iostream>
#include <cstddef>  // size_t
#include <stdexcept>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <sdl/LexicalCast.hpp>
#include <boost/any.hpp>
#include <sdl/Types.hpp>
#include <sdl/Util/MemoryInfo.hpp>
#include <sdl/Util/Debug.hpp>
#include <sdl/Util/ThreadSpecific.hpp>
#include <sdl/Util/LogHelper.hpp>


#ifdef _WIN32
#include <process.h>
#define getpid _getpid
#else
#include <sys/resource.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#endif
#if __linux__
#include <sys/types.h>
#include <unistd.h>
// man -2 getpid
#endif

#if defined(__MACH__) || defined(__FreeBSD__) || defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

namespace sdl {
namespace Util {

double physicalMemoryBytes() {
// TODO: test
#if defined(_SC_PHYS_PAGES) && defined(_SC_PAGESIZE)
  {
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);
    if (pages != -1 && page_size != -1) return (double)pages * (double)page_size;
  }
#endif
#ifdef HW_PHYSMEM
  {
    unsigned physmem;
    size_t len = sizeof physmem;
    static int mib[2] = {CTL_HW, HW_PHYSMEM};

    if (sysctl(mib, sizeof(mib) / sizeof(mib[0]), &physmem, &len, NULL, 0) == 0 && len == sizeof(physmem))
      return (double)physmem;
  }
#endif
  // TODO: windows
  return 0;
}


namespace {
double const kMB = 1024. * 1024.;
double const kGB = 1024. * kMB;
double const kOneOverMB = 1. / kMB;
double const kOneOverGB = 1. / kGB;
}

double physicalMemoryGB() {
  return physicalMemoryBytes() * kOneOverGB;
}

MemoryInfo::MemoryInfo(MemoryInfo const&) {
  throw UnimplementedException("no copy - use MemoryInfo::instance().getSizeInMB())");
}

/**
   TODO Make this work on non-unix platforms too
*/
MemoryInfo::MemoryInfo() {
  C99snprintf(memoryFilename, buflen - 1, "/proc/%d/stat", getpid());
}

MemoryInfo MemoryInfo::instance_;

#ifdef _WIN32
std::size_t MemoryInfo::getSize() {
  SDL_TRACE(MemoryInfo, "MemoryInfo::getSize() not yet supported on Windows.");
  return 0;
}
#elif __APPLE__
std::size_t MemoryInfo::getSize() {
  SDL_TRACE(MemoryInfo, "MemoryInfo::getSize() not yet supported on Apple.");
  return 0;
}
#else
std::size_t MemoryInfo::getSize() {
  // unsigned long long memoryUsage;
  std::string line;
  SDL_DEBUG(Util.MemoryInfo, "reading " << memoryFilename);
  std::ifstream memoryFileStream(memoryFilename);
  if (!memoryFileStream)
    SDL_THROW_LOG(Util.MemoryInfo, FileException, "couldn't open process stat file '"
                                                      << memoryFilename << "' for memory usage");
  std::getline(memoryFileStream, line);
  const std::string val0 = getColumn(line, 22);  // virtual memory size column
  if (val0.empty()) {  // TODO@MD: Fix for non-linux systems
    return 0;
  }
  std::size_t val = sdl::lexical_cast<std::size_t>(val0);
  return val;
}
#endif

double MemoryInfo::getSizeInMB() {
  // TODO: test
  return getSize() * kOneOverMB;
}

double MemoryInfo::getSizeInGB() {
  // TODO: test
  return getSize() * kOneOverGB;
}

/**
   Returns the specified column of a string.
   (columns are blank-separated, counting from 0)
*/
std::string MemoryInfo::getColumn(std::string const& s, unsigned columnNumber) {
  std::string::size_type start = 0;
  while (columnNumber-- > 0) {
    start = s.find(' ', start) + 1;
  }
  return s.substr(start, s.find(' ', start + 1) - start);
}


}}
