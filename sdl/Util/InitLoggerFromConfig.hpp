/** \file

 .
*/

#ifndef INITLOGGERFROMCONFIG_JG_2014_11_15_HPP
#define INITLOGGERFROMCONFIG_JG_2014_11_15_HPP
#pragma once

#include <sdl/Util/InitLogger.hpp>
#include <sdl/Util/FindFile.hpp>
#include <log4cxx/xml/domconfigurator.h>

namespace sdl {
namespace Util {

inline void initLoggerFromConfig(std::string& logConfigFile, char const* progname = "xmt",
                                 LogLevel defaultLevel = kLogInfo) {
  if (logConfigFile.empty())
    initLogger(progname, logLevel(defaultLevel));
  else {
    logConfigFile = Util::findFile()(logConfigFile);
    log4cxx::xml::DOMConfigurator::configure(logConfigFile);
    Util::findFile().activateLogging();
  }
}


}}

#endif
