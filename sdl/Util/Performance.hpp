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

    automatic (scoped) elapsed time logging.
*/

#ifndef SDL_UTIL_PERFORMANCE_HPP
#define SDL_UTIL_PERFORMANCE_HPP
#pragma once

//TODO: move impl to .cpp?
#include <iostream>
#include <sdl/Util/CpuTimes.hpp>
#include <sdl/Util/LogInfo.hpp>
#include <sdl/Util/Flag.hpp>
#include <sdl/Util/Debug.hpp>
#include <sdl/Log.hpp>
#include <boost/noncopyable.hpp>

namespace sdl { namespace Util {

CpuTimes cpuTimesNow();

inline double secondsSince(CpuTimes const& since) {
  return seconds(elapsed(cpuTimesNow().wall, since.wall));
}

double processBytes();

enum MeasureProcessMemory { kNoMeasureProcessMemory = 0, kMeasureProcessMemory = 1 };


/** memory usage change and elapsed time */
struct Elapsed
{
  double sec;
  double wallSec;
  double peakBytes;
  double deltaBytes;
  void reset() { sec = peakBytes = deltaBytes = 0; }
  /** used by average per input size */
  Elapsed& operator /=(double per)
  {
    if (per) {
      sec /= per;
      wallSec /= per;
      deltaBytes /= per;
    }
    return *this;
  }

  template <class N>
  Elapsed operator /(N const& n) const {
    Elapsed per=*this;
    per /= n;
    return per;
  }
  /** used to get total elapsed */
  Elapsed& operator +=(Elapsed const& o)
  {
    sec += o.sec;
    wallSec += o.wallSec;
    deltaBytes += o.deltaBytes;
    peakBytes = std::max(peakBytes, o.peakBytes);
    return *this;
  }

  /** difference. for elapsed from current usage */
  Elapsed operator -(Elapsed const& start) const
  {
    return Elapsed(sec-start.sec, wallSec-start.wallSec, peakBytes, peakBytes-start.peakBytes);
  }

  Elapsed(double sec, double wallSec, double peakBytes = 0, double deltaBytes = 0)
      : sec(sec), wallSec(wallSec), peakBytes(peakBytes), deltaBytes(deltaBytes)
  {}


  Elapsed(MeasureProcessMemory measureProcessMemory = kNoMeasureProcessMemory)
      : sec()
      , wallSec()
      , deltaBytes()
  {
    measureMemory(measureProcessMemory);
  }

  void measureElapsed(boost::timer::cpu_timer const &timer) {
    boost::timer::cpu_times elapsed(timer.elapsed());
    const double ns = 1e-9;
    sec = elapsed.user*ns;
    wallSec = elapsed.wall*ns;
  }

  void measureMemory(MeasureProcessMemory measureProcessMemory = kMeasureProcessMemory) {
    measureMemory(measureProcessMemory==kMeasureProcessMemory);
  }

  void measureMemory(bool measureProcessMemory) {
    peakBytes = measureProcessMemory ? processBytes() : 0;
  }

  /**
     initialize with elapsed time from (already created/started) timer.
  */
  Elapsed(boost::timer::cpu_timer const &timer, double peakBytes = 0, double deltaBytes = 0)
      : peakBytes(peakBytes), deltaBytes(deltaBytes)
  {
    measureElapsed(timer);
  }

  /**
     initialize with elapsed time from (already created/started) timer, and
     optionally measure process memory.
  */
  Elapsed(boost::timer::cpu_timer const &timer, MeasureProcessMemory measureProcessMemory, double peakBytes = 0, double deltaBytes = 0)
      : peakBytes(peakBytes), deltaBytes(deltaBytes)
  {
    measureElapsed(timer);
    measureMemory(measureProcessMemory);
  }

  double deltaMb() const {
    return deltaBytes/(1024.*1024.);
  }
  double peakMb() const {
    return peakBytes/(1024.*1024.);
  }

  std::string str() const;

  template <class Out>
  void print(Out &o) const {
    o << str();
  }

  template <class Ch, class Tr>
  friend std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr> &o, Elapsed const& self)
  { self.print(o); return o; }
};


/**
   This class automatically writes time spent and OS memory allocated
   between construction and destruction.
*/
struct Performance : boost::noncopyable
{
  Performance() { } // doesn't report anywhere
  Performance(std::string const& name, std::ostream &out) {
    init(StringOut(out, name+prefixSeparator()));
  }
  /** log to prefix - name. */
  Performance(std::string const& name, char const* const prefix)
  {
    init(logger(name, prefix));
  }
  /** log to prefix - name. */
  Performance(std::string const& name, std::string const& prefix="sdl.Performance")
  {
    init(logger(name, prefix));
  }
  /** StringConsumer is a function accepting a string argument. name is unused for now */
  Performance(std::string const& name, StringConsumer const& stringConsumer_)
  {
    init(stringConsumer_);
  }

  ~Performance() { finalReport(); }

  // restart timer for elapsed() purposes. does not reactivate reporting
  void restart();

  Elapsed now() const {
    return Elapsed(timer, kMeasureProcessMemory);
  }

  Elapsed elapsed() const {
    return now()-start;
  }

  /** log usage now, without preventing logging at destructor */
  void report()
  {
    if (stringConsumer.empty()) return;
    stringConsumer("(FINISHED) "+elapsed().str());
  }

  /** log usage now, and don't log again at destructor */
  void finalReport()
  {
    if (!latch(reported)) return;
    report();
  }

  bool disableReport() {
    return latch(reported);
  }

  void enableReport() {
    reported.clear();
  }

  static inline std::string prefixSeparator() { return "."; }

  static inline StringConsumer logger(std::string const& name,
                                      std::string const& prefix="sdl.Performance") {
    return logInfo(name, prefix + prefixSeparator());
  }


 private:
  void init(StringConsumer const& s);
  Elapsed start;
  Flag reported;
  StringConsumer stringConsumer;
  boost::timer::cpu_timer timer;
};


}}

#endif
