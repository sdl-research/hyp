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

namespace sdl { namespace Util {

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

#else
typedef log4cxx::LevelPtr LogLevelPtr;
typedef log4cxx::LoggerPtr LoggerPtr;

//FIXME: logLevel(0) -> pure virtual method called
inline LogLevelPtr logLevel(int i) {
#define LEVELATLEAST(x, name) if (i >= (int)x) return log4cxx::Level::get##name()
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
  log4cxx::LevelPtr r=logLevel(i);
  return l.empty() ? r : log4cxx::Level::toLevel(l, r);
}
#endif

}}

#endif
