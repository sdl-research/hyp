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
std::string const kLogPrefix(SDL_LOG_PREFIX_STR);

/// used for process summary (peak rss, vmsize) and time elapsed
std::string const kPerformanceLogPrefix(SDL_LOG_PREFIX_STR "Performance.");

/// used for input size (# words, time per word, etc.) logging
std::string const kPerformancePerLogPrefix(SDL_LOG_PREFIX_STR "PerformancePer.");
}

}

#endif
