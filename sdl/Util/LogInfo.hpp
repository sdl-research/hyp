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

    StringConsumer (fn accepting string) for log4cxx LOG_INFO.
*/

#ifndef LOGINFO_JG2012813_HPP
#define LOGINFO_JG2012813_HPP
#pragma once

#include <sdl/Log.hpp>
#include <sdl/StringConsumer.hpp>
#include <sdl/Util/LogLevel.hpp>
#ifndef NLOG
#include <log4cxx/logger.h>
#endif

namespace sdl { namespace Util {

struct LogInfo
{
  explicit LogInfo(std::string const& logname) : logname(logname) {
  }
  std::string logname; //TODO: save log4cxx object? for now, let logging cfg change and we reflect that always. this is used for infos only so perf. shouldn't matter. would use:
  //  LoggerPtr plog;
  void operator()(std::string const& msg) const
  {
#ifdef NLOG
    std::cerr << logname << ": " << msg << '\n';
#else
    LOG4CXX_INFO(log4cxx::Logger::getLogger(logname), msg);
#endif
  }
};

inline StringConsumer logInfo(std::string const& module, std::string const& prefix="sdl.")
{
  return LogInfo(prefix+module);
}


struct LogAtLevel
{
  explicit LogAtLevel(std::string const& logname_, LogLevel level = kLogInfo) {
    set(logname_, level);
  }
  LogAtLevel(std::string const& logname_, LogLevelPtr levelptr) {
    set(logname_, levelptr);
  }
  LogAtLevel(std::string const& logname_, std::string const& levelName, LogLevel fallbacklevel = kLogInfo) {
    set(logname_, levelName, fallbacklevel);
  }
  void setLevel(std::string const& name, LogLevel fallbacklevel = kLogInfo)
  {
    plevel = logLevel(name, fallbacklevel);
  }
  void setLevel(LogLevel level = kLogInfo)
  {
    plevel = logLevel(level);
  }
  void set(std::string logname_)
  {
    logname = logname_;
#ifndef NLOG
    plog = log4cxx::Logger::getLogger(logname);
#endif
  }
  void set(std::string const& logname_, std::string const& logLevelName, LogLevel fallbacklevel = kLogInfo) {
    set(logname_);
    setLevel(logLevelName, fallbacklevel);
  }
  void set(std::string const& logname_, LogLevel level) {
    set(logname_);
    setLevel(level);
  }
  void set(std::string const& logname_, LogLevelPtr levelptr) {
    set(logname_);
    plevel = levelptr;
  }
  std::string logname; //TODO: save log4cxx object? for now, let logging cfg change and we reflect that always. this is used for warnings only so perf. shouldn't matter. would use:
  LoggerPtr plog;
  LogLevelPtr plevel;
  void operator()(std::string const& msg) const
  {
#ifdef NLOG
    std::cerr << logname << ": " << msg << '\n';
#else
    LOG4CXX_LOG(plog, plevel, msg);
#endif
  }
};

inline StringConsumer logAtLevel(std::string const& module, std::string const& level, std::string const& prefix = SDL_LOG_PREFIX_STR, LogLevel fallbacklevel = kLogInfo)
{
  return LogAtLevel(prefix+module, level, fallbacklevel);
}

inline StringConsumer logAtLevel(std::string const& module, LogLevel level = kLogInfo, std::string const& prefix = SDL_LOG_PREFIX_STR)
{
  return LogAtLevel(prefix+module, level);
}

}}

#endif
