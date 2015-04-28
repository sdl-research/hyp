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

    Report total performance for each module (by name) and avg by input size (#words, states, etc).
*/

#ifndef PERFORMANCEPER_JG2012813_HPP
#define PERFORMANCEPER_JG2012813_HPP
#pragma once


#include <boost/detail/lightweight_mutex.hpp>
#include <sdl/Util/Performance.hpp>
#include <sdl/Hypergraph/InputSize.hpp>
#include <sdl/Util/Flag.hpp>

namespace sdl { namespace Util {

struct PerformancePerOptions
{
  //maybe todo: option to write to dedicated file vs just logging
  std::string name;
  bool showTotal, showIncrement, measureMemory;
  /** at your own risk: use configure() to init. */
  PerformancePerOptions() : showTotal(true), showIncrement(false), measureMemory(false) {}
  template <class Config>
  void configure(Config &c)
  {
    c.is("measure per-input-size performance");
    c("memory", &measureMemory)("(may cause slowdown) report memory allocated from OS").init(false);
    c("every-input", &showIncrement)("log every input's size and performance").init(false);
    c("total", &showTotal)("log aggregate (total, avg per input size) performance").init(true);
    c("name", &name)("name for performance counter (if empty, a reasonable default will apply).").verbose().init("");
  }
  /** Is anything enabled? */
  bool enabled() const
  {
    return showTotal || showIncrement;
  }
  void disable()
  {
    showIncrement = showTotal = false;
  }
  /** Should memory be reported for every input?

      Probably the most heavyweight scenario
  */
  bool everyMem() const
  {
    return showIncrement && measureMemory;
  }

  void provideDefaultName(std::string const& defaultName)
  {
    if (name.empty())
      name = defaultName;
  }

};

/** Log each input's size and time spent - automatically output on destructor. */
struct PerformancePer
{
 private:
  typedef boost::detail::lightweight_mutex Mutex;
  PerformancePerOptions opt;
  Mutex mutex;
  Elapsed totalElapsed;
  std::size_t nInputs;
  InputSize totalInputSize;
  StringConsumer stringConsumer;
  std::string const& inputSizeUnits() const {
    return totalInputSize.units;
  }
  Flag reported;
 public:
  static inline std::string prefixSeparator() { return "."; }
  MeasureProcessMemory measuringMemory() const { return opt.measureMemory ? kMeasureProcessMemory : kNoMeasureProcessMemory; }
  bool enabled() const { return opt.enabled(); }

  PerformancePer() : nInputs() {
  }

  /** Constructor - default name applies only if opt.name empty. */
  explicit PerformancePer(PerformancePerOptions const& opt_, std::string const& defaultName="Performance") : nInputs() {
    init(opt_, defaultName);
  }

  /* Doesn't reset stats. */
  void init(PerformancePerOptions const& opt, std::string const& defaultName);

  /** set name (overriding configured name if any). */
  void setName(std::string const& name)
  {
    opt.name = name;
    init(opt, name);
  }
  /** return name (which is the configured name else defaultName). */
  std::string const& getName() const
  {
    return opt.name;
  }

  /** copy ctor. implicit is fine but we have a mutex which can't be copied */
  PerformancePer(PerformancePer const& o)
      : opt(o.opt)
      , totalElapsed(o.totalElapsed), nInputs(o.nInputs), totalInputSize(o.totalInputSize)
      , stringConsumer(o.stringConsumer)
  {}

  /** log usage now, and don't log again at destructor */
  void finalReport();

  void report();

  ~PerformancePer() { report(); }

  /** (thread-safe) add to elapsed time for input of given size. */
  PerformancePer &recordElapsed(InputSize const& size, Elapsed const &delta);

  /** (slightly faster recordElapsed) doesn't check that input size units match. */
  PerformancePer &recordElapsedUnitless(double size, Elapsed const &delta);

  /** printable representation of total/avg performance. */
  std::string str() const;


  template <class Out>
  void print(Out &o) const {
    o << str();
  }
  template <class Ch, class Tr>
  friend std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr> &o, PerformancePer const& self)
  { self.print(o); return o; }

  /** \return avg size per input */
  InputSize sizePerInput() const
  {
    return nInputs ? totalInputSize/nInputs : InputSize();
  }

  /** \return avg time and memory increase, per totalInputSize().units */
  Elapsed elapsedPerSize() const
  {
    double sz = totalInputSize.amount;
    if (!sz) return Elapsed();
    return totalElapsed/sz;
  }

  /** Scoped guard for adding the incremental time/memory used in processing an
      input for a total PerformancePer.  Measures memory only if
      PerformancePerOptions::measureMemory */
  struct RecordElapsed
  {
    PerformancePer *ptotal;
    Elapsed start;
    boost::timer::cpu_timer timer;
    InputSize inputSize;
    bool enabled;

    void init(PerformancePer *total, InputSize const& inputSize_)
    {
      ptotal = total;
      if (ptotal && ptotal->enabled()) {
        start.measureMemory(ptotal->measuringMemory());
        inputSize = inputSize_;
        timer.start();
      }
    }

    void init(PerformancePer &total, InputSize const& inputSize_)
    {
      init(&total, inputSize_);
    }
    template <class InputVariant>
    void init(PerformancePer &total, InputVariant const& inputVariant)
    {
      init(&total, getVariantInputSize(inputVariant));
    }

    RecordElapsed() {}
    template <class InputVariant>
    RecordElapsed(PerformancePer &total, InputVariant const& inputVariant)
    {
      init(total, inputVariant);
    }
    RecordElapsed(PerformancePer &total, InputSize const& inputSize)
    {
      init(total, inputSize);
    }
    RecordElapsed(PerformancePer *total, InputSize const& inputSize)
    {
      init(total, inputSize);
    }
    Elapsed now() const {
      return Elapsed(timer, ptotal->measuringMemory());
    }
    Elapsed elapsed() const {
      return now()-start;
    }
    ~RecordElapsed()
    {
      if (ptotal && ptotal->enabled())
        ptotal->recordElapsed(inputSize, elapsed());
    }
  };
};

}}

#endif
