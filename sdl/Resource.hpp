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

#include <sdl/Util/IsDebugBuild.hpp>

namespace sdl {

#ifndef SDL_SHOW_RESOURCE_DESTRUCTOR_CERR
#define SDL_SHOW_RESOURCE_DESTRUCTOR_CERR 0
#endif

struct Resource : Evictable, Util::IAddLeakChecks {
  /// subclasses will have (via ResourceTraits.hpp or otherwise):
  // static char const* getTypeName();
  /// (not defined here to force compiler errors if missing)

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

  virtual char const* category() const override { return "resource"; }
  virtual void initProcessPhase(InitProcessPhase phase) override {
    if (phase == kPhase0) initProcess();
  }

 protected:
  /// no need to override if you override initProcessPhase
  virtual void initProcess() override {
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
  char const* type() override { return "AddLeakCheck"; }
  void operator()(Resource& adder) override { adder.addLeakChecks(ref); }
};

struct AcceptMaybeInitThread : AcceptResource {
  char const* type() override { return "MaybeInitThread"; }
  void operator()(Resource& resource) override { resource.maybeInitThread(); }
};

struct AcceptMaybeInitProcess : AcceptResource {
  InitProcessPhase phase;
  AcceptMaybeInitProcess(InitProcessPhase phase) : phase(phase) {}
  char const* type() override { return "MaybeInitProcess"; }
  void operator()(Resource& resource) override { resource.maybeInitProcess(phase); }
};

struct AcceptEvictThread : AcceptResource {
  bool any;
  Occupancy occupancy;
  AcceptEvictThread(Occupancy const& occupancy) : any(), occupancy(occupancy) {}
  char const* type() override { return "EvictThread"; }
  void operator()(Resource& resource) override {
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
  return static_pointer_cast<Resource>(pImpl);
}


}

#endif
