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
