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

    Initializes the Log4CXX logger, using an xml config file or simplified
    command line options.

    Default config for the Logger; which is only the warnings / errors
    and fatal messages get logged and all logs are directed to a local
    file in the current working directory. The file name is always
    generated at runtime, using the application name and the current
    time-stamp.

    This is mostly used for unit tests and a few stand-alone command line
    utilities. `xmt` shell logging is configurable only via XML config file.
*/

#ifndef SDL_UTIL_INITLOGGER_HPP_
#define SDL_UTIL_INITLOGGER_HPP_
#pragma once

#include <sdl/LexicalCast.hpp>
#include <string>
#include <sdl/Util/LogLevel.hpp>
#include <sdl/Util/Log.hpp>
#include <sdl/Util/Locale.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace sdl {
namespace Util {

/**
   \return timestamped appname-xxxxxx.log.
*/
std::string logFileName(std::string const& appname);

struct InitLoggerOptions {
  int verbose;  // chattier about log init. if 1 or more, activates Info.
  // whether or not the application uses multiple threads
  // if set to true, then logger will print out thread id information
  bool multiThread;
  bool console, removeAppenders, overwriteFile;
  std::string loglvl;
  std::string file;
  std::string xmlConfigFile;
  std::string patternLayout;

  InitLoggerOptions()
      : verbose(true), console(true), removeAppenders(false), overwriteFile(true), multiThread(false) {}

  InitLoggerOptions& setConsole(bool b = true) {
    console = b;
    return *this;
  }
  InitLoggerOptions& setVerbose(int v = 1) {
    verbose = v;
    return *this;
  }
  InitLoggerOptions& setMultiThread(int v = true) {
    multiThread = v;
    return *this;
  }
  InitLoggerOptions& setRemoveAppenders(bool b = true) {
    removeAppenders = b;
    return *this;
  }
  InitLoggerOptions& setOverwriteFile(bool b = true) {
    overwriteFile = b;
    return *this;
  }
  InitLoggerOptions& setFile(std::string const& str) {
    file = str;
    return *this;
  }
  InitLoggerOptions& setXmlConfigFile(std::string const& str) {
    xmlConfigFile = str;
    return *this;
  }
  InitLoggerOptions& setPatternLayout(std::string const& str) {
    patternLayout = str;
    return *this;
  }

  InitLoggerOptions setConsoleUnlessFile() {
    console = file.empty();
    return *this;
  }

  template <class Config>
  void configure(Config& c) {
    c.is("Logging");
    c("log-path", &file)("log4cxx: if AUTO, unique filename in cwd is created. if '' (empty string), stderr");
    c("log-level", &loglvl)(
        "log4cxx: log level e.g. DEBUG. integer verbosity (-v) is used as a fallback - 10=trace,5=Debug, "
        "1=Info");
    c("log-config", &xmlConfigFile)("optional log4cxx xml config file");
    c("console", &console)("log to stderr").init(true).verbose();
    c("verbose", &verbose)("verbose initialization info").init(true).verbose();
    c("remove-appenders", &removeAppenders)("remove existing log4cxx appenders first").init(false).verbose();
    c("overwrite-file", &overwriteFile)("overwrite existing output file [log-path]").init(true).verbose();
    c("multithread-layout", &multiThread)("use multithreaded log layout (show thread id")
        .init(false)
        .verbose();
  }

  template <class O>
  void print(O& o) const {
    o << "InitLogger(";
    if (!xmlConfigFile.empty())
      o << "log-config=" << xmlConfigFile;
    else {
      o << "console=" << console << " verbose=" << verbose << " overwriteFile=" << overwriteFile;
    }
    o << " loglvl=" << loglvl << ")";
  }
  template <class C, class T>
  friend std::basic_ostream<C, T>& operator<<(std::basic_ostream<C, T>& o, InitLoggerOptions const& self) {
    self.print(o);
    return o;
  }
  template <class C, class T>
  friend std::basic_ostream<C, T>& operator<<(std::basic_ostream<C, T>& o, InitLoggerOptions const* selfp) {
    o << "InitLoggerOptions@0x" << (void*)selfp << ": ";
    if (selfp) selfp->print(o);
    return o;
  }
};

void initLogger(std::string const& appname, LogLevelPtr level,
                InitLoggerOptions const& opts = InitLoggerOptions());

void initLogger(std::string const& appname, InitLoggerOptions opts = InitLoggerOptions());

void initLoggerConsole(char const* name = "InitLogger(default)", LogLevel level = kLogDebug);

/**
   usage: (e.g. in unit test).

   sdl::Util::InitLogger initLog("TestWhatever");
*/
struct InitLogger {
  InitLogger(char const* name = "InitLogger(default)", LogLevel level = kLogDebug) {
    initLoggerConsole(name, level);
  }
};


}}

#endif
