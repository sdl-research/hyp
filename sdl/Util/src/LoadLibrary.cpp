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
#include <sdl/Util/LoadLibrary.hpp>

namespace sdl {
namespace Util {

#ifdef _WIN32

LoadedLib loadLib(const char* name) {
  return ::LoadLibrary(name);
}
void unloadLib(LoadedLib aLib) {
  ::FreeLibrary(aLib);
}
void* loadProc(LoadedLib aLib, const char* name) {
  return ::GetProcAddress(aLib, name);
}

#else

LoadedLib loadLib(const char* name) {
  return ::dlopen(name, RTLD_LAZY);
}
void unloadLib(LoadedLib aLib) {
  ::dlclose(aLib);
}
void* loadProc(LoadedLib aLib, const char* name) {
  return ::dlsym(aLib, name);
}

#endif


}}
