// Copyright 2014-2015 SDL plc
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/** \file

    macros SDL_* for logging and logged exceptions using log4cxx in the
    "sdl." prefix
*/

#ifndef SDL_UTIL_LOGHELPER_HPP
#define SDL_UTIL_LOGHELPER_HPP
#pragma once

#include <sdl/Exception.hpp>

#define SDL_EMPTY_STATEMENT() \
  do {                        \
  } while (0)

#ifdef NDEBUG
#define SDL_DEBUG_CERR(a, b)
#else
// for debugging static init problems temporarily
// (don't leave any SDL_DEBUG_CERR in production permanently, even for debug build)
#define SDL_DEBUG_CERR(a, b)                         \
  do {                                               \
    std::cerr << "\nsdl." << #a << " " << b << '\n'; \
  } while (0)
#endif

#ifndef SDL_LOG_SEQUENCE_NUMBER
#ifdef NDEBUG
#define SDL_LOG_SEQUENCE_NUMBER 0
#else
#define SDL_LOG_SEQUENCE_NUMBER 1
#endif
#endif

#include <sdl/Log.hpp>
#include <cstddef>
#include <sdl/Util/LogLevel.hpp>

#if SDL_LOG_SEQUENCE_NUMBER
/// for setting debugger breakpoints
extern std::size_t gLogSeqXmt;
#endif

// TODO: instead of directly including log4cxx headers, include this file so we
// can avoid globally disabling this useful warning:
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4231)
#endif

namespace sdl {
namespace Util {

#if SDL_LOG_SEQUENCE_NUMBER
std::size_t nextLogSeq();
#else
inline std::size_t nextLogSeq() {
  return 0;
}
#endif

struct LogSeq {
  template <class Ch, class Tr>
  friend std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& out, LogSeq const& self) {
    std::size_t const s = Util::nextLogSeq();
    if (s) out << "#~" << s << ": ";
    return out;
  }
};

// TODO: macros that increment seq number whether or not log is displayed (so you can tweak logging level and
// break on same seq) ... and named Seq logger w/ member log seq field so you can have one for each type of
// thing you might want to break on?

extern bool gFinishedLogging;

void finishLogging();

struct FinishLoggingAfterScope {
  ~FinishLoggingAfterScope() { finishLogging(); }
};

/// InitLogger (ctor) + FinishLoggingAfterScope (dtor)
struct WithInitLogging : FinishLoggingAfterScope {
  WithInitLogging(char const* name = "InitLogger(default)", LogLevel level = kLogDebug);
};
}
}


namespace sdl {
static Util::LogSeq const gseq = {};  // usage: SDL_TRACE(blah, gseq << blah) and then set hardware watchpoint
// on gLogSeqXmt == the # you see printed
}


#ifndef NLOG

#define LOG_ENABLED(x) x
#include <log4cxx/logger.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <sstream>

#define RELEASE_LOG_DEBUG_NAMESTR(loggerName, expression)                                               \
  do {                                                                                                  \
    if (!sdl::Util::gFinishedLogging) LOG4CXX_DEBUG(log4cxx::Logger::getLogger(loggerName), expression) \
  } while (0)
#define RELEASE_LOG_TRACE_NAMESTR(loggerName, expression)                                               \
  do {                                                                                                  \
    if (!sdl::Util::gFinishedLogging) LOG4CXX_TRACE(log4cxx::Logger::getLogger(loggerName), expression) \
  } while (0)

// Enable trace & debug statements if in Debug mode, or if explicitly allowed
#if !defined(NDEBUG) || defined(SDL_ENABLE_DEBUG_LOGGING) || defined(SDL_ENABLE_TRACE_LOGGING)

// the LOG4CXX_ macros unfortunately only enclose in {} and not do {} while (0), so we must:

#define LOG_DEBUG_NAMESTR(loggerName, expression)                                                       \
  do {                                                                                                  \
    if (!sdl::Util::gFinishedLogging) LOG4CXX_DEBUG(log4cxx::Logger::getLogger(loggerName), expression) \
  } while (0)

