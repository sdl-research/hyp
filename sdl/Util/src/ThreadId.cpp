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
#include <sdl/Util/ThreadId.hpp>
#include <sdl/Util/LogHelper.hpp>
#ifdef _WIN32
#include <windows.h>
#endif

namespace sdl {
namespace Util {

namespace impl {
bool gSingleThreadProgram;
bool gFixedSingleThreadProgram;
ThreadId gLastThreadId;
}


/// if program is declared as single thread, throw if reached by more than one thread
void maybeCheckSingleThread() {
  if (impl::gFixedSingleThreadProgram && isSingleThreadProgram()) checkSingleThread();
}


void checkSingleThread() {
  ThreadId const id = threadId();
  if (impl::gLastThreadId && id != impl::gLastThreadId)
    SDL_THROW_LOG(Util.ThreadId, ProgrammerMistakeException,
                  "more than one thread: called checkSingleThread from " << id << " and also "
                                                                         << impl::gLastThreadId);
}

void setFixedSingleThread() {
  impl::gFixedSingleThreadProgram = true;
}

void setSingleThreadProgram(bool singleThread) {
  using namespace impl;
  if (singleThread != gSingleThreadProgram) {
    if (gFixedSingleThreadProgram)
      SDL_THROW_LOG(Util.ThreadId, ProgrammerMistakeException,
                    "must call only once: setSingleThreadProgram("
                        << singleThread << ") - previous value was " << gSingleThreadProgram);
    gSingleThreadProgram = singleThread;
    setFixedSingleThread();
  }
}

#ifdef _WIN32

ThreadId threadId() {
  return GetCurrentThreadId();
}

#else

#include <pthread.h>
ThreadId threadId() {
  pthread_t id = pthread_self();
  if (sizeof(pthread_t) == sizeof(ThreadId)) {
    return reinterpret_cast<ThreadId const&>(id);
  } else
    return reinterpret_cast<unsigned const&>(id);
}

#endif


}}
