/** \file

    before C++11, use swap to move-assign.

*/

#ifndef MOVE_JG_2014_06_13_HPP
#define MOVE_JG_2014_06_13_HPP
#pragma once

#include <algorithm>
#include <utility>

namespace sdl { namespace Util {

template <class T>
void adlSwap(T &to, T &from) {
  using namespace std;
  swap(to, from); //TODO: C++11
}

template <class T>
void moveAssign(T &to, T &from) {
#if __cplusplus >= 201103L || CPP11
  to = std::move(from);
#else
  adlSwap(to, from);
#endif
}


}}

#endif
