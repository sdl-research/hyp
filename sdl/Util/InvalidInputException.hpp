/** \file

    throw an InvalidInputException exception showing error position in input file (using seek if possible).

*/

#ifndef INPUTEXCEPTION_JG2013220_HPP
#define INPUTEXCEPTION_JG2013220_HPP
#pragma once

#include <sdl/Exception.hpp>

namespace sdl { namespace Util {

void throwInvalidInputException(std::istream &in, std::string const& error="", std::size_t itemNumber = 0, const char *item="line");

}}

#endif
