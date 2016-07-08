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

    std::quick_exit or normal return rc; from main()
*/

#ifndef QUICKEXIT_JG_2014_12_16_HPP
#define QUICKEXIT_JG_2014_12_16_HPP
#pragma once

#ifndef SDL_PREFER_QUICK_EXIT
#define SDL_PREFER_QUICK_EXIT 0
#endif

#include <cstdlib>
#if (defined(_WIN32) || defined(__WIN32__) || defined(WIN32)) && !defined(__CYGWIN__)
#include <stdlib.h>
#else
#include <unistd.h>
#endif
#include <iostream>
#include <utility>

namespace sdl {
namespace Util {

/**
    exit without calling static or local destructors

    so if you use this make sure you flushed output sockets/files you care about.

    we flush cout/cerr for you. if you were using cstdio (or fstream etc)
    instead you have to close/flush that yourself
*/
[[noreturn]] inline void quickExit(int rc = 0) {
  std::cout.flush();
  std::cerr.flush();
  using namespace std;
#if __APPLE__
  //TODO: std::quick_exit not found gcc-6.1 -std=c++14
  std::exit(rc);
#else
  std::quick_exit(rc);  // linux: _exit(rc)
#endif
}

[[noreturn]] inline void quickAbort(int rc = 1) {
  quickExit(rc);
}


/** usage: from int main():

    return normalExit(rc);

*/
inline int normalExit(int rc = 0) {
#if SDL_PREFER_QUICK_EXIT
  quickExit(rc);
#endif
  /* we were calling std::exit; however, this lead to double calling of .so destructors because of exit():

     #1  0x00007fffdc93ee69 in __run_exit_handlers () from /lib64/libc.so.6

     and libxmt_shared.so unload destruction:

#1  0x00007fffdc93f1da in __cxa_finalize () from /lib64/libc.so.6
#2  0x00007fffe4815e46 in __do_global_dtors_aux () from /home/graehl/x/Debug/xmt/lib/libxmt_shared_debug.so

 for example extern global defined once in libsdl-Db.so of sdl::BdbIndexTypeNames::~BdbIndexTypeNames()

 therefore in your main() you should return normalExit(rc);
  */
  return rc;
}


}}

#endif
