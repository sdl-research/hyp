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
#ifndef SDL_UTIL_LOAD_LIBRARY_HPP
#define SDL_UTIL_LOAD_LIBRARY_HPP
#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace sdl { namespace Util {

#ifdef _WIN32
typedef HMODULE LoadedLib;
#else
typedef void* LoadedLib;
#endif

LoadedLib loadLib(const char* name);

void unloadLib(LoadedLib aLib);

void* loadProc(LoadedLib aLib, const char* name);

}}

#endif
