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

   symbol type and index in a 32-bit POD

   TODO: kPersistent* are now meaningless and should not be used. we should
   repurpose the id space (but have some saved ids in databases and would have
   to port)
 */

#ifndef SDL_SYMBOLIDENTIFIER_H_
#define SDL_SYMBOLIDENTIFIER_H_
#pragma once

#include <vector>
#include <cstddef>
#include <cassert>
#include <boost/range/detail/safe_bool.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/functional/hash.hpp>
#include <boost/serialization/is_bitwise_serializable.hpp>
#include <boost/serialization/level.hpp>
#include <sdl/Util/Unordered.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Exception.hpp>

namespace sdl {

VERBOSE_EXCEPTION_DECLARE(SymOutOfRangeException)
VERBOSE_EXCEPTION_DECLARE(InvalidSym)
VERBOSE_EXCEPTION_DECLARE(InvalidSymType)

typedef unsigned SymInt;
typedef SymInt BlockId;

struct Sym;

// TODO@JG: replace 'std::string[Ref] with just 'std::string [const&]' - will never change

/* SymbolType specifies how the 32 bit Sym space is divided:
  bits 29-32 ([0...6] << 29):   SymbolType:
 ------------                -----------------------
   000 kSpecialTerminal
   001 (unused - no need for separate special nts)
   010 kVariable
   011 kPersistentNonterminal
   100 kNonterminal
   101 kPersistentTerminal
   11 kTerminal

the last is a larger (2x) space of symbols - see kLargeSizeMask vs
kSmallSizeMask

 */

enum {
  kMaxSmallSymIndex = (1u << 29) - 1,
  kMaxLargeSymIndex = (1u << 30) - 2,  // kNoSymbol takes up an index in the large space
  kNoSymbol = 0xFFFFFFFFu
};  // setting this to UINT_MAX might give an 8-byte enum for some compilers

// implementation note: be careful to cast these to (Sym) since enums may be 8 byte signed
enum SymbolType {
  kBeginSpecial = 0,
  kSpecialTerminal = 0,
  kEndSpecialTerminal = 1u << 29,
  kEndSpecial = 2u << 29,

  kVariable = 2u << 29,
  kEndVariable = 3u << 29,

  kBeginNonterminal = 3u << 29,

  kPersistentNonterminal = 3u << 29,
  kNonterminal = 4u << 29,

  kEndNonterminal = 5u << 29,

  kBeginTerminal = 5u << 29,
  // TODO: make kTerminal start here (only hangup: vocab/grammar db backward
  // compat). for now kPersistentTerminal and kPersistentNonterminal ranges are
  // unusable in practice

  // meaning persistent or not - not including special. might want to make all
  // the terminal types contiguous (then kNoSymbol wouldn't be end)
  kPersistentTerminal = 5u << 29,
  kTerminal = 6u << 29,
  kLargeIdsStart = 6u << 29,  // and 7 also - part of large range

  kEndTerminal = (SymInt)kNoSymbol,
  // of course you have to cast all these to SymInt anyway in case compiler
  // decided that SymbolType is signed.

  kLargeIncrementable = 0xcFFFFFFFu,
  // looking at just the 29 lsb suffices to find unincrementable (max index)
  // items, as long as you also check this, which *is* incrementable

  kBeginMaybePersistent = kBeginNonterminal,
  // above this, difference in kPersistentBit distinguishes persistent/not
  kPersistentBit = 1u << 29,
  // TODO@SK: CM-275 changing persistent/nonpersistent changes whether that bit is 0 or 1

  kNoSymbolType = kNoSymbol,
  kAllSymbols = kNoSymbol - 1,
};

inline std::ostream& operator<<(std::ostream& o, SymbolType t) {
  return o << "SymbolType=" << ((unsigned)t >> 29);
}

/**
   (string, type) -> (index, type) in the context of a vocabulary. the whole thing
   is 32 bits and a POD (no constructors, so is amenable to more
   optimizations). use static methods to create an instance. POD means feel free
   to memcpy to copy or serialize
*/
struct Sym {

  enum {
    kSmallTypeMask = (SymInt)(7u << 29),  // 6 types that use 29 bits for offset and 3 for type id
    kSmallSizeMask = (SymInt)0x1FFFFFFFu,  // msbits 0001
    kLargeTypeMask = (SymInt)(6u << 29),  // the large space uses 30 bits for offset and 2 for type id
    kLargeSizeMask = (SymInt)0x3FFFFFFFu,  // msbits 0011
    // k*TypeMask | k*SizeMask == -1
    // k*TypeMask & k*SizeMask == 0
  };

