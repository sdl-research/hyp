/** \file

    a vector you can keep adding to without invalidating earlier iterators.
*/

#ifndef STABLEVECTOR_JG_2014_03_04_HPP
#define STABLEVECTOR_JG_2014_03_04_HPP
#pragma once

#include <sdl/Util/Valgrind.hpp>
#include <graehl/shared/stable_vector.hpp>
#include <sdl/Sym.hpp>
#include <string>

namespace sdl { namespace Util {

using graehl::stable_vector;

// TODO: experiment to find best performing kLog2StableVectorChunkSize. set now
// for (2^9 * 8 = 4096) (sizeof(std::string) is 8, page is typically 4096)
enum { kLog2StableVectorChunkSize = 9 };

typedef stable_vector<std::string, SymInt, kLog2StableVectorChunkSize> StableStrings;


}}

#endif
