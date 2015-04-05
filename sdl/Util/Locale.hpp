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

    helpers for system locale.
*/

#ifndef LWUTIL_LOCALE__HPP
#define LWUTIL_LOCALE__HPP
#pragma once

#if defined(_MSC_VER) || defined(ANDROID)
//TODO@SK: boost is failing to find icu at link time. investigate whether icuin.lib or icudt.lib should be used, or possibly the debug versions instead of release
// e.g. missing icu_48::Calendar::createInstance(...)
# define SDL_BOOST_LOCALE 0
#else
# define SDL_BOOST_LOCALE 1
#endif

#include <locale>
#include <locale.h>
#include <graehl/shared/stream_util.hpp>
#if SDL_BOOST_LOCALE
#include <boost/locale/util.hpp>
#endif

namespace sdl {
namespace Util {

/** apply system locale configuration (e.g. LC_ALL) on startup. if this is not called logging output will have '?' for every non-ascii char

    We should decide whether we want xmt to be affected by the OS notion of what locale is active (so that perhaps the locale env vars will be set appropriately depending on LP).
*/
inline void defaultLocale()
{
  /* using boost::locale because:

     "" should use system default locale

     but apple crashes with that argument to std::locale if LC_ALL is not one of
     `locale -a`, e.g. en_US.UTF-8. (most notably: an unset LC_ALL causes the crash)

     and if we want to set a specific locale as default, windows has different names than POSIX, e.g. "en-US"

     boost::locale promises even to enable utf8 somewhat in windows, which has no official support for that

  */
#if SDL_BOOST_LOCALE
  std::string sysLocale = boost::locale::util::get_system_locale(true);
  boost::locale::generator localeGen;
  std::locale::global(localeGen(sysLocale));
  ::setlocale(LC_ALL, sysLocale.c_str());
#else
  std::locale::global(std::locale("C"));
  ::setlocale(LC_ALL, "C");
#endif
}

/**
   Usage (start of main or outside it):

   Util::DefaultLocaleFastCout initCout;

   defaultLocale() above so you can see log messages

   and performance improvements for both cin and cout

   note: on Windows you shouldn't use stdin/stdout for high performance, since
   they translate newlines (and it's not possible in standard C++ to reopen cin
   and cout)
*/
struct DefaultLocaleFastCout
    : graehl::use_fast_cout
{
  DefaultLocaleFastCout(std::size_t bufSize = 256*1024, bool noBufferForTerminal = true)
      : graehl::use_fast_cout(bufSize, noBufferForTerminal)
  {
    defaultLocale();
  }
};


}}

#endif
