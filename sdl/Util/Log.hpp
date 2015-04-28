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