  static inline char const* getTypeName(SymbolType type) {
    switch (type) {
      case kSpecialTerminal:
        return "Special Terminal";
      case kVariable:
        return "Variable";
      case kPersistentNonterminal:
        return "Persistent Non-terminal";
      case kNonterminal:
        return "Non-terminal";
      case kPersistentTerminal:
        return "Persistent Terminal";
      case kTerminal:
        return "Terminal";
      case kAllSymbols:
        return "All types";
      default:
        SDL_THROW_LOG(Sym, InvalidSymType, "invalid symbol type " << type);
        return "Invalid symbol type!";
    }
  }

  static inline char const* getTypeNameShort(SymbolType type) {
    switch (type) {
      case kSpecialTerminal:
        return "Special:";
      case kVariable:
        return "Variable:";
      case kPersistentNonterminal:
        return "Persistent-NT:";
      case kNonterminal:
        return "NT:";
      case kPersistentTerminal:
        return "Persistent:";
      case kTerminal:
        return ":";
      case kAllSymbols:
        return "All:";
      default:
        SDL_THROW_LOG(Sym, InvalidSymType, "invalid symbol type " << type);
        return "Invalid symbol type!";
    }
  }

  char const* getTypeName() const { return getTypeNameShort(type()); }
  char const* getTypeNameShort() const { return getTypeNameShort(type()); }

  /*
      "constructor" (static factory method).
   *
      Given an unsignedeger, a bit-mask indicating the type,
      this method verifies that the id is within range and creates
      an instance of Sym of the correct type.
   *
      This method should only be used by the Vocabulary and in unit tests.
   *
      @param identifier, the id.
      @param type, a bitmask identifying the type of the symbol.
   */
  static inline Sym createSym(SymInt index, SymbolType type) {
    Sym id;
    id.set(index, type);
    return id;
  }

#include <graehl/shared/warning_push.h>
  GCC_DIAG_IGNORE(uninitialized);
#if HAVE_GCC_4_8
  GCC_DIAG_IGNORE(maybe-uninitialized);
/* because of this (RuleBinarySerializer) gem: (doesn't understand that
 * code.decode, a virtual fn, will set SymInt x:

   Sym.hpp:207:17: warning: ‘x’ may be used uninitialized in this function
     [-Wmaybe-uninitialized] id_ = index | type;
 */
#endif
  void set(SymInt index, SymbolType type) {
    assert(!isPersistentType(type));
    assert(index <= kSmallSizeMask || index <= kLargeSizeMask && (SymInt)type == kLargeSizeMask);
    id_ = index | type;
  }
#include <graehl/shared/warning_pop.h>

  /**
     "constructor" (static factory method). that just sets id_ to identifier.
  */
  static inline Sym getVariableId(SymInt x) {
    assert(x <= (SymInt)kSmallSizeMask);
    Sym id;
    id.id_ = x | (SymInt)kVariable;
    return id;
  }

  static inline bool isSpecialType(SymbolType type) { return type == (SymInt)kSpecialTerminal; }

  static inline bool isTerminalType(SymbolType type) {
    assert(type != (SymInt)kPersistentTerminal);
    return type == (SymInt)kTerminal;
  }

  static inline bool isNonterminalType(SymbolType type) {
    assert(type != (SymInt)kPersistentNonterminal);
    return type == (SymInt)kNonterminal;
  }

  static inline bool isPersistentType(SymbolType type) {
    return type == (SymInt)kPersistentTerminal || type == (SymInt)kPersistentNonterminal;
  }

  /**
     type that gets cleared by resetCaches
  */
  static inline bool isClearableType(SymbolType type) {
    return type == (SymInt)kTerminal || type == (SymInt)kNonterminal;
  }

  static inline bool isVariableType(SymbolType type) { return type == (SymInt)kVariable; }

  static inline SymInt maxIndexForType(SymbolType typeId) {
    return (SymInt)(typeId == kLargeIdsStart ? kLargeSizeMask : kSmallSizeMask);
  }


#ifdef __clang__
#include <graehl/shared/warning_push.h>
#pragma clang diagnostic ignored "-Wtautological-compare"
// because one of the boundaries will be 0 and unsigned can't be < 0
#endif

