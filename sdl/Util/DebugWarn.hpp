/** \file

    cerr-based facility for debugging problems in static init. or
    destruct. (where log4cxx is unsafe to use and not yet configured)
*/

#ifndef SDL_DEBUGWARN_HPP
#define SDL_DEBUGWARN_HPP
#pragma once

#ifdef NDEBUG
# define DWARN(x)
#else
# define DWARN(x) std::cerr << "\nWARNING: " << x<<"!\n\n"
# include <iostream>
#endif

#endif
