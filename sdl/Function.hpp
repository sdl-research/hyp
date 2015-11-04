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

     workaround for MSCV2015 sdl::function defect:

http://stackoverflow.com/questions/32078339/valid-code-fail-to-be-compiled-by-visual-studio-2015-std-function-bug

          VS 2015 does not (yet) support expression SFINAE, so you can't use
          std::is_constructible on sdl::function. (Note that this is strictly
          speaking a C++14 feature; C++11 sdl::function construction is not
          SFINAE-enabled.)

          From
          http://blogs.msdn.com/b/vcblog/archive/2015/04/29/c-11-14-17-features-in-vs-2015-rc.aspx:

          We're planning to start implementing Expression SFINAE in the compiler
          immediately after 2015 RTM, and we're planning to deliver it in an
          Update to 2015, supported for production use. (But not necessarily
          2015 Update 1. It might take longer.)
*/

#ifndef SDL__FUNCTION_GRAEHL_2015_11_02_HPP
#define SDL__FUNCTION_GRAEHL_2015_11_02_HPP
#pragma once

#if defined(_MSC_VER) && _MSC_VER <= 1900
#define SDL_FUNCTION_NS boost
#include <boost/function.hpp>
template <class F>
void setNullFunction(boost::function<F>& f) {
  f.clear();  // should also work: f = 0;
}
#else
#define SDL_FUNCTION_NS std
#include <functional>
template <class F>
void setNullFunction(std::function<F>& f) {
  f = nullptr;  // should also work: f = 0;
}
#endif

namespace sdl {

using SDL_FUNCTION_NS::function;


}

#endif
