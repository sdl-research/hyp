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
