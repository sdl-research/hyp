// Copyright 2014-2015 SDL plc
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

    atomic count for intrusive_refcount base class. this gives you the same
    thread safety as shared_ptr but with memory and speed savings. the downside
    is that it requires some extra code compared to just `shared_ptr<MyImpl>`.

    usage:

    struct MyRefCounted : sdl::intrusive_refcount<MyRefcounted, RefCount> {
    //...
    };

    typedef Util::intrusive_ptr<MyRefcounted> MyPtr;
*/


#ifndef SDL_UTIL__REF_COUNT_HPP
#define SDL_UTIL__REF_COUNT_HPP
#pragma once


// it would only be appropriate to set this to 0 for a single-threaded program
#ifndef SDL_USE_ATOMIC_COUNT
# define SDL_USE_ATOMIC_COUNT 1
#endif

#if SDL_USE_ATOMIC_COUNT
#include <boost/detail/atomic_count.hpp>
/*win32 doesn't have
  __sync_add_and_fetch (though you could use InterlockedAdd etc on
  volatile int32, anyway there are gotchas with thread priority
  inversion)
*/
#endif

#include <sdl/SharedPtr.hpp>

#include <graehl/shared/intrusive_refcount.hpp>

namespace sdl {
namespace Util {

typedef boost::detail::atomic_count AtomicCount;

#if SDL_USE_ATOMIC_COUNT
typedef AtomicCount RefCount;
#else
typedef unsigned RefCount;
#endif

using graehl::intrusive_refcount;

using graehl::shared_ptr_maybe_intrusive;
using graehl::is_refcounted;
using graehl::intrusive_clone;
using graehl::intrusive_copy_on_write;
using graehl::intrusive_make_unique;
using graehl::intrusive_make_valid_unique;
using graehl::shared_from_intrusive;

}}

#endif
