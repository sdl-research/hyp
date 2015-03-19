/** \file

    throw OSException exceptions for C API calls that use global errno. (usually
    the fn returns -1 - only then should you call Util::throw_errno()

    or use

    throw_if_errno(fd = open(...), "open");

    which will call throw_errno iff the first argument is -1

*/

#ifndef ERRNO_JG_2013_08_12_HPP
#define ERRNO_JG_2013_08_12_HPP
#pragma once

#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <Util/LogHelper.hpp>


namespace sdl {
namespace Util {

enum { kErrnoBufsize = 2000 };

template <class Exception, class String>
inline void throwErrno(String const& callname, char const* logname = "sdl.Util.errno") {
  char buf[kErrnoBufsize];
  using namespace std;
#ifdef WIN32
  char const* msg = buf;
  ::strerror_s(buf, kErrnoBufsize, errno);
#elif(__APPLE__ || _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && !_GNU_SOURCE
  char const* msg = buf;
  // XSI-compliant
  strerror_r(errno, buf, kErrnoBufsize);
#else
  // gnu
  char const* msg = strerror_r(errno, buf, kErrnoBufsize);
#endif
  if (!msg || !msg[0]) msg = "(couldn't get errno)";
  THROW_LOG_NAMESTR(logname, Exception, "errno " << errno << ": ``" << msg << "'' in " << callname);
}

template <class Exception, class String>
inline void throwIfErrno(int throwIfMinus1, String const& callname,
                         char const* logname = "sdl.Util.errno") {
  if (throwIfMinus1 == -1) throwErrno<Exception>(callname, logname);
}

inline void throw_errno(std::string const& callname) {
  throwErrno<OSException>(callname, "sdl.Util.errno");
}

inline void throw_if_errno(int throwIfMinus1, std::string const& callname) {
  if (throwIfMinus1 == -1) throw_errno(callname);
}


}}

#endif
