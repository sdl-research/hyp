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

    Implementation for Performance
*/

#include <sdl/Util/MemoryInfo.hpp>
#include <sdl/Util/Performance.hpp>
#include <boost/format.hpp>

namespace sdl {
namespace Util {

static const double megaPer1 = 1. / (1024 * 1024);
std::string Elapsed::str() const {
  if (peakBytes)
    return boost::str(boost::format("%1$.2fs (%2$.2fs real time), %3$+.2f MB => [%4$.2f MB]") % sec % wallSec
                      % (deltaBytes * megaPer1) % (peakBytes * megaPer1));
  else
    return boost::str(boost::format("%1$.2fs (%2$.2fs real time)") % sec % wallSec);
}

void Performance::init(StringConsumer const& s) {
  stringConsumer = s;
  restart();
}

void Performance::restart() {
  start.measureMemory();
  timer.start();
}

double processBytes() {
  return (double)sdl::Util::MemoryInfo::instance().size();
}

CpuTimer gTimer;  // The timer is started automatically when it gets initialized

CpuTimes cpuTimesNow() {
  return gTimer.elapsed();
}


}}