#if !defined(NDEBUG) || defined(SDL_ENABLE_TRACE_LOGGING)
#define LOG_TRACE_NAMESTR(loggerName, expression)                                                       \
  do {                                                                                                  \
    if (!sdl::Util::gFinishedLogging) LOG4CXX_TRACE(log4cxx::Logger::getLogger(loggerName), expression) \
  } while (0)
#else
#define LOG_TRACE_NAMESTR(loggerName, expression) SDL_EMPTY_STATEMENT()
#endif

#else  // NDEBUG: Release mode or if no override specified: Disable trace & debug statements

#define LOG_TRACE_NAMESTR(loggerName, expression) SDL_EMPTY_STATEMENT()
#define LOG_DEBUG_NAMESTR(loggerName, expression) SDL_EMPTY_STATEMENT()

#endif  // NDEBUG

#define LOG_LEVEL_NAMESTR(loggerNameSuffix, logLevel, expression)               \
  do {                                                                          \
    if (!sdl::Util::gFinishedLogging)                                           \
      LOG4CXX_LOG(log4cxx::Logger::getLogger(loggerName), logLevel, expression) \
  } while (0)

#define LOG_INFO_NAMESTR(loggerName, expression)                                                       \
  do {                                                                                                 \
    if (!sdl::Util::gFinishedLogging) LOG4CXX_INFO(log4cxx::Logger::getLogger(loggerName), expression) \
  } while (0)

#ifdef SDL_SUPPRESS_SOURCE_LOCATION  // Used with release mode for external releases: Hide source locations in
// log messages

#define LOG_WARN_NAMESTR(loggerName, expression)                                                       \
  do {                                                                                                 \
    if (!sdl::Util::gFinishedLogging) LOG4CXX_WARN(log4cxx::Logger::getLogger(loggerName), expression) \
  } while (0)

#define LOG_ERROR_NAMESTR(loggerName, expression)                                                       \
  do {                                                                                                  \
    if (!sdl::Util::gFinishedLogging) LOG4CXX_ERROR(log4cxx::Logger::getLogger(loggerName), expression) \
  } while (0)

#define LOG_FATAL_NAMESTR(loggerName, expression)                                          \
  do {                                                                                     \
    if (sdl::Util::gFinishedLogging)                                                       \
      throw std::runtime_error("fatal error - can't log cause because logging has ended"); \
    else {                                                                                 \
      LOG4CXX_FATAL(log4cxx::Logger::getLogger(loggerName), expression);                   \
    }                                                                                      \
  } while (0)

#else  // Don't SDL_SUPPRESS_SOURCE_LOCATION // Debug or Release mode for internal company use

#define LOG_WARN_NAMESTR(loggerName, expression)                                                               \
  do {                                                                                                         \
    if (!sdl::Util::gFinishedLogging)                                                                          \
      LOG4CXX_WARN(log4cxx::Logger::getLogger(loggerName), __FILE__ << ": " << __LINE__ << ": " << expression) \
  } while (0)

#define LOG_ERROR_NAMESTR(loggerName, expression)                                                               \
  do {                                                                                                          \
    if (!sdl::Util::gFinishedLogging)                                                                           \
      LOG4CXX_ERROR(log4cxx::Logger::getLogger(loggerName), __FILE__ << ": " << __LINE__ << ": " << expression) \
  } while (0)

#define LOG_FATAL_NAMESTR(loggerName, expression)                                                \
  do {                                                                                           \
    if (sdl::Util::gFinishedLogging)                                                             \
      throw std::runtime_error("fatal error - can't log cause because logging has ended");       \
    else {                                                                                       \
      LOG4CXX_FATAL(log4cxx::Logger::getLogger(loggerName), __FILE__ << ": " << __LINE__ << ": " \
                                                                     << expression);             \
    }                                                                                            \
  } while (0)

#endif  // SDL_SUPPRESS_SOURCE_LOCATION

