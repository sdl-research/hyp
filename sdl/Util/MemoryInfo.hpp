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

    Usage:

    MemoryInfo::instance().size();
    MemoryInfo::instance().sizeInMB();
    MemoryInfo::instance().sizeInGB();

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
  static MemoryInfo& instance() {
    static MemoryInfo instance_;
    return instance_;
  };

  // Disallowed because singleton:
  MemoryInfo(MemoryInfo const&) = delete;  // Copy construct
  MemoryInfo(MemoryInfo&&) = delete;  // Move construct
  MemoryInfo& operator=(MemoryInfo const&) = delete;  // Copy assign
  MemoryInfo& operator=(MemoryInfo&&) = delete;  // Move assign

  std::size_t size();  // TODO: change type?
  double sizeInMB();
  double sizeInGB();


 private:
  enum { buflen = 64 };

  char memoryFilename[buflen];
  std::string getColumn(std::string const& s, unsigned columnNumber);

  // Private because Singleton
  MemoryInfo();
};


}}

#endif
