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

    elapsed time from boost cpu timers

    see Util/Performance.hpp for CpuTimes Util::cpuTimesNow().
*/

#ifndef CPUTIMES_JG_2014_01_15_HPP
#define CPUTIMES_JG_2014_01_15_HPP
#pragma once

#include <boost/timer/timer.hpp>
#include <cassert>

namespace sdl {

typedef boost::timer::nanosecond_type Nanoseconds;

/**
   cpu_timer and auto_cpu_timer obtain Wall-clock timings from Boost.Chrono's
   high_resolution_clock. On Intel compatible CPU's running Windows, Linux, and
   Mac OS X, this is a "steady clock" [C++11 20.11.3], but may not be steady on
   other platforms. cpu_timer_info.cpp reports whether or not the
   high_resolution_clock is steady on a particular platform.

   (so changing system clock won't throw off measured durations)
*/

typedef boost::timer::cpu_times CpuTimes;
typedef boost::timer::cpu_timer CpuTimer;

/// TODO: there might be some way to derive these constants (max signed int
/// value for #bytes) in a way that would work for templated int types
Nanoseconds const kNanosecondLsbs = 0x7FFFFFFFFFFFFFFFLL;
Nanoseconds const kNanosecondMsb = ~kNanosecondLsbs;

/**
   \return (finish - start) - nanoseconds elapsed start older time 'start',
   protecting against the (every ~500 years) possibility that the clock wrapped
   back around to 0 between start and finish - see TestUtil.cpp for demonstration

   Amos observes empirically that CpuTimer seems to start at 0 for the process;
   if that's guaranteed then you could safely just use (finish - start) for over
   500 years of uptime
*/
inline Nanoseconds elapsed(Nanoseconds finish, Nanoseconds start) {
  Nanoseconds r = finish - start;
  if (r & kNanosecondMsb) r -= kNanosecondMsb;
  assert((r & kNanosecondLsbs) == r);
  // correct for overflow - our server may run indefinitely but no request
  // should take more than 8 giga-seconds
  return r;
}

/**
   note: kUser and kSystem might be per-process totals so shouldn't be summed
   across threads; wall time extents have a sort of meaning for multiple
   threads, though.
*/
enum CpuTimeType { kWall, kUser, kSystem, knCpuTimeTypes };

/**
   \return elapsed time in nanoseconds, of type kWall kUser or kSystem.
*/
inline Nanoseconds nanoseconds(CpuTimes const& times, CpuTimeType type = kWall) {
  assert(type < knCpuTimeTypes);
  switch (type) {
    case kWall:
      return times.wall;
    case kUser:
      return times.user;
    case kSystem:
      return times.system;
    default:
      return -1;  // impossible
  }
}

double const kNanosecondsToSeconds = 1e-9;

inline double seconds(Nanoseconds ns) {
  return kNanosecondsToSeconds * ns;
}

inline double seconds(CpuTimes const& times, CpuTimeType type = kWall) {
  return seconds(nanoseconds(times, type));
}

inline Nanoseconds elapsedNanoseconds(CpuTimes const& finish, CpuTimes const& start, CpuTimeType type = kWall) {
  return elapsed(nanoseconds(finish, type), nanoseconds(start, type));
}

inline double elapsedSeconds(CpuTimes const& finish, CpuTimes const& start, CpuTimeType type = kWall) {
  return seconds(elapsedNanoseconds(finish, start, type));
}

inline std::string str(CpuTimes const& times) {
  return boost::timer::format(times);
}


}

#endif
