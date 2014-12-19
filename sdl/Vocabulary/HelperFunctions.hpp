/** \file

    things that could be in IVocabulary but aren't.
*/

#ifndef SDL_VOCABULARY_HELPERFUNCTIONS_HPP
#define SDL_VOCABULARY_HELPERFUNCTIONS_HPP
#pragma once

#include <ostream>

#include <sdl/SharedPtr.hpp>
#include <sdl/Syms.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Util/PrintRange.hpp>
#include <sdl/Util/StringBuilder.hpp>
#include <sdl/Util/ObjectCount.hpp>
#include <sdl/Util/ThreadId.hpp>

namespace sdl {
namespace Vocabulary {

struct VocabularyResidentLeakCheck : Util::LeakCheckBase {
  IVocabulary const& vocab;
  VocabularyResidentLeakCheck(IVocabulary const& vocab) : vocab(vocab) {
    init("VocabularyResidentLeakCheck");
  }
  void print(std::ostream& o) const { o << "Resident:" << vocab; }
  std::size_t size() const { return vocab.residentSize(); }
};

struct VocabularyUnkLeakCheck : Util::LeakCheckBase {
  IVocabulary const& vocab;
  VocabularyUnkLeakCheck(IVocabulary const& vocab) : vocab(vocab) { init("VocabularyUnkLeakCheck"); }
  void print(std::ostream& o) const { o << "Unk:" << vocab; }
  std::size_t size() const { return vocab.countSinceFreeze(); }
};

/// oddly, # read only symbols changes after init - maybe grammar wasn't fully loaded yet?
struct VocabularyLeakCheck : Util::LeakCheckBase {
  IVocabulary const& vocab;
  VocabularyLeakCheck(IVocabulary const& vocab) : vocab(vocab) { init("VocabularyLeakCheck"); }
  void print(std::ostream& o) const { o << "Size:" << vocab; }
  std::size_t size() const { return vocab.size(); }
};

/**
   Creates default (Resident) vocabulary.
 */
IVocabularyPtr createDefaultVocab();

/**
   Simple function that looks up Syms in the vocab
   and prints them.

   \param begin The begin of the sequence, e.g., begin() in
   std::vector<Sym> or Syms.

   \param pVoc The Vocabulary to map from Sym to the
   string symbols; may be empty (in which case the Syms
   are printed)
 */
template <class ForwardIterator>
void lookupAndPrintSymbols(ForwardIterator begin, ForwardIterator end, IVocabulary const* pVoc,
                           std::ostream& out) {
  Util::Sep space(" ");
  for (ForwardIterator symbolIdIter = begin; symbolIdIter != end; ++symbolIdIter) {
    if (pVoc)
      out << space << pVoc->str(*symbolIdIter);
    else
      out << space << *symbolIdIter;
  }
}

template <class ForwardIterator>
void lookupAndPrintSymbols(ForwardIterator begin, ForwardIterator end, IVocabularyPtr const& pVoc,
                           std::ostream& out) {
  lookupAndPrintSymbols(begin, end, pVoc.get(), out);
}

/**
   Function overload for convenience.
 */
template <class Container>
void lookupAndPrintSymbols(Container const& container, IVocabulary const* pVoc, std::ostream& out) {
  lookupAndPrintSymbols(container.begin(), container.end(), pVoc, out);
}

template <class Container>
void lookupAndPrintSymbols(Container const& container, IVocabularyPtr const& pVoc, std::ostream& out) {
  lookupAndPrintSymbols(container.begin(), container.end(), pVoc, out);
}

/// for use in debugging and logging
template <class ForwardIterator>
std::string lookupSymbolsToString(ForwardIterator begin, ForwardIterator end, IVocabulary const* pVoc) {
  std::stringstream ss;
  lookupAndPrintSymbols(begin, end, pVoc, ss);
  return ss.str();
}

template <class ForwardIterator>
std::string lookupSymbolsToString(ForwardIterator begin, ForwardIterator end, IVocabularyPtr const& pVoc) {
  std::stringstream ss;
  lookupAndPrintSymbols(begin, end, pVoc, ss);
  return ss.str();
}

/**
   Maps space-separated terminal tokens to Symbols,
       appends to the passed-in vector,
       and as a side-effect adds novel tokens to dictionary

   \param text space-separated terminal text tokens

   \param syms Symbol vector to be appended to

   \param pVocabulary Vocabulary for lookup and insertion

   appends to output Syms
 */
void terminalTextToSignature(std::string const& text, Syms* syms, IVocabularyPtr const& pVocabulary);

void terminalTextToSignature(std::string const& text, Syms* syms, IVocabulary& vocab);

void terminalStringsToSignature(std::vector<std::string> const& strs, Syms* syms, IVocabulary& vocab);

/**
   Removes block symbols from ngram
 */
void removeBlockSymbols(Syms const& ngram, Syms& result);
Syms removeBlockSymbols(Syms const& ngram);
}

struct PrintSyms {
  Syms const& syms;
  IVocabulary const* voc;
  explicit PrintSyms(Syms const& syms, IVocabulary const* voc = 0) : syms(syms), voc(voc) {}
  PrintSyms(Syms const& syms, IVocabulary const& voc) : syms(syms), voc(&voc) {}
  friend inline std::ostream& operator<<(std::ostream& out, PrintSyms const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const { Vocabulary::lookupAndPrintSymbols(syms, voc, out); }
};


}

#endif
