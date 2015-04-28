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

    microsoft C++ isn't thread safe with

    int const& f() {
     static const int expensive((compute_int_expensive()));
     return expensive;
    }

    which is guaranteed to be initialized once without threads.

    C++11 requires thread safety. gcc provides it.

    therefore, we detect _MSC_VER and allow

    SDL_CONST_REF_STATIC_LOCAL(int) f() {
     SDL_RETURN_STATIC_LOCAL(int, compute_int_expensive());
    }

    or equivalently

    SDL_CONST_REF_STATIC_LOCAL(int, f(), compute_int_expensive())

    a consequence using this macro, for microsoft, we execute
    compute_int_expensive() *every time* f() is called. so
    compute_int_expensive() should be idempotent and not terribly slow

    note that the expression in the macro goes into a type(...) constructor, so
    extra parens may be needed to avoid a false parse on a function declaration
    (google "C++ most vexing parse")

    for other compilers, we have a relatively cheap init-once

    you can force the compute-once version if you know there are no threads:

    # define SDL_THREAD_SAFE_STATIC_LOCAL 1
*/

#ifndef SDL_STATICLOCALSINGLETON_JG20121030_HPP
#define SDL_STATICLOCALSINGLETON_JG20121030_HPP
#pragma once


#ifndef SDL_THREAD_SAFE_STATIC_LOCAL
// you can override autodetection or provide better performance in the
// single-thread case by setting -DSDL_RETURN_STATIC_LOCAL=1
#ifdef _MSC_VER
# define SDL_THREAD_SAFE_STATIC_LOCAL 0
#else
# define SDL_THREAD_SAFE_STATIC_LOCAL 1
#endif
#endif

#if SDL_THREAD_SAFE_STATIC_LOCAL
# define SDL_CONST_REF_STATIC_LOCAL(type) static inline type
# define SDL_RETURN_STATIC_LOCAL(type, expr) return expr
#else
# define SDL_CONST_REF_STATIC_LOCAL(type) static inline type const&
# define SDL_RETURN_STATIC_LOCAL(type, expr) static type const save_return_##type(expr); return save_return_##type
#endif

/**
   e.g. nameAndArgs zero() or open(int door)
*/

#define SDL_CACHE_STATIC_LOCAL(type, nameAndArgs, expr) \
  SDL_CONST_REF_STATIC_LOCAL(type) nameAndArgs { SDL_RETURN_STATIC_LOCAL(type, expr); }

#endif
