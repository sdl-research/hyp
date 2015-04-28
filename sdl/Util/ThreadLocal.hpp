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

     for thread-safe globals (that don't need default ctor or dtor):

     THREADLOCAL int verbosity=1; // every thread gets this default value initially

     //thread safe dynamic scoping! (like perl "local")

     void f() {
       sdl::Util::SaveLocal<int> save(verbosity);
       verbosity=10;
       {
         sdl::Util::SetLocal<int> set(verbosity,11);
         //now verbosity is 11!
       }
       //now verbosity is back to 10
     } // on return, verbosity is back to its old value (probably 1)

     typedef ThreadLocalSingleton<Random01> ThreadRandom01;

     ThreadRandom01::get().random01();
*/

#ifndef THREADLOCAL_JG20121010_HPP
#define THREADLOCAL_JG20121010_HPP
#pragma once

#include <graehl/shared/threadlocal.hpp>

namespace sdl { namespace Util {

using graehl::SaveLocal;
using graehl::SetLocal;
using graehl::ThreadLocalSingleton;

}}

#endif
