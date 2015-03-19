/** \file

    inherit from Resource to allow something to be runtime-yaml-configured via
    Resources.
*/

#ifndef RESOURCE_JG_2013_11_14_HPP
#define RESOURCE_JG_2013_11_14_HPP
#pragma once

#include <sdl/Evictable.hpp>
#include <sdl/SharedPtr.hpp>
#include <sdl/Util/LogHelper.hpp>
#if SDL_ASSERT_THREAD_SPECIFIC
#include <sdl/Util/AssertThreadSpecific.hpp>
#endif
#include <sdl/Util/LeakCheck.hpp>
#include <sdl/Util/Override.hpp>
#include <sdl/Util/IsDebugBuild.hpp>

namespace sdl {

#ifndef SDL_SHOW_RESOURCE_DESTRUCTOR_CERR
#define SDL_SHOW_RESOURCE_DESTRUCTOR_CERR 0
#endif

struct Resource : Evictable, Util::IAddLeakChecks {
#if SDL_ASSERT_THREAD_SPECIFIC
  /**
     true if you should assertThreadSpecific.
  */
  Util::Flag threadSpecific_;
#endif

  virtual ~Resource() {
    if (SDL_SHOW_RESOURCE_DESTRUCTOR_CERR && Util::isDebugBuild())
      std::cerr << "\n~Resource '" << name_ << "'\n";
#if SDL_ASSERT_THREAD_SPECIFIC
    if (threadSpecific_) Util::AssertThreadSpecific::clear(this);
#endif
  }

  void setThreadSpecific(bool val = true) {
#if SDL_ASSERT_THREAD_SPECIFIC
    threadSpecific_ = val;
#endif
  }

  void check() const {
#if SDL_ASSERT_THREAD_SPECIFIC
    SDL_TRACE(Resource.assertThreadSpecific, "checking if resource " << name()
                                                                     << " is exclusive to this thread");
    if (threadSpecific_) assertThreadSpecific(nameC(), this);
#endif
  }

  virtual char const* category() const OVERRIDE { return "resource"; }
  virtual void initProcessPhase(InitProcessPhase phase) OVERRIDE {
    if (phase == kPhase0) initProcess();
  }

 protected:
  /// no need to override if you override initProcessPhase
  virtual void initProcess() OVERRIDE {
    SDL_THROW_LOG(Resource, ProgrammerMistakeException, "no initProcess defined for resource "
                                                        << name()
                                                        << " - see docs/xmt-initialization-and-eviction.md");
  }
};

struct AcceptResource {
  virtual char const* type() { return "AcceptResource"; }
  virtual void operator()(Resource& resource) {}
  virtual ~AcceptResource() {}
};

struct AcceptAddLeakChecks : AcceptResource {
  Util::ILeakChecks& ref;
  AcceptAddLeakChecks(Util::ILeakChecks& ref) : ref(ref) {}
  char const* type() OVERRIDE { return "AddLeakCheck"; }
  void operator()(Resource& adder) OVERRIDE { adder.addLeakChecks(ref); }
};

struct AcceptMaybeInitThread : AcceptResource {
  char const* type() OVERRIDE { return "MaybeInitThread"; }
  void operator()(Resource& resource) OVERRIDE { resource.maybeInitThread(); }
};

struct AcceptMaybeInitProcess : AcceptResource {
  InitProcessPhase phase;
  AcceptMaybeInitProcess(InitProcessPhase phase) : phase(phase) {}
  char const* type() OVERRIDE { return "MaybeInitProcess"; }
  void operator()(Resource& resource) OVERRIDE { resource.maybeInitProcess(phase); }
};

struct AcceptEvictThread : AcceptResource {
  bool any;
  Occupancy occupancy;
  AcceptEvictThread(Occupancy const& occupancy) : any(), occupancy(occupancy) {}
  char const* type() OVERRIDE { return "EvictThread"; }
  void operator()(Resource& resource) OVERRIDE {
    if (resource.maybeEvictThread(occupancy)) any = true;
  }
};

typedef shared_ptr<Resource> AnyResourcePtr;

/**
   \return shared_ptr<Resource>, which can act as a generic object pointer similar
   to Resource*. When a shared_ptr<Resource> instance constructed as:

   shared_ptr<Resource> pv(new X);

   is destroyed, it will correctly dispose of the X object by executing ~X.
*/
template <class Impl>
AnyResourcePtr anyResourcePtr(Impl* newImpl) {
  return AnyResourcePtr(newImpl);
}

template <class Impl>
AnyResourcePtr anyResourcePtr(shared_ptr<Impl> const& pImpl) {
  return boost::static_pointer_cast<Resource>(pImpl);
}


}

#endif
