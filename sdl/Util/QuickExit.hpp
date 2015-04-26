// Copyright 2014 SDL plc
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

 exit without calling static destructors.

 //TODO: std::quick_exit from latest standard
*/

#ifndef QUICKEXIT_JG_2014_12_16_HPP
#define QUICKEXIT_JG_2014_12_16_HPP
#pragma once

#if (defined(_WIN32) || defined(__WIN32__) || defined(WIN32)) && !defined(__CYGWIN__)
# include <stdlib.h>
#else
# include <unistd.h>
#endif
#include <iostream>

namespace sdl { namespace Util {

inline void quickExit(int rc = 0) {
  std::cout.flush();
  std::cerr.flush();
  using namespace std;
  _exit(rc);
}


}}

#endif
