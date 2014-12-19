/** \file

    resources and other things that might have some evictable contents when
    we're under memory pressure (Occupancy is near 1)
*/

#ifndef SDL_EVICTABLE__JG_2013_11_14_HPP
#define SDL_EVICTABLE__JG_2013_11_14_HPP
#pragma once

#include <cassert>
#include <vector>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <sdl/Util/Override.hpp>
#include <sdl/Util/ThreadSpecific.hpp>
#include <sdl/Util/Flag.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/MemFence.hpp>
#include <sdl/Config/Named.hpp>

namespace sdl {

struct Occupancy {
  float fraction_;
  Occupancy(float fraction = 0.f) : fraction_(fraction) { assert(fraction >= 0 && fraction <= 1); }
  float fraction() const { return fraction_; }
  operator float() const { return fraction_; }
};

enum InitProcessPhase { kPhase0, kPhase1, knInitProcessPhase };

struct Evictable : Config::INamed {
 protected:
  typedef boost::recursive_mutex Mutex;
  typedef boost::lock_guard<Mutex> Lock;

  /// set to true in subclass if you want maybeInit to init every time. there's
  /// no easy way I know of without a per-process mutex to modify threadlocal
  /// 'done' state in another thread, so this hack is in place for now (for
  /// ResourceManager)
  Util::Flag alwaysInitThread_;

  typedef std::vector<Evictable*> Uses;  // TODO: switch to explicit depends-on edges

 public:
  bool alwaysInitThread() const { return alwaysInitThread_; }

  virtual ~Evictable() {}

  /**
     \return whether eviction happened - may not have init yet, or else
     implementation may not actually evict anything, in which case we leave
     threadInitDone_ true if init were called.
  */
  bool callEvictThread(bool always = true, Occupancy const& occupancy = Occupancy()) {
    if (always || threadInitDone_.get()) {
      SDL_DEBUG(evict.Evictable, category() << " " << name() << " evictThread(" << occupancy << ")");
      if (evictThread(occupancy) || always) {  // don't swap order (short circuit or)
        threadInitDone_.set(false);
        return true;
      }
    }
    return false;
  }

  bool maybeEvictThread(Occupancy const& occupancy = Occupancy()) {
    return callEvictThread(alwaysInitThread_, occupancy);
  }

  /**
     \return whether initialization had to be performed (false means it was
     already initialized w/o an evict since).
  */
  bool maybeInitThread() {
    if (alwaysInitThread_ || !threadInitDone_.get()) {
      SDL_DEBUG(evict.init.Evictable, category() << " " << name() << " initThread()");
      threadInitDone_.set(true);
      initThread();
      return true;
    } else
      return false;
  }

  /**
     \return whether init had to be performed (post: initialized).

     \param maybeEvict - call evictThread first if true
  */
  bool maybeReinitThread(Occupancy const& occupancy, bool maybeEvict = true) {
    if (maybeEvict) maybeEvictThread(occupancy);
    return maybeInitThread();
  }

  bool threadInitDone() const { return threadInitDone_.get(); }

  /// *Process methods must be called while no thread is doing work (you handle
  /// *this synchronization yourself)

  /**
     \return whether initialization had to be performed (false means it was
     already initialized w/o an evict since).

     safe to call from multiple threads whenever - provided you never evict.
  */
  bool maybeInitProcess(InitProcessPhase phase = kPhase0) {
    if (!processInitDone_[phase].get()) {
      SDL_LOAD_FENCE(); // correct double checked locking - see
      // http://www.aristeia.com/Papers/DDJ_Jul_Aug_2004_revised.pdf
      Lock lock(initProcessMutex_);
      if (!processInitDone_[phase].get()) {
        SDL_DEBUG(evict.init.Evictable, category() << " " << name() << " initProcess(phase=" << phase << ")");
        initProcessPhase(phase);
        processInitDone_[phase].set(true);
        SDL_STORE_FENCE();
        return true;
      }
      return false;
    }
    return false;
  }

  bool maybeInitProcessAndThread() {
    bool r = false;
    if (maybeInitProcess(kPhase0)) r = true;
    if (maybeInitProcess(kPhase1)) r = true;
    if (maybeInitProcess()) r = true;
    return r;
  }

  /// should be called when you know it hasn't been yet (e.g. constructing your own thing)
  void callInitProcess(InitProcessPhase phase = kPhase0) {
    SDL_DEBUG(evict.init.Evictable, category() << " " << name() << " initProcess(phase=" << phase << ")");
    initProcessPhase(phase);
    processInitDone_[phase].set(true);
    SDL_STORE_FENCE();
  }

  bool processInitDone(InitProcessPhase phase) const { return processInitDone_[phase].get(); }

  bool processInitDone() const {
    return processInitDone_[0].get() && processInitDone_[knInitProcessPhase - 1].get();
  }

  Evictable() {}

  /// call maybe* instead to prevent redundant re-init and evict-with-no-init
  virtual void initThread() {}

  /**
     initProcessPhase(kPhase0) must be called for all things before initProcessPhase(kPhase1).

     //TODO: explicit dependency dag would be safer
     */
  virtual void initProcessPhase(InitProcessPhase phase) {
    if (phase == kPhase0) initProcess();
  }
  /**
     \return whether anything needs to be initThread before next use.
  */
  virtual bool evictThread(Occupancy const&) { return false; }

  /// for Config/Dynamic clone()
  Evictable(Evictable const& o) : Config::INamed(o) { assignDone(o); }

  /// for Config/Dynamic clone()
  void operator=(Evictable const& o) {
    Config::INamed::operator=(o);
    assignDone(o);
  }

 protected:
  /// optional: if you don't override initProcess(phase)
  void assignDone(Evictable const& o) {
    for (unsigned i = 0; i < (unsigned)knInitProcessPhase; ++i) processInitDone_[i] = o.processInitDone_[i];
  }

  virtual void initProcess() {}

  Util::Flag processInitDone_[knInitProcessPhase];
  boost::recursive_mutex initProcessMutex_;

  /**
     should be called while holding initProcessMutex_

     e.g. when you define/create a new resource in ResourceManager - process
     specific actions have to happen again. since it would require a mutex in
     every thread's request processing to dirty the thread state as well, we'll
     avoid creating new resources after modules' init methods - see
     docs/xmt-initialization-and-eviction.md
  */
  void dirtyProcess() {
    for (unsigned i = 0; i < (unsigned)knInitProcessPhase; ++i) processInitDone_[i] = false;
  }

 private:
  Util::ThreadSpecificBool threadInitDone_;
};


}

#endif
