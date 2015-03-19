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
