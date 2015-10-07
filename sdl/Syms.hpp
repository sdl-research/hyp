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

    Syms (list of Syms)

    hash fns for Syms

    utility fns for Sym -> str

    this is not part of Sym so that Vocabulary and others can have a minimal dependency on Sym itself
*/

#ifndef SYMIDS_JG_2013_04_26_HPP
#define SYMIDS_JG_2013_04_26_HPP
#pragma once

#include <cstdio>
#include <sdl/Sym.hpp>
#include <sdl/Util/PrintRange.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Util/SmallVector.hpp>

namespace sdl {

typedef SymInt SymsIndex;
typedef std::pair<SymInt, SymInt> SymSpan;
typedef Sym const* Psym;
typedef std::pair<Psym, Psym> SymSlice;
typedef graehl::pod_array_ref<Sym> SymPodArrayRef; //TODO: use this instead of SymSlice. faster <=> Syms.

enum { kInlineSyms = 3 };
// 16 bytes (more if >3 elements). note: small_vector<SymInt, 1> and <SymInt, 2> have the same size due to alignment.
// For release builds, this is faster (by about 1-2%)

typedef Util::small_vector<Sym, kInlineSyms, SymsIndex> Syms;

// 16 on stack only for small phrases (length <=3). else similar to std::vector (array on heap). TODO: test time/space tradeoff w/ 4 or higher?
// note: small_vector<SymInt, 1> and <SymInt, 2> have the same size due to alignment.

inline bool isLexical(Syms const& syms) {
  for (Syms::const_iterator i = syms.begin(), e = syms.end(); i != e; ++i)
    if (!i->isLexical())
      return false;
  return true;
}

inline unsigned nLexical(Syms const& syms) {
  unsigned count = 0;
  for (Syms::const_iterator i = syms.begin(), e = syms.end(); i != e; ++i)
    count += i->isLexical();
  return count;
}


/**
   explicit hash fn for Syms keys

   we could just use boost::hash_range, but this is explicit and perhaps
   slightly more stable across platforms and versions - for serialization of
   hashed structures, and same-regression stateid output.
*/

template<class Itr>
std::size_t RangeHash(Itr itr_start, Itr itr_end, std::size_t total_hash = 0)
{
  for (; itr_start != itr_end; itr_start++)
    boost::hash_combine(total_hash, *itr_start);
  return total_hash;
}

template<class Container>
std::size_t RangeHash(Container c, std::size_t total_hash = 0)
{
  return RangeHash(c.begin(), c.end(), total_hash);
}

/**
   TODO@JG: eventually use a more efficient pod-array-hashing (64-bits or larger
   chunks at a time) fn. should be a 1-2% overall speedup
*/
struct SymsHashOperator
{
  std::size_t operator()( Syms const* sig ) const
  {
    return RangeHash(sig->begin(), sig->end());
  }

  std::size_t operator()( Syms * sig ) const
  {
    return RangeHash(sig->begin(), sig->end());
  }

  std::size_t operator()( Syms const& sig ) const
  {
    return RangeHash(sig.begin(), sig.end());
  }

};


inline std::ostream& operator<<(std::ostream& os, Syms const& ngram) {
  Util::printRange(os, ngram, Util::RangeSep(" ","<",">"));
  return os;
}

/**
   return beginning of interior all-lexical region, setting lenInterior
*/
inline Sym const* lexicalInteriorLength(Syms const& syms, SymsIndex &lenInterior) {
  lenInterior = (SymsIndex)syms.size();
  if (lenInterior) {
    SymsIndex len = lenInterior;
    typedef Sym const* Psym;
    Psym interior = &syms[0];
    if (!interior->isLexical()) {
      // find the juicy all-lexical middle part of interior, skipping open/close
      // block for ConstraintSubstitute, and epsilon for translating noswap
      // block open/close, or actual epsilon lattice input
      while (--len && !(++interior)->isLexical()) ;
      Psym last = interior + len;
      while (last > interior && !(--last)->isLexical()) ;
      len = (SymsIndex)(last - interior);
    }
    assert(!len || interior->isLexical() && interior[len-1].isLexical());
    lenInterior = len;
    return interior;
  } else
    return 0;
}

//TODO: it would be nice to put print in a .cpp somewhere since performance
//isn't critical. but we have no lib for the toplevel xmt/ headers.
inline void print(std::ostream &out, Sym sym, IVocabulary const* voc
                  , char const* variablePrefix="X") {
  //TODO: consider always escaping for syntax rule formats (anything with
  //delimiters other than space, which we never allow inside tokens, etc)
  if (variablePrefix && sym.isVariable())
    out << "X" << sym.index();
  // some unit tests use vocabularies that don't set a variables voc. if
  // that's fixed, we can remove the variablePrefix arg
  else if (voc)
    out << voc->str(sym); // if you want quotes + escaping, use Hypergraph/SymbolPrint.hpp
  else
    out << sym.getTypeNameShort() << sym.index();
}

inline void print(std::ostream &out, Sym sym, IVocabularyPtr const& voc
                  , char const* variablePrefix="X") {
  print(out, sym, voc.get(), variablePrefix);
}

/**
   to be found by ADL for out << Util::print(phrase, voc).
*/
inline void print(std::ostream &out, Syms const& phrase, IVocabulary const* voc) {
  Util::printRangeState(out, voc, phrase);
}

/**
   to be found by ADL for out << Util::print(phrase, voc).
*/
inline void print(std::ostream &out, Syms const& phrase, IVocabulary const& voc) {
  Util::printRangeState(out, &voc, phrase);
}

inline void print(std::ostream &out, Syms const& phrase, IVocabularyPtr const& voc) {
  Util::printRangeState(out, voc.get(), phrase);
}


}

BOOST_CLASS_IMPLEMENTATION(sdl::Syms, object_serializable)

#endif
