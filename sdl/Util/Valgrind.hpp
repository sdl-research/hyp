// Copyright 2014 SDL plc
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/** \file

     macros selecting valgrind safety (at the cost of performance) for the debug
     build
*/

#ifndef SDL_VALGRIND_JG2012126_HPP
#define SDL_VALGRIND_JG2012126_HPP
#pragma once

#ifndef SDL_VALGRIND
# ifdef NDEBUG
//release:
# define SDL_VALGRIND 0
# else
# define SDL_VALGRIND 1
# endif
#endif

// if you see a redefinition warning, you should include this header first.
#define GRAEHL_VALGRIND SDL_VALGRIND

#endif
