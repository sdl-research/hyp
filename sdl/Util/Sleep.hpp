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

    sleep some number of seconds or microseconds.
*/

#ifndef SLEEP_JG_2014_05_21_HPP
#define SLEEP_JG_2014_05_21_HPP
#pragma once

#include <thread>

namespace sdl {
namespace Util {


inline void usSleep(std::size_t microsecondsToSleep = 1e5) {
  if (microsecondsToSleep) std::this_thread::sleep_for(std::chrono::microseconds(microsecondsToSleep));
}


inline void msSleep(std::size_t millisecondsToSleep = 1e2) {
  if (millisecondsToSleep) std::this_thread::sleep_for(std::chrono::milliseconds(millisecondsToSleep));
}

/// use sleepSeconds instead for fractional seconds
inline void sSleep(std::size_t wholeSeconds = 1) {
  if (wholeSeconds) std::this_thread::sleep_for(std::chrono::seconds((std::size_t)wholeSeconds));
}

inline std::size_t clampedSize(double x) {
  return x > (double)(std::size_t)-2 ? (std::size_t)-2 : (std::size_t)x;
}

double constexpr kSecondsToMicroseconds = 1e6;
double constexpr kMicrosecondsToSeconds = 1e-6;

/// note: fractional seconds.
inline void sleepSeconds(double secondsToSleep) {
  return usSleep(secondsToSleep * kSecondsToMicroseconds);
}


}}

#endif
