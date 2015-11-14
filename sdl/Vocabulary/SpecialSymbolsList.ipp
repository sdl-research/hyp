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
   Hypergraph arc special symbols.

   user-unfriendly include instead of preprocessor macro because there's no
   other standard-compliant way to get a counter starting at 0 at compile time
   (__COUNTER__ is nonstandard and can't be reset to 0 anyway)

   Meanings: epsilon and phi don't advance the input pointer when
   taking the arc (rho and sigma do).

   epsilon and sigma match any symbol; phi and rho are "else" - they
   match any symbol not explicitly named on another outgoing arc.

   GOTCHA: you must keep the first 4 eps sigma phi rho - at the start and the
   block symbols at the end.
*/

/*
 * Hypergraph/Compose.hpp expects that eps, sigma, phi, rho are the
 * first lexical symbols (in that order). DO NOT CHANGE THIS.
 */

/// This appears first so <eps> has index 0 and thus Sym::id() 0, so
/// EPSILON::ID is 0, like in OpenFst.
/// The IDs for sigma, phi, and rho are different from the
/// corresponding IDs in OpenFst, where they are negative numbers.
#define BOOST_PP_VALUE 0
#include BOOST_PP_ASSIGN_SLOT(1)

#define SDL_SPECIAL_SYMBOL_NAME EPSILON
#define SDL_SPECIAL_SYMBOL_TEXT "eps"
#include SDL_SPECIAL_SYMBOL_INC
#define SDL_SPECIAL_SYMBOL_NAME SIGMA
#define SDL_SPECIAL_SYMBOL_TEXT "sigma"
#include SDL_SPECIAL_SYMBOL_INC
#define SDL_SPECIAL_SYMBOL_NAME PHI
#define SDL_SPECIAL_SYMBOL_TEXT "phi"
#include SDL_SPECIAL_SYMBOL_INC
#define SDL_SPECIAL_SYMBOL_NAME RHO
#define SDL_SPECIAL_SYMBOL_TEXT "rho"
#include SDL_SPECIAL_SYMBOL_INC


#define SDL_SEGMENT_NONSTANDARD_START_END 0

#define SDL_SPECIAL_SYMBOL_NAME UNK
#define SDL_SPECIAL_SYMBOL_TEXT "unk"
#include SDL_SPECIAL_SYMBOL_INC

#define SDL_SPECIAL_SYMBOL_NAME SENT_START
#define SDL_SPECIAL_SYMBOL_TEXT "s"
#include SDL_SPECIAL_SYMBOL_INC
#define SDL_SPECIAL_SYMBOL_NAME SENT_END
#define SDL_SPECIAL_SYMBOL_TEXT "/s"
#include SDL_SPECIAL_SYMBOL_INC

// Used by tokenizers to mark tokens.
// Example: <tok> 't' 'e' 's' 't' </tok>
// IMPORTANT: placing new symbols before TOK_START and TOK_END
// will break unit tests (hardwired values).
#define SDL_SPECIAL_SYMBOL_NAME TOK_START
#define SDL_SPECIAL_SYMBOL_TEXT "tok"
#include SDL_SPECIAL_SYMBOL_INC
#define SDL_SPECIAL_SYMBOL_NAME TOK_END
#define SDL_SPECIAL_SYMBOL_TEXT "/tok"
#include SDL_SPECIAL_SYMBOL_INC

// Used by RegexTokenizer to protect expressions from being split into
// tokens. Example: <tok-protect> 'e' '.' 'g' '.' </tok-protect>
#define SDL_SPECIAL_SYMBOL_NAME TOK_PROTECT_START
#define SDL_SPECIAL_SYMBOL_TEXT "tok-protect"
#include SDL_SPECIAL_SYMBOL_INC
#define SDL_SPECIAL_SYMBOL_NAME TOK_PROTECT_END
#define SDL_SPECIAL_SYMBOL_TEXT "/tok-protect"
#include SDL_SPECIAL_SYMBOL_INC


/// this is not the SyntaxBased "GLUE" nonterminal.
#define SDL_SPECIAL_SYMBOL_NAME GLUE
#define SDL_SPECIAL_SYMBOL_TEXT "glue"
#include SDL_SPECIAL_SYMBOL_INC

// Used by TrieGrammar and Syntax decoder to separate source formula and preconditions
#define SDL_SPECIAL_SYMBOL_NAME SDL_STAR
#define SDL_SPECIAL_SYMBOL_TEXT "lw-star"
#include SDL_SPECIAL_SYMBOL_INC

// Used by syntax decoder to denote the start of the foreign sentence
#define SDL_SPECIAL_SYMBOL_NAME FS
#define SDL_SPECIAL_SYMBOL_TEXT "foreign-sentence"
#include SDL_SPECIAL_SYMBOL_INC

// Used by abortSegment to return the output of the canceled segment
#define SDL_SPECIAL_SYMBOL_NAME ABORTSEGMENT
#define SDL_SPECIAL_SYMBOL_TEXT "abort-segment"
#include SDL_SPECIAL_SYMBOL_INC
#define SDL_SPECIAL_SYMBOL_NAME NULL_TAG
#define SDL_SPECIAL_SYMBOL_TEXT "null"
#include SDL_SPECIAL_SYMBOL_INC

#define SDL_SPECIAL_SYMBOL_NAME JUMP_WALL
#define SDL_SPECIAL_SYMBOL_TEXT "jump-wall"
#include SDL_SPECIAL_SYMBOL_INC

//////////////////////////////
// SDL_BLOCK symbols
//////////////////////////////

// These are used by the decoders: They do not move any words or
// phrases into the block or out of the block. BLOCK_START and CONSTRAINT_SUBSTITUTE get SDL_NUM_BLOCKS symbols <xmt-blockN> and <xmt-entityN> starting at N=0 (up to N = SDL_NUM_BLOCKS-1)

#define SDL_SPECIAL_SYMBOL_NAME BLOCK_END
#define SDL_SPECIAL_SYMBOL_TEXT "/xmt-block"
#include SDL_SPECIAL_SYMBOL_INC
/// see SpecialSymbols.hpp: sym < BLOCK_START::ID + SDL_NUM_BLOCKS
#define SDL_SPECIAL_SYMBOL_NAME BLOCK_START
#define SDL_SPECIAL_SYMBOL_TEXT "xmt-block"
#include SDL_SPECIAL_SYMBOL_INC

/// see SpecialSymbols.hpp: BLOCK_START::ID + SDL_NUM_BLOCKS <= sym < BLOCK_START::ID + 2 * SDL_NUM_BLOCKS
#define SDL_SPECIAL_SYMBOL_NAME CONSTRAINT_SUBSTITUTE
#define SDL_SPECIAL_SYMBOL_TEXT "xmt-entity"
#include SDL_SPECIAL_SYMBOL_INC

// CONSTRAINT_SUBSTITUTE and BLOCK_START are actually a range of symbols, each one which prints as the same string
// //TODO: preprocessor iteration or similar to give each one a numeric id suffix for clarity and round-trip hg io

// Do NOT define additional special symbols here at the
// end. Additional special symbols must be defined BEFORE the block
// symbols.
