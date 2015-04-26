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

    some more uniform, constant versions of infinity() etc.

    TODO: remove the ones that aren't compile time constants (CM-214)

    (note: in C++11 it's possible to guarantee that they're actually used as
    constants and not by memory load)

*/

#ifndef SDL_UTIL_CONSTANTS_HPP
#define SDL_UTIL_CONSTANTS_HPP
#pragma once

#include <limits>
#include <cstddef>
#include <utility>
#include <algorithm>

namespace sdl {

// TODO: Add namespace Util

// numeric limits class
template <class T>
class FloatLimits {
 public:
  static const T posInfinity;
  static const T negInfinity;
  static const T bad;
  static const T max;
};

template <class T>
const T FloatLimits<T>::posInfinity = std::numeric_limits<T>::infinity();

template <class T>
const T FloatLimits<T>::negInfinity = -FloatLimits<T>::posInfinity;

template <class T>
const T FloatLimits<T>::bad = std::numeric_limits<T>::quiet_NaN();

template <class T>
const T FloatLimits<T>::max = std::numeric_limits<T>::max();

template <class T>
class FloatConstants {
 public:
  static const T epsilon;
};

template <class T>
const T FloatConstants<T>::epsilon = (T)1e-6;

namespace Util {

/**
   \return Returns undef value (by convention, max() value for the
   type T).
*/
template <class T>
T getUndef() {
  return std::numeric_limits<T>::max();
}

/**
   \return whether valOrUndef is 'defined' (not equal to max() value).
*/
template <class T>
bool defined(T valOrUndef) {
  return valOrUndef != getUndef<T>();
}

/**
   \return val1OrUndef if it's 'defined' (not equal to max() value), else orElse.
*/
template <class T>
T definedOr(T val1OrUndef, T orElse) {
  return defined(val1OrUndef) ? val1OrUndef : orElse;
}

template <class T>
void setUnlessDefined(T& val1OrUndef, T orElse) {
  if (!defined(val1OrUndef)) val1OrUndef = orElse;
}

/**
   \return min of whatever of the two values are 'defined' (not equal to max() value)
*/
template <class T>
T minDefined(T val1OrUndef, T val2OrUndef) {
  return val2OrUndef < val1OrUndef ? val2OrUndef : val1OrUndef;
}

/**
   \return max of whatever of the two values are 'defined' (not equal to max() value)
*/
template <class T>
T maxDefined(T val1OrUndef, T val2OrUndef) {
  return defined(val1OrUndef) ? (defined(val2OrUndef) ? std::max(val1OrUndef, val2OrUndef) : val1OrUndef)
                              : val2OrUndef;
}

using sdl::FloatLimits;
using sdl::FloatConstants;


}}

#endif
