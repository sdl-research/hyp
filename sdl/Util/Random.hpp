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

    configuration for random seed..
*/

#ifndef SDL_UTIL_RANDOM_HPP_
#define SDL_UTIL_RANDOM_HPP_
#pragma once

#include <graehl/shared/random.hpp>
#include <sdl/Util/ThreadSpecific.hpp>
#include <sdl/Util/ThreadLocal.hpp>

namespace sdl {
namespace Util {

/**
   configurable random seed (converts to unsigned).
*/
typedef graehl::random_seed RandomSeed;

/**
   Function object for uniformly distributed random numbers on [0,1). don't make copies of it unless you want
   to clone the hidden random state. For passing to functions that use it, use Random01Fct
*/
struct Random01 {
  typedef graehl::random_generator Generator;
  typedef boost::uniform_01<double> Distribution;
  typedef boost::variate_generator<Generator, Distribution> Variate;

  Variate random01;
  double operator()() { return random01(); }

  Random01() : random01(Generator(graehl::default_random_seed()), Distribution()) {}


  Random01(unsigned seed) : random01(Generator(seed), Distribution()) {}

  void reset(unsigned seed) { random01.engine().seed(seed); }
/**
   additional helper methods that use random01() e.g. double random0n(double n)
*/
#include <graehl/shared/random.ipp>
};

struct Random01Fct {
  Random01* pRandom;
  Random01Fct(Random01& random) : pRandom(&random) {}
  double operator()() const { return (*pRandom)(); }
  operator Random01&() const { return *pRandom; }
};

typedef ThreadSpecific<Random01> ThreadSpecificRandom01;

/// if necessary, put in anon namespace (or static linkage)
extern THREADLOCAL Random01* gThreadRandom01;

/**
   threadRandom()() -> random on [0.0,1).
*/
inline Random01& threadRandom() {
  // TODO: test
  if (!gThreadRandom01) gThreadRandom01 = new Random01();
  return *gThreadRandom01;
}

struct ThreadRandom01Fct {
  double operator()() const { return threadRandom()(); }
  operator Random01&() const { return threadRandom(); }
};


/**
   if you're sure you don't need thread safety. this one may be explicitly seeded.
*/
extern Random01 gRandom01;

struct GlobalRandom01Fct {
  double operator()() const { return gRandom01(); }
  operator Random01&() const { return gRandom01; }
};


}}

#endif