/// these macros take unquoted constant strings for loggerName
#define LOG_LEVEL(loggerName, logLevel, expression) LOG_DEBUG_NAMESTR(#loggerName, logLevel, expression)
#define LOG_DEBUG(loggerName, expression) LOG_DEBUG_NAMESTR(#loggerName, expression)
#define LOG_INFO(loggerName, expression) LOG_INFO_NAMESTR(#loggerName, expression)
#define LOG_WARN(loggerName, expression) LOG_WARN_NAMESTR(#loggerName, expression)
#define LOG_TRACE(loggerName, expression) LOG_TRACE_NAMESTR(#loggerName, expression)
#define LOG_ERROR(loggerName, expression) LOG_ERROR_NAMESTR(#loggerName, expression)
#define LOG_EXCEPTION_NAMESTR(loggerName, expression, exception) \
  LOG_ERROR_NAMESTR(loggerName, expression << ": " << exception.what())
#define LOG_FATAL(loggerName, expression) LOG_FATAL_NAMESTR(#loggerName, expression)
// e.g. disabled: #define ifmacro(x)
// e.g. enabled: #define ifmacro(x) x

#else  // NLOG is defined:

#include <string>
#include <sdl/Exception.hpp>  // some standard exception types for SDL_THROW_LOG etc

#define LOG_ENABLED(x)
#define LOG_DEBUG(loggerName, expression) SDL_EMPTY_STATEMENT()
#define LOG_INFO(loggerName, expression) SDL_EMPTY_STATEMENT()
#define LOG_WARN(loggerName, expression) SDL_EMPTY_STATEMENT()
#define LOG_TRACE(loggerName, expression) SDL_EMPTY_STATEMENT()
#define LOG_ERROR(loggerName, expression) SDL_EMPTY_STATEMENT()
#define LOG_EXCEPTION_NAMESTR(loggerName, expression, exception) (void) exception
#define LOG_FATAL(loggerName, expression) SDL_EMPTY_STATEMENT()
#define RELEASE_LOG_TRACE_NAMESTR(loggerName, expression) SDL_EMPTY_STATEMENT()
#define RELEASE_LOG_DEBUG_NAMESTR(loggerName, expression) SDL_EMPTY_STATEMENT()
#define LOG_TRACE_NAMESTR(loggerName, expression) SDL_EMPTY_STATEMENT()
#define LOG_DEBUG_NAMESTR(loggerName, expression) SDL_EMPTY_STATEMENT()
#define LOG_INFO_NAMESTR(loggerName, expression) SDL_EMPTY_STATEMENT()
#define LOG_WARN_NAMESTR(loggerName, expression) SDL_EMPTY_STATEMENT()
#define LOG_ERROR_NAMESTR(loggerName, expression) SDL_EMPTY_STATEMENT()
#define LOG_FATAL_NAMESTR(loggerName, expression) SDL_EMPTY_STATEMENT()

#endif  // end ifndef NLOG

// same functionality as SDL_THROW (see Exception.hpp), but uses
// streams instead of %1% formatting
#define SDL_THROW_NOLOG(className, expression) \
  do {                                         \
    std::stringstream UNLIKELYss;              \
    UNLIKELYss << expression;                  \
    throw className(UNLIKELYss.str());         \
  } while (0)

#define THROW_LOG(loggerName, className, expression) THROW_LOG_NAMESTR(#loggerName, className, expression)

#define THROW_LOG_NAMESTR(loggerName, className, expression)                  \
  do {                                                                        \
    std::stringstream UNLIKELYss;                                             \
    UNLIKELYss << expression;                                                 \
    LOG_ERROR_NAMESTR(loggerName, UNLIKELYss.str() << " (Exception thrown)"); \
    throw className(UNLIKELYss.str());                                        \
  } while (0)

