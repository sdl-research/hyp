// Copyright 2014 SDL plc
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

    an atomic and regular flag with latch (return true first time only). see
    also Flag::latch
*/

#ifndef ONCEFLAG_LW2012425_HPP
#define ONCEFLAG_LW2012425_HPP
#pragma once

#include <sdl/Util/MemFence.hpp>
#include <sdl/Util/Flag.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

namespace sdl {
namespace Util {

/**
   multiple calls to first() across threads are safe - only one returns
   true. other ways of modifying value require caller hold mutex AND call
   SDL_STORE_FENCE() after - just use first()

   no use of atomic exchange/add because we get a fast return when first() is
   false and only set it once.
*/
struct OnceFlagAtomic {
  bool done_;

  /**
     first() may return true for a short time after.
  */
  void finishNonAtomic() {
    done_ = true;
  }

  typedef boost::mutex Mutex;
  typedef boost::lock_guard<Mutex> Lock;

  Mutex mutex_;

  OnceFlagAtomic() : done_() {}
  OnceFlagAtomic(OnceFlagAtomic const& o) : done_(o.done_) {}
  void operator=(OnceFlagAtomic const& o) { done_ = o.done_; }
  /**
     \return true the first time only (works across multiple threads). pre:
     mutex_ is not held by this thread or any other thread for long
  */
  inline bool first()
  {
    if (done_) return false;
    SDL_LOAD_FENCE();
    Lock lock(mutex_);
    if (done_) return false;
    done_ = true;
    SDL_STORE_FENCE();
    return done_;
  }
  /** \return synonym for first() - for interface consistency with Latch.hpp */
  friend inline bool latch(OnceFlagAtomic &once)
  {
    return once.first();
  }

  /** \return if first() has already returned true (peeks without modifying state) */
  inline bool already() const
  {
    SDL_LOAD_FENCE();
    return done_;
  }
  operator bool () const {
    return already();
  }
};


}}

#endif
