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
