/** \file

    avoid compiler warnings from log4cxx by including this.
*/

#ifndef SDL_LOG_JG20121125_HPP
#define SDL_LOG_JG20121125_HPP
#pragma once

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 )
#pragma warning ( disable: 4251 )
#endif

#ifndef ANDROID
#include <log4cxx/logger.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/helpers/transcoder.h>
#endif

#if defined(_MSC_VER)
#pragma warning (pop)
#endif

#endif
