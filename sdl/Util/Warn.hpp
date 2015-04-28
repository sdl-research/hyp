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

    StringConsumer that logs with severity warning.
*/

#ifndef WARN_JG2012615_HPP
#define WARN_JG2012615_HPP
#pragma once

#include <sdl/StringConsumer.hpp>
#include <sdl/Util/LogHelper.hpp>
#ifndef NLOG
#include <log4cxx/logger.h>
#endif

namespace sdl { namespace Util {

struct LogWarning
{
  explicit LogWarning(std::string const& logname) : logname(logname) {}
  std::string logname; //TODO: save log4cxx object? for now, let logging cfg change and we reflect that always. this is used for warnings only so perf. shouldn't matter. would use:
  //  LoggerPtr plog;
  void operator()(std::string const& msg) const
  {
#ifdef NLOG
    std::cerr << logname << "(WARNING): " << msg << '\n';
#else
    LOG4CXX_WARN(log4cxx::Logger::getLogger(logname), msg);
#endif
  }
};

inline StringConsumer logWarning(std::string const& module, std::string const& prefix = SDL_LOG_PREFIX_STR)
{
  return LogWarning(prefix+module);
}

}}

#endif
