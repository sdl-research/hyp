// Copyright 2014 SDL plc
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

    C++11 override and final keyword support for some compilers pre-C++11:
    declare classes final (for better performance), and virtual methods
    'override' (very useful for catching typos preventing the method you wrote
    from getting called when you expect it

   C++11 override, final

   class D SDL_FINAL {
   };

   class B {
     virtual int f();
   };

   struct C : B {
     int f(int) OVERRIDE; //error
   }

   http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2010/n3206.htm


   SDL_FINAL will optimize virtual calls in YourClass to non-virtual:

   struct YourClass SDL_FINAL : IWhatever {

   };

   also OVERRIDE_FINAL applies to an overriden function (should allow it to be called from that class w/o
   virtual dispatch, at least on some compilers w/ C++11)

   struct C : B {
     int f() OVERRIDE_FINAL;
   }
*/

#ifndef OVERRIDE_LW201213_HPP
#define OVERRIDE_LW201213_HPP
#pragma once

#include <graehl/shared/noreturn.hpp>

#if defined(_MSC_VER)
#define OVERRIDE override
#define SDL_FINAL
#elif defined(__clang__)
#define OVERRIDE override
#define SDL_FINAL final
#else
#if __GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ >= 7
#if __cplusplus >= 201103L
#if !defined(HAVE_CPP11_FINAL)
#define HAVE_CPP11_FINAL 1
#endif
#if !defined(HAVE_CPP11_OVERRIDE)
#define HAVE_CPP11_OVERRIDE 1
#endif
#endif
#endif
#if HAVE_CPP11_OVERRIDE
#define OVERRIDE override
#else
#define OVERRIDE
#endif
#endif

#ifndef SDL_FINAL
#if HAVE_CPP11_FINAL
#define SDL_FINAL final
#else
#define SDL_FINAL
#endif
#endif

#define OVERRIDE_FINAL OVERRIDE SDL_FINAL

#endif
