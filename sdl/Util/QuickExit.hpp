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

    std::quick_exit
*/

#ifndef QUICKEXIT_JG_2014_12_16_HPP
#define QUICKEXIT_JG_2014_12_16_HPP
#pragma once

#if (defined(_WIN32) || defined(__WIN32__) || defined(WIN32)) && !defined(__CYGWIN__)
#include <stdlib.h>
#else
#include <unistd.h>
#endif
#include <iostream>

namespace sdl {
namespace Util {

/**
    exit without calling static or local destructors

    so if you use this make sure you flushed output sockets/files you care about.

    we flush cout/cerr for you. if you were using cstdio (or fstream etc)
    instead you have to close/flush that yourself
*/
inline void quickExit(int rc = 0) {
  std::cout.flush();
  std::cerr.flush();
  using namespace std;
#if __cplusplus >= 201103L
  std::quick_exit(rc);
#else
  /// for msvc: _exit Performs quick C library termination procedures,
  /// terminates the process, and exits with the supplied status code.
  _exit(rc);
#endif
}


}}

#endif
