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

    log4cxx logger name prefixes.
*/

#ifndef SDL_LOG_JG_2014_04_07_HPP
#define SDL_LOG_JG_2014_04_07_HPP
#pragma once

#include <string>

/// preprocessor constant so we can do compile-time string constant concatentation in Util/LogHelper macros
#define SDL_LOG_PREFIX_STR "sdl."

namespace sdl {

namespace {
/// used for everything in xmt/... code
static std::string const kLogPrefix(SDL_LOG_PREFIX_STR);

/// used for process summary (peak rss, vmsize) and time elapsed
static std::string const kPerformanceLogPrefix(SDL_LOG_PREFIX_STR "Performance.");

/// used for input size (# words, time per word, etc.) logging
static std::string const kPerformancePerLogPrefix(SDL_LOG_PREFIX_STR "PerformancePer.");
}


}

#endif
