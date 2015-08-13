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

    objects for declaring special symbols

    TODO: make ID a compile-time constant.

    SpecialSymbols have id and index starting at 0: epsilon, sigma, phi, rho
    (these are the Hypergraph fst composition special symbols)
*/

#ifndef SDL_VOCABULARY_SPECIALSYMBOLS_HPP
#define SDL_VOCABULARY_SPECIALSYMBOLS_HPP
#pragma once

#include <sdl/Vocabulary/SpecialSymbolVocab.hpp>
#include <sdl/Vocabulary/SpecialSymbolsTemplate.hpp>
#include <sdl/Exception.hpp>
#include <sdl/Sym.hpp>

#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/slot/slot.hpp>

#define SDL_SPECIAL_SYMBOL_INC "src/DefineSpecialSymbol.ipp"

// The following include will make each special symbol X globablly
// accessible as X::TOKEN and X::ID.
#include <sdl/Vocabulary/SpecialSymbolsList.ipp>

namespace sdl {

/// causes instantiation of SpecialSymbolsTemplate classes in order, so we can
/// fill the SpecialSymbolVoc. putting this in .cpp won't work for some reason.
struct SpecialSymbolsOrder {
  SpecialSymbolsOrder() {
#undef SDL_SPECIAL_SYMBOL_INC
#define SDL_SPECIAL_SYMBOL_INC "src/OrderSpecialSymbol.ipp"
#include <sdl/Vocabulary/SpecialSymbolsList.ipp>
  }
};

/// must be a multiple of 10
SymInt const SDL_NUM_BLOCKS = 10000;

namespace Vocabulary {

inline bool isFstComposeSpecial(Sym sym) {
  return sym <= RHO::ID;
}

struct WhichFstComposeSpecials {
  enum { knFstComposeSpecials = RHO::id + 1 };
  char ids_;
  WhichFstComposeSpecials() : ids_() {}
  bool defined() const { return ids_ < (1 << knFstComposeSpecials); }
  static WhichFstComposeSpecials undefined() {
    WhichFstComposeSpecials r;
    r.ids_ = 1 << knFstComposeSpecials;
    return r;
  }
  bool test(Sym sym) const { return test(sym.id()); }
  void set(Sym sym) { return set(sym.id()); }
  bool test(SymInt id) const {
    assert(id <= knFstComposeSpecials);
    return ids_ & (1 << id);
  }
  void set(SymInt id) {
    assert(id <= knFstComposeSpecials);
    ids_ |= (1 << id);
  }
  void check(Sym sym) {
    if (isFstComposeSpecial(sym)) set(sym);
  }
  friend inline std::ostream& operator<<(std::ostream& out, WhichFstComposeSpecials const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const {
    out << "fst-compose-specials:";
    for (SymInt id = 0; id < knFstComposeSpecials; ++id)
      if (test(id)) out << ' ' << specialSymbols().str(specialTerminal(id));
  }
};

inline bool specialTerminalIsAnnotation(Sym specialTerminal) {
  assert(specialTerminal.type() == kSpecialTerminal);
  return specialTerminal >= JUMP_WALL::ID;
}

inline bool isAnnotation(Sym sym) {
  return sym.type() == kSpecialTerminal && specialTerminalIsAnnotation(sym);
}

/**
   Returns true if terminal but not epsilon or phi
   etc.
 */
inline bool isConsuming(Sym sym) {
  return sym.isTerminal() && !(sym == EPSILON::ID || sym == PHI::ID);
}

/**
   Returns true if the sym is BLOCK_START or BLOCK2_START,
   etc.
 */
inline bool isBlockStartSymbol(Sym sym) {
  return sym >= BLOCK_START::ID && sym < BLOCK_START::ID + SDL_NUM_BLOCKS;
}

inline bool isBlockOpen(Sym sym) {
  return sym >= BLOCK_START::ID && sym < BLOCK_START::ID + SDL_NUM_BLOCKS;
}

inline Sym kSpecialEnd() {
  return BLOCK_START::ID + 2 * SDL_NUM_BLOCKS;
}

inline bool isConstraintSubstituteSym(Sym sym) {
  return sym >= BLOCK_START::ID + SDL_NUM_BLOCKS && sym < BLOCK_START::ID + 2 * SDL_NUM_BLOCKS;
}

inline BlockId blockIdForSubstituteSym(Sym sym) {
  assert(isConstraintSubstituteSym(sym));
  return sym.id() - (BLOCK_START::ID.id() + (SDL_NUM_BLOCKS - 1));
}

/// \return blockIdForSubstituteSym(sym) - 1 for a 0-based index instead of 1-based
inline BlockId indexForSubstituteSym(Sym sym) {
  assert(isConstraintSubstituteSym(sym));
  return sym.id() - (BLOCK_START::ID.id() + SDL_NUM_BLOCKS);
}

inline Sym substituteSymForIndex(BlockId id) {
  assert(id <= SDL_NUM_BLOCKS);
  return (BLOCK_START::ID + SDL_NUM_BLOCKS) + id;
}

inline Sym substituteSymForBlockId(BlockId id) {
  assert(id <= SDL_NUM_BLOCKS);
  assert(id > 0);
  return (BLOCK_START::ID + SDL_NUM_BLOCKS - 1) + id;
}

/**
   Returns true if sym is a block start or block end symbol.

   (recall that we have BLOCK_END, BLOCK_START, ...)
*/
inline bool isBlockSymbol(Sym sym) {
  return sym >= BLOCK_END::ID && sym < BLOCK_START::ID + SDL_NUM_BLOCKS;
}

inline bool isBlockSymbolOrJumpWall(Sym sym) {
  return sym >= JUMP_WALL::ID && sym < BLOCK_START::ID + SDL_NUM_BLOCKS;
}

inline bool isJumpWallOrBlockOrSubstituteSymbol(Sym sym) {
  return sym >= JUMP_WALL::ID && sym < BLOCK_START::ID + 2 * SDL_NUM_BLOCKS;
}

inline bool isBlockOrSubstituteSymbol(Sym sym) {
  return sym >= BLOCK_END::ID && sym < BLOCK_START::ID + 2 * SDL_NUM_BLOCKS;
}

inline bool isBlockOpenOrSubstituteSymbol(Sym sym) {
  return sym >= BLOCK_START::ID && sym < BLOCK_START::ID + 2 * SDL_NUM_BLOCKS;
}

inline bool isBlockClose(Sym sym) {
  return sym == BLOCK_END::ID;
}

struct IsBlockSymbol {
  bool operator()(Sym sym) const { return isBlockSymbol(sym); }
};

struct IsBlockStartSymbol {
  bool operator()(Sym sym) const { return isBlockStartSymbol(sym); }
};

/// probably we can just check IsConstraintSubstitute instead (since no other
/// special symbol should contribute to rule-source-sides)
struct IsNotBlockSymbolOrJumpWall {
  bool operator()(Sym sym) const { return !isBlockSymbolOrJumpWall(sym); }
};

struct IsLexical {
  bool operator()(Sym sym) const { return sym.isLexical(); }
};

struct IsConstraintSubstitute {
  bool operator()(Sym sym) const { return Vocabulary::isConstraintSubstituteSym(sym); }
};

inline BlockId blockIdForStart(Sym sym) {
  assert(isBlockStartSymbol(sym));
  return sym.id() - (BLOCK_START::ID.id() - 1);
}

inline BlockId blockIndexForStart(Sym sym) {
  assert(isBlockStartSymbol(sym));
  return sym.id() - BLOCK_START::ID.id();
}


/**
   Returns the ID of the block started (0 if
   none). BLOCK_START::ID starts block #1, BLOCK_START::ID+1 starts
   block #2, BLOCK_START::ID+2 starts block #3, etc.
 */
inline BlockId getBlockId(Sym sym) {
  return isBlockStartSymbol(sym) ? blockIdForStart(sym) : 0;
}

inline BlockId getBlockOp(Sym sym) {
  return sym.id() - BLOCK_END::ID.id();
}

inline bool isBlockOp(SymInt blockOp) {
  return blockOp <= SDL_NUM_BLOCKS;
}

inline void checkBlockId(BlockId id) {
  if (!(id <= SDL_NUM_BLOCKS && id > 0))
    SDL_THROW_LOG(Vocabulary.BLOCK_OPEN, ConfigException,
                  "<xmt-block"
                      << id << "> exceeded max # of blocks " << SDL_NUM_BLOCKS
                      << " - recompile with a larger limit or decode with fewer constraints per segment");
}

// TODO: because of the way SpecialSymbolsList works, ID is not a compile-time
// constant. perhaps we could use boost preprocessor to give us an enum (like
// SDL_ENUM)
inline Sym getBlockOpen(BlockId id) {
  assert(id <= SDL_NUM_BLOCKS);
  assert(id > 0);
  return (BLOCK_START::ID - 1) + id;
}

inline Sym blockOpenForIndex(BlockId index) {
  if (index >= SDL_NUM_BLOCKS)
    SDL_THROW_LOG(Constraints, ConfigException,
                  "Fixed limit SDL_NUM_BLOCKS="
                      << SDL_NUM_BLOCKS << " exceeded with block-open index" << index
                      << " - recompile with larger or prune your input, or use fewer constraints");

  return BLOCK_START::ID + index;
}

inline Sym getBlockOpenChecked(BlockId id) {
  checkBlockId(id);
  return (BLOCK_START::ID - 1) + id;
}


}}

#endif
