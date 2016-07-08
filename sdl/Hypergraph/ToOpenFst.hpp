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

    copy hg fsm to openfst w/ same weight type
*/

#ifndef SDL_HYP__TOOPENFST_LW201213_HPP
#define SDL_HYP__TOOPENFST_LW201213_HPP
#pragma once

// TODO: Make our Hypergraph derive from OpenFst class instead of copying?

#if HAVE_OPENFST
#include <sdl/Hypergraph/HypergraphWriter.hpp>  //dbg print
#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/UseOpenFst.hpp>
#include <sdl/Exception.hpp>
#include <sdl/LexicalCast.hpp>
#include <fst/script/fst-class.h>
#include <fst/symbol-table.h>
#include <fst/vector-fst.h>

namespace sdl {
namespace Hypergraph {

// complaint: symbol id 0 for epsilon seems to be required by openfst. other special symbols' ids too? but
// this should be sufficient for printing.
// read-only access to an IVocabulary using openfst SymbolTable interface
struct IVocabularySymbolTable : public fst::SymbolTable {  // unfortunate: this means we have an empty openfst
  // symbol table implementation as well
  typedef fst::SymbolTable Base;
  typedef IVocabularySymbolTable self_type;
  IVocabularyPtr pVoc;
  explicit IVocabularySymbolTable(IVocabularyPtr pVoc = IVocabularyPtr())
      : Base("IVocabularySymbolTable"), pVoc(pVoc) {}
  IVocabularySymbolTable(self_type const& o) : Base("IVocabularySymbolTable"), pVoc(o.pVoc) {}
  typedef std::string string;
  typedef ::int64 int64;  // fst/types.h
  virtual fst::SymbolTable* Copy() const override { return new IVocabularySymbolTable(*this); }
  virtual string Find(int64 key) const override {
    Sym id = {(unsigned)key};
    return pVoc->str(id);
  }

  virtual int64 AddSymbol(string const&) override {
    SDL_THROW0(UnimplementedException);
    return 0;
  }
  virtual int64 AddSymbol(string const&, int64) override {
    SDL_THROW0(UnimplementedException);
    return 0;
  }
  void AddTable(SymbolTable const& table) override { SDL_THROW0(UnimplementedException); }
  virtual string CheckSum() const override {
    SDL_THROW0(UnimplementedException);
    return string();
  }
  virtual string LabeledCheckSum() const override {
    SDL_THROW0(UnimplementedException);
    return string();
  }
  virtual bool Write(std::ostream& strm) const override {
    SDL_THROW0(UnimplementedException);
    return false;
  }
  virtual bool WriteText(std::ostream& strm) const override {
    SDL_THROW0(UnimplementedException);
    return false;
  }
  virtual int64 Find(string const& symbol) const override {
    SDL_THROW0(UnimplementedException);
    return 0;
  }
  virtual int64 Find(char const* symbol) const override {
    SDL_THROW0(UnimplementedException);
    return 0;
  }
  virtual int64 AvailableKey(void) const override {
    SDL_THROW0(UnimplementedException);
    return 0;
  }
  virtual ::size_t NumSymbols(void) const override {
    SDL_THROW0(UnimplementedException);
    return 0;
  }
  virtual int64 GetNthKey(ssize_t pos) const override {
    SDL_THROW0(UnimplementedException);
    return 0;
  }
};

struct StateNamesSymbolTable : public IVocabularySymbolTable {
  Syms ssym;

  template <class A>
  explicit StateNamesSymbolTable(IHypergraph<A> const& h, LabelType labelType = kInput, bool allowLexical = false)
      : IVocabularySymbolTable(h.getVocabulary()), ssym(h.size(), NoSymbol) {
    for (StateId s = 0, ns = ssym.size(); s != ns; ++s) {
      Sym i = h.label(s, labelType);
      if (allowLexical || !i.isLexical()) ssym[s] = i;
    }
  }

  virtual string Find(int64 key) const override {
    Sym l = ssym[key];
    if (l == NoSymbol) return sdl::lexical_cast<std::string>(key);
    return pVoc->str(l);
  }
};

template <class Arc, class FArc = fst::ArcTpl<typename Arc::Weight>>
struct ToOpenFst : boost::noncopyable {
  IVocabularySymbolTable syms;
  typedef shared_ptr<StateNamesSymbolTable> Ssymp;
  Ssymp pssyms;
  typedef fst::script::FstClass FstClass;
  typedef typename Arc::Weight Weight;
  typedef FArc FstArc;
  typedef typename FstArc::Weight FWeight;
  IHypergraph<Arc> const& h;
  mutable fst::VectorFst<FstArc> fst;

  void operator()(Arc const* a) const {
    LabelPair io = h.labelPair(a->fsmSymbolState());
    fst.AddArc(a->fsmSrc(), FstArc(input(io).id(), output(io).id(), FWeight(a->weight().getValue()), a->head()));
  }

  ToOpenFst(IHypergraph<Arc> const& h, bool useStatenames = false) : h(h), syms(h.getVocabulary()) {
    if (useStatenames) pssyms.reset(new StateNamesSymbolTable(h));
    fst.DeleteStates();
    fst.SetInputSymbols(&syms);
    fst.SetOutputSymbols(&syms);
    for (StateId i = 0, e = h.size(); i != e; ++i) {
      StateId s = fst.AddState();
      assert(s == i);
    }
    StateId s = h.start(), f = h.final();
    if (s != Hypergraph::kNoState) fst.SetStart(s);
    if (f != Hypergraph::kNoState) fst.SetFinal(f, FWeight::One());
    h.forArcs(*this);
  }

  typedef fst::Fst<FstArc> Fst;

  Fst& getFst() { return fst; }

  fst::SymbolTable const& symbolTable() const { return syms; }

  fst::SymbolTable const* stateNames() const { return pssyms.get(); }
};
}
}

#endif


#endif
