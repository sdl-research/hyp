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

    Implementation for Performance and PerformancePer.
*/

#include <sdl/Util/PerformancePer.hpp>
#include <sdl/Util/MemoryInfo.hpp>
#include <boost/format.hpp>

namespace sdl { namespace Util {

std::string Elapsed::str() const
{
  const double inMB = 1./(1024*1024);
  if (peakBytes)
    return boost::str(boost::format("%1$.2fs (%2$.2fs real time), %3$+.2f MB => [%4$.2f MB]")
                      % sec % wallSec % (deltaBytes*inMB) % (peakBytes*inMB));
  else
    return boost::str(boost::format("%1$.2fs (%2$.2fs real time)") % sec % wallSec);
}

// Performance
void Performance::init(StringConsumer const& s)
{
  stringConsumer = s;
  start.measureMemory();
  timer.start();
}

double processBytes() {
  return (double)sdl::Util::MemoryInfo::instance_.getSize();
}

// PerformancePer

void PerformancePer::init(PerformancePerOptions const& opt_, std::string const& name)
{
  opt = opt_;
  opt.provideDefaultName(name);
  stringConsumer = LogInfo(kPerformancePerLogPrefix + opt.name);
}

PerformancePer &PerformancePer::recordElapsedUnitless(double size, Elapsed const &delta)
{
  {
    Mutex::scoped_lock guard(mutex);
    totalElapsed += delta;
    nInputs++;
    totalInputSize.amount += size;
  }
  if (opt.showIncrement)
    stringConsumer(boost::str(boost::format("Input #%1% of size %2% took %3%")
                              % nInputs % quantity(size, totalInputSize.units) % delta));
  return *this;
}

PerformancePer &PerformancePer::recordElapsed(InputSize const& size, Elapsed const &delta)
{
  if (totalInputSize.units.empty())
    totalInputSize.units = size.units;
  // this assertion won't work for multi-thread case
  //  assert(totalInputSize.units==size.units);
  return recordElapsedUnitless(size.amount, delta);
}

std::string PerformancePer::str() const {
  return boost::str(
      boost::format("Total: processed %1% in %2% inputs of avg size %3% in %4%, taking %5% per %6%")
      % totalInputSize % nInputs % sizePerInput() % totalElapsed % elapsedPerSize() % inputSizeUnits());
}

void PerformancePer::finalReport()
{
  if (!latch(reported)) return;
  report();
}

void PerformancePer::report()
{
  if (opt.enabled() && nInputs)
    stringConsumer(str());
}

CpuTimer gTimer; // The timer is started automatically when it gets initialized

CpuTimes cpuTimesNow() {
  return gTimer.elapsed();
}

}}
