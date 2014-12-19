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
