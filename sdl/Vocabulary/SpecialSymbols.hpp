/** \file

    objects for declaring special symbols

    TODO: make ID a compile-time constant.
*/

#ifndef SDL_VOCABULARY_SPECIALSYMBOLS_HPP
#define SDL_VOCABULARY_SPECIALSYMBOLS_HPP
#pragma once

#include <sdl/Vocabulary/SpecialSymbolsTemplate.hpp>
#include <sdl/Exception.hpp>
#include <sdl/Sym.hpp>

#define SPECIAL_SYMBOL(Name, UnquotedSymbol, Type)              \
  namespace sdl { \
  namespace Vocabulary {                                        \
  namespace Symbols {                                           \
  struct Symbol_##Name : SpecialSymbolTemplate<Symbol_##Name> { \
    static char const* str() { return #UnquotedSymbol; }  \
    static SymbolType type() { return Type; }                   \
  };                                                            \
  }                                                             \
  }                                                             \
  typedef sdl::Vocabulary::Symbols::Symbol_##Name Name; \
  }

// The following include will make each special symbol X globablly
// accessible as X::TOKEN and X::ID.
#include <sdl/Vocabulary/SpecialSymbolsList.ipp>

/**
   Specifies the order of symbol IDs. The constructor's being defined at all
   (and nobody instantiates a SpecialSymbolsOrder, according to grep) causes
   instantiations of the static variables, and further, in the fixed order given
   in SpecialSymbolsList.

   I'm not sure about whether this is standards-guaranteed or just gcc -
   JG. Wonder if an anon free function would do it also, but there's no reason to
   switch to that even if it does.

*/
struct SpecialSymbolsOrder {

  SpecialSymbolsOrder() {

// We re-define the macro SPECIAL_SYMBOL; this is important so that the
// special symbols get IDs in the same order everytime we execute the
// code. (Otherwise the order of symbols/IDs would be determined by which
// one happens to get *accessed* first.)
#undef SPECIAL_SYMBOL
#define SPECIAL_SYMBOL(n, s, t) \
  (void) sdl::n::ID; \
  (void) sdl::n::TOKEN;
// this seems to make a difference even in Release, so the compiler is
// processing the instantiations even though the statements are likely
// optimized away
#include <sdl/Vocabulary/SpecialSymbolsList.ipp>
  }
};

namespace sdl {

/// must be a multiple of 10
SymInt const SDL_NUM_BLOCKS = 1000;

namespace Vocabulary {

inline bool isFstComposeSpecial(Sym sym) {
  return sym < BLOCK_END::ID;
}

inline bool specialTerminalIsAnnotation(Sym specialTerminal) {
  assert(specialTerminal.type() == kSpecialTerminal);
  return !isFstComposeSpecial(specialTerminal);
}

inline bool isAnnotation(Sym sym) {
  return sym.type() == kSpecialTerminal && !isFstComposeSpecial(sym);
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

struct IsNotBlockSymbol {
  bool operator()(Sym sym) const { return !isBlockSymbol(sym); }
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

inline bool isBlockOp(SymIdInt blockOp) {
  assert(blockOp >= 0);
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
