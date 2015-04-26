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

    log-level constants for log4cxx.
*/

#ifndef LOGLEVEL_JG_2013_11_21_HPP
#define LOGLEVEL_JG_2013_11_21_HPP
#pragma once

#ifndef NLOG
#include <log4cxx/level.h>
#include <log4cxx/logger.h>
#endif

namespace sdl {
namespace Util {

enum LogLevel {
  kLogAll = 100,
  kLogTrace = 10,
  kLogDebug = 5,
  kLogInfo = 1,
  kLogWarn = -1,
  kLogError = -5,
  kLogFatal = -10
};

#ifdef NLOG
typedef void* LogLevelPtr;
typedef void* LoggerPtr;

inline LogLevelPtr logLevel(int i) {
  return NULL;
}

inline LogLevelPtr logLevel(std::string const& l, int i) {
  return NULL;
}

inline std::string levelName(LogLevelPtr) {
  return "NOLOG";
}

#else
typedef log4cxx::LevelPtr LogLevelPtr;
typedef log4cxx::LoggerPtr LoggerPtr;

// FIXME: logLevel(0) -> pure virtual method called
inline LogLevelPtr logLevel(int i) {
#define LEVELATLEAST(x, name) \
  if (i >= (int)x) return log4cxx::Level::get##name()
  LEVELATLEAST(kLogAll, All);
  LEVELATLEAST(kLogTrace, Trace);
  LEVELATLEAST(kLogDebug, Debug);
  LEVELATLEAST(kLogInfo, Info);
  LEVELATLEAST(kLogWarn, Warn);
  LEVELATLEAST(kLogError, Error);
  LEVELATLEAST(kLogFatal, Fatal);
#undef LEVELATLEAST
  return log4cxx::Level::getOff();
}

// defaults
inline LogLevelPtr logLevel(std::string const& l, int i) {
  log4cxx::LevelPtr r = logLevel(i);
  return l.empty() ? r : log4cxx::Level::toLevel(l, r);
}

inline std::string levelName(LogLevelPtr l) {
  std::string r;
  l->toString(r);
  return r;
}

#endif


}}

#endif
