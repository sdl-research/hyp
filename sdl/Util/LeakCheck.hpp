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

    see ObjectCount for impls:

    #include <sdl/Util/ObjectCount.hpp>

*/

#ifndef LEAKCHECK_JG_2014_03_13_HPP
#define LEAKCHECK_JG_2014_03_13_HPP
#pragma once

#include <iostream>
#include <sdl/Util/ThreadId.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#if SDL_OBJECT_COUNT
#include <sdl/Util/ObjectCount.hpp>
#include <sdl/Util/GenSym.hpp>
#endif

#ifndef SDL_SUPPRESS_IGNORABLE_WARNINGS
# define SDL_SUPPRESS_IGNORABLE_WARNINGS 0
#endif

# ifndef SDL_OBJECT_COUNT_REPORT_NONZERO_INIT
# define SDL_OBJECT_COUNT_REPORT_NONZERO_INIT 1
# endif

#if SDL_OBJECT_COUNT
# define SDL_LEAK_CHECK_NAME_GLOBAL(type, name, nonce, global) sdl::Util::LeakCheck< type, global > const SDL_GENSYM(gLeakCheck ## nonce) (name);
# define SDL_LEAK_CHECK_NAME_GLOBAL_NS(type, name, nonce, global) namespace { SDL_LEAK_CHECK_NAME_GLOBAL(type, name, nonce, global) }
#else
# define SDL_LEAK_CHECK_NAME_GLOBAL(type, name, nonce, global)
# define SDL_LEAK_CHECK_NAME_GLOBAL_NS(type, name, nonce, global)
#endif

#define SDL_LEAK_CHECK_GLOBAL(type, global) SDL_LEAK_CHECK_NAME_GLOBAL(type, #type, Guard, global)

/// for global check - logger might not be initialized
#define SDL_LEAK_CHECK(type) SDL_LEAK_CHECK_GLOBAL(type, true)

/// for other checks - use log4cxx ERROR
#define SDL_LEAK_CHECK_LOG(type) SDL_LEAK_CHECK_GLOBAL(type, false)

namespace sdl { namespace Util {


struct ILeakCheck {
  virtual void init(char const* classname = "Object",
                    bool enableCheck = true,
                    bool nonGlobalSoYouCanLog = true) {}
  virtual void print(std::ostream &o) const {}
  virtual void start() {}
  virtual void finish() {}
  virtual std::size_t size() const { return 0; }
  virtual char const* type() const { return "Object"; }
  virtual ~ILeakCheck() {}
};

struct ILeakChecks {
  virtual bool enabled() const { return false; }
  /**
     pass new ILeakCheck which we will own and delete.
  */
  virtual void add(ILeakCheck *p) = 0;
  /**
     start from same thread and so that matching finish() should report same ILeakCheck::size.
  */
  virtual void start() = 0;
  /**
     start from same thread and so that matching start() should report same ILeakCheck::size.
  */
  virtual void finish() = 0;
  /**
     will finish() if that wasn't called manually
  */
  virtual ~ILeakChecks() {}

  virtual std::size_t size() const { return 0; }
};

struct IAddLeakChecks {
  virtual void addLeakChecks(ILeakChecks &leaks) {}
};

struct LeakCheckBase : ILeakCheck {
  char const* type() const { return name; }
  virtual void print(std::ostream &o) const { o << name; }
  friend inline std::ostream& operator<<(std::ostream &out, LeakCheckBase const& self) {
    self.print(out);
    return out;
  }
  std::size_t expect;
  char const* name;
  void init(char const* classname = "Object",
            bool enableCheck = true,
            bool nonGlobalSoYouCanLog = true)
  {
    enable = enableCheck;
    log = nonGlobalSoYouCanLog;
    name = classname;
    start();
  }
  LeakCheckBase(bool enable = true)
      : expect()
      , name("Object")
      , enable(enable)
      , log()
  {}
  bool enable;
  bool log;

  void restart() {
    enable = true;
    start();
  }

  void start() {
    if (enable) {
      expect = size();
      if (log)
        SDL_DEBUG(Leak.LeakCheck.start,
                  "initial count of "<<*this << ": " << expect);
    }
  }
  void finish() {
    if (enable) {
      std::size_t got = size();
      if (log)
        SDL_DEBUG(Leak.LeakCheck.finish,
                  "final count of "<<*this << ": " << got << " (initial=" << expect << ")");
      if (got > expect) {
        /// since we reset cache every N segs, the last one might clear more than it increased
        if (log) {
          // For Production builds, log this error without throwing an exception
          SDL_THROW_IF(!SDL_SUPPRESS_IGNORABLE_WARNINGS, Leak.ObjectCount, ProgrammerMistakeException,
                       "expected remaining count of " << *this << " to be its initial value " << expect <<
                       " but got " << got << " instead (make sure multiple threads aren't confusing your check boundaries)");
        }
        else if (got) {
          std::cerr << "\nWARNING: LeakCheck<"<<*this << ">: " << got << " live objects at exit instead of the expected "
                    <<expect << '\n';
        }
      }
    }
    enable = false;
  }
  ~LeakCheckBase() {
    if (enable)
      finish();
  }
};

struct LeakChecks : ILeakChecks {
  typedef boost::ptr_vector<ILeakCheck> Checks;
  Checks checks;
  bool enabled() const { return !checks.empty(); }
  void add(ILeakCheck *p) {
    checks.push_back(p);
  }
  void start() {
    for (Checks::iterator i = checks.begin(), e = checks.end(); i!=e; ++i)
      i->start();
  }
  void finish() {
    for (Checks::iterator i = checks.begin(), e = checks.end(); i!=e; ++i)
      i->finish();
    checks.clear();
  }
  std::size_t size() const { return checks.size(); }
  ~LeakChecks() {
    finish();
  }
};

struct NoLeakCheck : ILeakCheck {
  NoLeakCheck(char const* classname) {}
};

template <class Val, bool globalScope = false>
struct LeakCheck : LeakCheckBase {
#if SDL_OBJECT_COUNT
  std::size_t size() const {
    return ObjectCount<Val>::size();
  }
  LeakCheck(char const* classname)
      : LeakCheckBase(!globalScope && Util::isSingleThreadProgram())
  {
    name = classname;
    log = !globalScope;
    if (globalScope)
      enable = true;
    ObjectCount<Val>::setName(name);
    if (SDL_OBJECT_COUNT_REPORT_NONZERO_INIT && expect && globalScope)
      std::cerr << "\nLeakCheck<"<<*this << ">: at startup, " << expect
                <<" live objects at global scope - so there must be (indirectly) global objects of type '" << name << "'\n";
    start();
  }
#else
  LeakCheck(char const* classname)
      : LeakCheckBase(false)
  {
    name = classname;
  }
#endif
};


}}

#endif
