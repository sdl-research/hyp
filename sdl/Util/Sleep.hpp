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

#include <boost/thread/thread.hpp>

#ifndef SDL_POSIX_TIME_MICROSLEEP
# ifdef _MSC_VER
# define SDL_POSIX_TIME_MICROSLEEP 1
/**
   must be 1 for vs2010 or else this expression fails
   (nanosecs_per_tic <= 0.0L) ||
   (!boost::detail::winapi::QueryPerformanceCounter( &pcount ))
*/
# else
# define SDL_POSIX_TIME_MICROSLEEP 0
// 1 is needed for older boost (e.g. 1.49)
#endif
#endif

#if SDL_POSIX_TIME_MICROSLEEP
# include <boost/date_time/posix_time/posix_time.hpp>
#else
# include <boost/chrono/duration.hpp>
#endif

namespace sdl { namespace Util {


inline void microSleep(std::size_t microsecondsToSleep = 1e5)
{
  if (microsecondsToSleep)
#if SDL_POSIX_TIME_MICROSLEEP
    boost::this_thread::sleep(
        boost::posix_time::microseconds(microsecondsToSleep));
#else
  boost::this_thread::sleep_for (
      boost::chrono::microseconds(microsecondsToSleep));
#endif
}

inline std::size_t clampedSize(double x) {
  return x > (double)(std::size_t)-2 ? (std::size_t)-2 : (std::size_t)x;
}

double const kSecondsToMicroseconds = 1e6;
double const kMicrosecondsToSeconds = 1e-6;

inline void sleepSeconds(double seconds) {
  microSleep(clampedSize(kSecondsToMicroseconds * seconds));
}

}}

#endif
