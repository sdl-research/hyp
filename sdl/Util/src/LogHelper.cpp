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
#include <sdl/Util/InitLogger.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/RefCount.hpp>

namespace sdl {
namespace Util {

// DebugStaticInit DebugStaticInit::gDebugStaticInit;

#if SDL_LOG_SEQUENCE_NUMBER
static AtomicCount gAtomicLogSeqXmt(0);

std::size_t nextLogSeq() {
  std::size_t r = ++gAtomicLogSeqXmt;
  gLogSeqXmt = r;
  return r;
}
#endif

bool gFinishedLogging
#if __GNUC__
    __attribute__((used))
#endif
    = false;

void finishLogging() {
  gFinishedLogging = true;
}

WithInitLogging::WithInitLogging(char const* name, LogLevel level) {
  initLoggerConsole(name, level);
}


}}