  /** \return is terminal, including special, persistent or not
   */
  bool isTerminal() const {
    assert(!isPersistent());
    return id_ >= (SymInt)kBeginTerminal && id_ < (SymInt)kEndTerminal
           || id_ >= (SymInt)kSpecialTerminal && id_ < (SymInt)kEndSpecialTerminal;
  }

  bool isSpecialTerminal() const {
    assert(!isPersistent());
    return id_ >= (SymInt)kSpecialTerminal && id_ < (SymInt)kEndSpecialTerminal;
  }

  /** \return is a lexical non-special terminal symbol - kTerminal or kPeristentTerminal
   */
  bool isLexical() const {
    assert(!isPersistent());
    assert(((SymInt)(id_ + 1) > (SymInt)kTerminal)
           == (id_ >= (SymInt)kBeginTerminal && id_ < (SymInt)kEndTerminal));
    return (SymInt)(id_ + 1) > (SymInt)kTerminal;
  }

  /*
      Checks if the Sym instance is of type non-terminal.
   *
      The function will return true for non-special non-terminals
   */
  bool isNonterminal() const {
    assert(!isPersistent());
    return (id_ & kSmallTypeMask) == kNonterminal;
  }

  /*
      Checks if the Sym instance is of type variable.
   *
      The function will return true for all non-terminal symbol Id's, including
      non-persistent non-terminal, persistent non-terminal and special non-terminal symbols.
   */
  bool isVariable() const { return id_ >= (SymInt)kVariable && id_ < (SymInt)kEndVariable; }

  bool isPersistent() const { return isPersistentType(type()); }

  void removePersistentBit() {
    if (isPersistent()) id_ += (1u << 29);
  }

  void setTerminal() {
    assert(type() != kPersistentTerminal);
    id_ &= kMaxSmallSymIndex;
    id_ |= kTerminal;
  }

  bool isSpecial() const { return id_ >= (SymInt)kBeginSpecial && id_ < (SymInt)kEndSpecial; }

  SymInt maxIndex() const {
    return (SymInt)(id_ >= (SymInt)kLargeIdsStart ? kLargeSizeMask : kSmallSizeMask);
  }
#ifdef __clang__
#include <graehl/shared/warning_pop.h>
#endif

  /*
      Returns the index (without the type bitmask) for the Sym.
   *
      Sym::createSym(0, kPersistentTerminal).index() will return 0.
      Sym::createSym(10234, kPersistentTerminal).index() will return 10234.
      Sym::createSym(4, kVariable).index() will return 4.
      Sym::getVariableId(4).index() will return 4.
   */
  SymInt index() const { return id_ & maxIndex(); }


  /*
      set to NoSymbol
   */
  void reset() { id_ = (SymInt)kNoSymbol; }


  /** \return Checks if the Sym is not set to NoSymbol

      - simpler: just if(sym) or (bool)sym
  */
  bool isValid() const { return id_ != (SymInt)kNoSymbol; }

  bool operator!() const { return id_ == (SymInt)kNoSymbol; }

  bool operator==(Sym rhs) const { return id_ == rhs.id_; }

  bool operator==(SymInt rhs) const { return id_ == rhs; }

  bool operator!=(Sym rhs) const { return id_ != rhs.id_; }

  bool operator!=(SymInt rhs) const { return id_ != rhs; }

  bool operator<(Sym rhs) const { return id_ < rhs.id_; }

  bool operator<(SymInt rhs) const { return id_ < rhs; }

  bool operator>(Sym rhs) const { return id_ > rhs.id_; }

  bool operator>(SymInt rhs) const { return id_ > rhs; }

  bool operator<=(Sym rhs) const { return id_ <= rhs.id_; }

  bool operator<=(SymInt rhs) const { return id_ <= rhs; }

  bool operator>=(Sym rhs) const { return id_ >= rhs.id_; }

  bool operator>=(SymInt rhs) const { return id_ >= rhs; }

  bool incrementable() const { return ((id_ + 1) & kSmallSizeMask) || id_ == (SymInt)kLargeIncrementable; }

  /**
     preincrement - ++i
  */
  Sym& operator++() {
    assert(incrementable());
    ++id_;
    return *this;
  }

  /**
     postincrement - i++
  */
  Sym operator++(int) {
    assert(incrementable());
    Sym old(*this);
    ++id_;
    return old;
  }

