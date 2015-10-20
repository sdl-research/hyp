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

   open blocks (on a path from start S to u) are a fn of state u.

   you might think there are paths S -> (a|b) -> u where a and b are
   alternatives the lattice that leave you open in block(a) or block(b) in state
   u, but because the each block has only a single state that closes it, this
   would imply unbalanced block tags, which aren't allowed.

   therefore all the sets of blocks in BlockWeight +
   single-source-shortest-paths should be singletons.

   if 1: for each state, what's set of blocks we're in. in fact,
   InputPreprocessor does this w/ blockweight, but it's a set of multiple
   options, when with the BlockSpans property it should be a single (possibly
   empty) sequence
*/

#ifndef HG_BLOCKS_JG_2014_06_03_HPP
#define HG_BLOCKS_JG_2014_06_03_HPP
#pragma once

#include <sdl/Exception.hpp>
#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Util/SmallVector.hpp>

namespace sdl { namespace Hypergraph {

VERBOSE_EXCEPTION_DECLARE(BlockException)

typedef Util::small_vector<BlockId, kInlineSyms, SymsIndex> BlockIds;
typedef Util::small_vector<Position, kInlineSyms, SymsIndex> WordIds;

/// BlockOp meaning BlockId or kCloseBlock
typedef BlockId BlockOp;
BlockOp const kCloseBlock = 0;

typedef BlockIds BlockOps;

struct BlockWeight;
struct SingleBlockWeight;
typedef SingleBlockWeight BlocksForState;


}}

#endif