#define LOG_EXCEPTION(loggerName, expression, exception) \
  LOG_EXCEPTION_NAMESTR(#loggerName, expression, exception)

// Can be used to feed an expression containing commas to macros. or you could just use (x)
#define SDL_COMMA_PROTECT(x) x

#define SDL_LOG_PREFIX_NAMESTR(loggerNameSuffix) (SDL_LOG_PREFIX_STR loggerNameSuffix)
#define SDL_LOG_PREFIX(loggerNameSuffix) SDL_LOG_PREFIX_NAMESTR(#loggerNameSuffix)

/// cond => throw, else SDL_ERROR and continue
#define SDL_THROW_IF(cond, loggerNameSuffix, exceptionClass, expression) \
  do {                                                                   \
    if (cond) {                                                          \
      SDL_THROW_LOG(loggerNameSuffix, exceptionClass, expression);       \
    } else {                                                             \
      SDL_ERROR(loggerNameSuffix, expression);                           \
    }                                                                    \
  } while (0)

#define SDL_THROW_LOG(loggerNameSuffix, exceptionClass, expression) \
  THROW_LOG_NAMESTR(SDL_LOG_PREFIX(loggerNameSuffix), exceptionClass, expression)

#define LOG_RETHROW(loggerName, expression, exception) LOG_RETHROW_NAMESTR(#loggerName, expression, exception)
#define SDL_LOG_RETHROW(loggerNameSuffix, expression, exception) \
  LOG_RETHROW_NAMESTR(SDL_LOG_PREFIX(loggerNameSuffix), expression, exception)

/**
   SDL_LOG() is not disabled by release buildeven at debug/trace levels.
*/
#define SDL_LOG(loggerNameSuffix, logLevel, expression) \
  LOG_LEVEL_NAMESTR(SDL_LOG_PREFIX_NAMESTR(loggerNameSuffix), logLevel, expression)
/**
   SDL_TRACE and SDL_DEBUG are disabled in release build.
*/
#define SDL_TRACE(loggerNameSuffix, expression) \
  LOG_TRACE_NAMESTR(SDL_LOG_PREFIX(loggerNameSuffix), expression)
#define SDL_DEBUG(loggerNameSuffix, expression) \
  LOG_DEBUG_NAMESTR(SDL_LOG_PREFIX(loggerNameSuffix), expression)
#define SDL_DEBUG_ALWAYS(loggerNameSuffix, expression) \
  RELEASE_LOG_DEBUG_NAMESTR(SDL_LOG_PREFIX(loggerNameSuffix), expression)
#define RELEASE_SDL_TRACE(loggerNameSuffix, expression) \
  RELEASE_LOG_TRACE_NAMESTR(SDL_LOG_PREFIX(loggerNameSuffix), expression)
/**
   SDL_INFO and more severe are always enabled.
*/
#define SDL_INFO(loggerNameSuffix, expression) LOG_INFO_NAMESTR(SDL_LOG_PREFIX(loggerNameSuffix), expression)
#define SDL_WARN(loggerNameSuffix, expression) LOG_WARN_NAMESTR(SDL_LOG_PREFIX(loggerNameSuffix), expression)
#define SDL_ERROR(loggerNameSuffix, expression) \
  LOG_ERROR_NAMESTR(SDL_LOG_PREFIX(loggerNameSuffix), expression)
#define SDL_FATAL(loggerNameSuffix, expression) \
  LOG_FATAL_NAMESTR(SDL_LOG_PREFIX(loggerNameSuffix), expression)

#if ENABLE_TEST_SDL_DEBUG
#define TEST_SDL_DEBUG(what, msg) SDL_DEBUG_ALWAYS(what, msg)
#else
#define TEST_SDL_DEBUG(what, msg) SDL_DEBUG(what, msg)
#endif

#define SDL_ASSERT_THROW_LOG(loggerNameSuffix, cond, exceptionClass, expression) \
  do {                                                                           \
    if (!(cond)) {                                                               \
      SDL_THROW_LOG(loggerNameSuffix, exceptionClass, expression);               \
    }                                                                            \
  } while (0)

#define SDL_ASSERT_LOG(loggerNameSuffix, cond, expression) \
  SDL_ASSERT_THROW_LOG(loggerNameSuffix, cond, ProgrammerMistakeException, expression)

#define LOG_RETHROW_NAMESTR(loggerName, expression, exception)             \
  do {                                                                     \
    LOG_ERROR_NAMESTR(loggerName, expression << ": " << exception.what()); \
    (void) exception;                                                      \
    throw;                                                                 \
  } while (0)


#ifndef NDEBUG
#define SDL_DEBUG_IF(cond, name, msg) \
  do {                                \
    if (cond) SDL_DEBUG(name, msg);   \
  } while (0)
#else
#define SDL_DEBUG_IF(cond, name, msg)
#endif

#endif