  typedef boost::range_detail::safe_bool<SymInt Sym::*> safe_bool_t;
  typedef safe_bool_t::unspecified_bool_type unspecified_bool_type;

  operator unspecified_bool_type() const { return safe_bool_t::to_unspecified_bool(~id_, &Sym::id_); }

  /*
      used by KeyGeneratorUtil.hpp and in serializing {Phrase, Syntax}Rule
   */
  template <class Archive>
  void serialize(Archive& ar, const SymInt version) {
    ar& id_;
    assert(!isPersistent());
  }

  SymbolType type() const {
    if (id_ >= (SymInt)kLargeIdsStart)
      return ~id_ ? kLargeIdsStart : (SymbolType)id_;
    else
      return (SymbolType)(id_ & (SymInt)kSmallTypeMask);
  }

  SymInt id() const { return id_; }

  SymInt id_;
  bool isLexicalGivenIsTerminal() const {
    assert(isTerminal());
    return (1 << 31) & id_;
  }

  void operator+=(SymInt deltaIndex) {
    assert(index() + deltaIndex <= maxIndex());
    id_ += deltaIndex;
  }
  void operator-=(SymInt deltaIndex) {
    assert(index() > deltaIndex);
    id_ -= deltaIndex;
  }
  void operator+=(Sym delta) {
    assert(delta.type() == type());
    operator+=(delta.index());
  }
  void operator-=(Sym delta) {
    assert(delta.type() == type());
    operator-=(delta.index());
  }
};

enum { kMaxNTIndex = (SymInt)Sym::kSmallSizeMask };
enum { kNumPossibleNT = (SymInt)(kMaxNTIndex + 1) };
enum { kMaxTerminalIndex = (SymInt)Sym::kLargeSizeMask };
enum { kNumPossibleTerminal = (SymInt)(kMaxTerminalIndex + 1) };

/// we should be safe to use this during static init (because you'll have an
/// initialized version in every compliation unit)
static const Sym NoSymbol = {(SymInt)kNoSymbol};

inline Sym terminal(SymInt index) {
  Sym sym;
  assert(index <= kMaxTerminalIndex);
  sym.id_ = kTerminal | index;
  return sym;
}


inline Sym specialTerminal(SymInt index) {
  Sym sym;
  assert(index <= kMaxNTIndex);
  sym.id_ = index;
  return sym;
}


inline Sym operator+(SymInt index, Sym sym) {
  Sym r(sym);
  r += index;
  return r;
}

template <class Rhs>
Sym operator+(Sym lhs, Rhs rhs) {
  Sym r(lhs);
  r += rhs;
  return r;
}

template <class Rhs>
Sym operator-(Sym lhs, Rhs rhs) {
  Sym r(lhs);
  r -= rhs;
  return r;
}

/**
   WARNING: saving a bloom filter to disk and expecting to * use it later means
   you can't change your hash function. Therefore SDL_SYMID_FAST_HASH is 0 for now

   CT doesn't use bloom filters yet, but we do have a RegressionTest/xmt/regtest-chieng-bloom.yml

*/

// TODO: 1 (regr may change slightly)
#define SDL_SYMID_FAST_HASH 0
inline std::size_t hash_value(Sym const symId) {
#if SDL_SYMID_FAST_HASH
  return (symId.id_ + (symId.id_ >> 27) + (symId.id_ << 4));
// fast and probably good enough - >> 26 is to mix the type bits into lower (29 would be more aggressive)
#else
  // slower but perhaps higher quality //TODO@JG: check regression difference; if none, use fast permanently
  boost::hash<SymInt> hasher;
  return hasher(symId.id_);
#endif
}

inline std::ostream& operator<<(std::ostream& os, Sym symId) {
  os << symId.id_;
  return os;
}

inline std::istream& operator>>(std::istream& is, Sym& symId) {
  is >> symId.id_;
  return is;
}
}

namespace std {
template <>
struct hash<sdl::Sym> {
  size_t operator()(sdl::Sym x) const { return hash_value(x); }
};
}

BOOST_CLASS_IMPLEMENTATION(sdl::Sym, object_serializable)
// BOOST_CLASS_IMPLEMENTATION(sdl::Sym, primitive_type)
BOOST_IS_BITWISE_SERIALIZABLE(sdl::Sym)

#endif
