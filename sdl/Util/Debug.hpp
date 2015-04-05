// Copyright 2014 SDL plc
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

   in prog.cc:

    #define WHATEVER(x) x
    DECLARE_DBG_CH(WHATEVER)
    int main() {
      DBGM(WHATEVER,10, "x=" << x);
    }

   in prog.sh:

    WHATEVER_DBG=10 ./prog

    TODO: we seem to favor log4cxx, so deprecate this (or keep it, since it's
    more efficient for things you want to vary at runtime)

 */

#ifndef SDL_UTIL_DEBUG_HPP
#define SDL_UTIL_DEBUG_HPP
#pragma once

/// printing of debug msgs to stderr based on env vars like HYPERGRAPH_DBG=10

//TODO: use log4cxx instead of cerr?

//TODO: optionally include severity numeric level in header.

/// controlling DBGM and UTIL_DBG_MSG:
/// set to 1 for debugging a release build, else leave unset for debug only (see below).
//#define SDL_DEBUG_ENABLE 1

#ifndef SDL_DEBUG_ENABLE
#ifdef NDEBUG
# define SDL_DEBUG_ENABLE 0
#else
# define SDL_DEBUG_ENABLE 1
#endif
#endif

#ifndef GRAEHL_IFDBG_ENABLE
#define GRAEHL_IFDBG_ENABLE SDL_DEBUG_ENABLE
#endif
#include <sdl/graehl/shared/ifdbg.hpp>
#include <sdl/graehl/shared/dbg_level.hpp>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cstdlib> // atoi
#include <iostream>


#define OSTR_DBG std::cerr

// you must DECLARE_DBG_CH(ch) first
# define IFDBGENV(ch, level, x) do {if (DBG_MAX_LEVEL>=level && ch##_DBG_LEVEL>=level) {x;}}while(0)

#ifndef DBG_MAX_LEVEL
# define DBG_MAX_LEVEL 999
// decrease this to lose even overhead of if-enabled check; sets max effective level of getenv_int("{Channelname}_DBG");
#endif

#ifndef DBG_HEADER_SHOW_LVL
// show (LVL) after ChannelName
# define DBG_HEADER_SHOW_LVL 1
#endif

// TODO: should be controllable by environment var, too
#ifndef DBG_WITH_LINEINFO
// show :FILE:LINE after that
# define DBG_WITH_LINEINFO 1
#endif

#ifndef DBG_START_NEWLINE
// start new message with newline just in case
# define DBG_START_NEWLINE 1
#endif


#define THROW_EXCEPTION(ch, x) do {                                 \
  std::stringstream ss123;                                         \
  ss123 << "Exception ("#ch")" << __FILE__ << ":" << __LINE__ << ":" << x << '\n';   \
  throw std::runtime_error(ss123.str());                           \
  } while(0)


#include <graehl/shared/warning_compiler.h>
CLANG_DIAG_OFF(unneeded-internal-declaration)
CLANG_DIAG_OFF(unused-function)
#if DBG_WITH_LINEINFO
namespace {
inline std::string shortDebugFilename(std::string const& name) {
  std::string::size_type from=name.find_last_of('/');
  return from==std::string::npos ? name : std::string(name, from);
}
}
# define DBG_LINE_INFO <<':'<< shortDebugFilename(__FILE__) << ":" << __LINE__
#else
# define DBG_LINE_INFO
#endif
CLANG_DIAG_ON(unused-function)
CLANG_DIAG_ON(unneeded-internal-declaration)


//static local var means env var is checked once (like singleton)
#define DECLARE_DBG_CH(ch) DECLARE_DBG_LEVEL(ch)

#if DBGHEADER_SHOW_LVL
# define DBGVERBOSITY(n) <<"(" << n<<")"
#else
# define DBGVERBOSITY(n)
#endif

#if DBG_START_NEWLINE
# define DBG_INIT_NL "\n"
#else
# define DBG_INIT_NL
#endif

#define DBGHEADER(ch, n) DBG_INIT_NL #ch DBGVERBOSITY(n) DBG_LINE_INFO <<": "

#if SDL_DEBUG_ENABLE
#  define DBGX(ch, n, x) IFDBGENV(ch, n, x)
#  define DBGM(ch, n, x) IFDBGENV(ch, n, OSTR_DBG << DBGHEADER(ch, n) << x)
#  define DBGMC(ch, n, x) IFDBGENV(ch, n, OSTR_DBG << x)
#else
#  define DBGX(ch, n, x)
#  define DBGM(ch, n, x)
#  define DBGMC(ch, n, x)
#endif


// for example (needs to be in parent namespace of use):

//NOTE: identity macro of same name as debug level is required. I don't know how to automate declaring such macros.
//avoids expense of evaluating conditional dbg msgs in release:
#if SDL_DEBUG_ENABLE
# define UTIL(x) x
DECLARE_DBG_LEVEL(UTIL)
#else
# define UTIL(x)
#endif

#define UTIL_DBG_MSG(n, x) DBGM(UTIL, n, x)
//Continue on same line (no new header). you can also use for self-contained info msgs you don't want to jump to by source line in emacs
#define CUTIL_DBG_MSG(n, x) DBGMC(UTIL, n, x)
#define UTIL_DBG_EXEC(n, x) DBGX(UTIL, n, x)
#define THROW_UTIL_EXCEPTION(x) THROW_EXCEPTION(Util, x)

#endif
