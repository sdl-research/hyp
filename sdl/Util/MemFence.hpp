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

    memory fences (store, load, and store+load)

    see https://www.kernel.org/doc/Documentation/memory-barriers.txt

    useful for correctly implementing double-checked locking pattern.

    another common pattern:

    Processor A writes into a buffer.
    SDL_STORE_FENCE() or SDL_MEM_FENCE()
    Processor A writes "true" into a flag.

    Processor B waits until the flag is true.
    (SDL_LOAD_FENCE(), although no fence needed on inteIA-32, or intel64)
    Processor B reads the buffer.


    note that store, load fences are not really related to C++11 atomic release/acquire fences.

    From C++11 29.8 [atomics.fences]:

    A release fence A synchronizes with an acquire fence B if there exist atomic
    operations X and Y, both operating on some atomic object M, such that A is
    sequenced before X, X modifies M, Y is sequenced before B, and Y reads the
    value written by X or a value written by any side effect in the hypothetical
    release sequence X would head if it were a release operation.

    example:

    typedef AtomicCount M;    // from Util/RefCount.hpp

    M m = 0;

    release_fence(); // release fence A, before X
    ++m; // X writes to M

    // in another processor, perhaps:

    if (m > 0) happydance(); // Y reads from M, before B
    acquire_fence(); // acquire fence B

*/

#ifndef MEMFENCE_JG_2013_12_04_HPP
#define MEMFENCE_JG_2013_12_04_HPP
#pragma once

#ifdef _MSC_VER
#include <intrin.h>
// see also _mm_mfence

/**
   (*) General memory barriers.

   A general memory barrier gives a guarantee that all the LOAD and STORE
   operations specified before the barrier will appear to happen before all
   the LOAD and STORE operations specified after the barrier with respect to
   the other components of the system.

   A general memory barrier is a partial ordering over both loads and stores.

   General memory barriers imply both read and write memory barriers, and so
   can substitute for either.
*/

#define SDL_MEM_FENCE() _ReadWriteBarrier()

/**
   (*) Read (or load) memory barriers. implies

   A read barrier is a data dependency barrier plus a guarantee that all the
   LOAD operations specified before the barrier will appear to happen before
   all the LOAD operations specified after the barrier with respect to the
   other components of the system.

   A read barrier is a partial ordering on loads only; it is not required to
   have any effect on stores.

   [!] Note that read barriers should normally be paired with write barriers
*/

#define SDL_LOAD_FENCE() _ReadBarrier()

/**

   (*) Write (or store) memory barriers.

   A write memory barrier gives a guarantee that all the STORE operations
   specified before the barrier will appear to happen before all the STORE
   operations specified after the barrier with respect to the other
   components of the system.

   A write barrier is a partial ordering on stores only; it is not required
   to have any effect on loads.

   A CPU can be viewed as committing a sequence of store operations to the
   memory system as time progresses.  All stores before a write barrier will
   occur in the sequence _before_ all the stores after the write barrier.

   [!] Note that write barriers should normally be paired with read barriers
*/
#define SDL_STORE_FENCE() _WriteBarrier()

#elif (defined(__i386__) || defined(__x64__))
#define SDL_MEM_FENCE() asm volatile("mfence" ::: "memory")
#define SDL_LOAD_FENCE() asm volatile("lfence" ::: "memory")
#define SDL_STORE_FENCE() asm volatile("sfence" ::: "memory")
#else
#define SDL_MEM_FENCE() __sync_synchronize()
#define SDL_LOAD_FENCE SDL_MEM_FENCE
#define SDL_STORE_FENCE SDL_MEM_FENCE
#endif

#endif
