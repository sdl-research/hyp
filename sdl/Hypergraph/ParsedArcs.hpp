/** \file

    to hold hypergraph arcs to be converted later to a particular vocabulary.
*/

#ifndef PARSEDARCS_JG_2014_01_14_HPP
#define PARSEDARCS_JG_2014_01_14_HPP
#pragma once

#include <vector>

namespace sdl { namespace Hypergraph {

namespace ParserUtil {
struct Arc;
}

typedef ParserUtil::Arc ParsedArc;

/// will leak if you don't call parseText(ParsedArcs ...)
typedef std::vector<ParsedArc*> ParsedArcs;

}}

#endif
