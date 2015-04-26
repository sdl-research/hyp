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
