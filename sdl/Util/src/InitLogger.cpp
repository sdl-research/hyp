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
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4231 4251)
#endif
#include <sdl/Util/Locale.hpp>
#ifndef NLOG
#include <log4cxx/logger.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/helpers/transcoder.h>
#endif
#include <sdl/Util/InitLogger.hpp>
#include <sdl/Util/FindFile.hpp>
#include <sdl/Util/LogHelper.hpp>  //LOG_INFO() sdl::logLevel(int) etc
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <iostream>

#include <boost/date_time/posix_time/posix_time.hpp>

#if SDL_LOG_SEQUENCE_NUMBER
#include <sdl/Util/RefCount.hpp>
#endif

std::size_t gLogSeqXmt = 0;

namespace sdl {
namespace Util {

std::string logFileName(std::string appname) {
  boost::posix_time::ptime currentTime = boost::posix_time::second_clock::local_time();
  appname.push_back('-');
  appname += boost::posix_time::to_iso_string(currentTime);
  appname += ".log";
  return appname;
}

// TODO: free memory for valgrind?
#ifndef NLOG
void initLogger(std::string const& appname, Util::InitLoggerOptions opts) {
  LogLevelPtr level = logLevel(opts.loglvl, opts.verbose);
  initLogger(appname, level, opts);
}

void initLogger(std::string const& appname, LogLevelPtr level, Util::InitLoggerOptions const& opts) {
  // defaultLocale();
  if (!opts.xmlConfigFile.empty()) {
    if (opts.verbose) std::cerr << "Logging xml configuration from: " << opts.xmlConfigFile << '\n';
    log4cxx::xml::DOMConfigurator::configure(opts.xmlConfigFile);
    return;
  }
  log4cxx::LoggerPtr g_pStatsDBLogger(log4cxx::Logger::getRootLogger());
  if (opts.removeAppenders) g_pStatsDBLogger->removeAllAppenders();
  log4cxx::LogString logStrLayout;
  std::string layout(opts.patternLayout);
  if (layout.empty()) layout = (opts.multiThread ? "%-4t %-5p %c{2} - %m%n" : "%-5p %c{2} - %m%n");
  log4cxx::helpers::Transcoder::decode(layout, logStrLayout);
  log4cxx::PatternLayout* pLayout = new log4cxx::PatternLayout(logStrLayout);
  if (!opts.file.empty()) {
    std::string const& logFile = opts.file == "AUTO" ? logFileName(appname) : opts.file;
    if (opts.verbose) std::cerr << "Log: " << logFile << '\n';
    log4cxx::LogString logFileName;
    log4cxx::helpers::Transcoder::decode(logFile, logFileName);
    log4cxx::FileAppender* pAppender = new log4cxx::FileAppender(pLayout, logFileName, !opts.overwriteFile);
    g_pStatsDBLogger->addAppender(pAppender);
  }
  if (opts.console) {
    g_pStatsDBLogger->addAppender(
        new log4cxx::ConsoleAppender(pLayout, log4cxx::ConsoleAppender::getSystemErr()));
  }
  g_pStatsDBLogger->setLevel(level);
  if (opts.verbose)
    SDL_TRACE(log, "Started logging: " << appname << " log-level=" << levelName(level)
                                       << " (no log config file)");
  findFile().activateLogging();
}

void initLoggerConsole(char const* name, LogLevel level) {
  LogLevelPtr plevel = logLevel(level);
  //    defaultLocale(); // not well enough tested
  initLogger(name, plevel, InitLoggerOptions().setConsole(true));
}

#else
void initLogger(std::string const& appname, LogLevelPtr level, Util::InitLoggerOptions const& opts) {
  // defaultLocale();
}

void initLogger(std::string const& appname, Util::InitLoggerOptions opts) {
  // defaultLocale();
}

void initLoggerConsole(char const* name, LogLevel level) {
}

#endif  // end ifndef NLOG


}}
