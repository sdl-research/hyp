/** \file

    copy hg fsm to openfst w/ same weight type
*/

#ifndef SDL_HYP__TOOPENFST_LW201213_HPP
#define SDL_HYP__TOOPENFST_LW201213_HPP
#pragma once

// TODO: Make our Hypergraph derive from OpenFst class instead of copying?

#if HAVE_OPENFST
#include <sdl/Hypergraph/UseOpenFst.hpp>
#include <sdl/LexicalCast.hpp>

#include <fst/symbol-table.h>
#include <fst/vector-fst.h>
#include <fst/script/fst-class.h>

#include <sdl/Util/Override.hpp>
#include <sdl/Exception.hpp>
#include <sdl/Util/FnReference.hpp>

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/HypergraphWriter.hpp> //dbg print

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
  virtual fst::SymbolTable* Copy() const OVERRIDE { return new IVocabularySymbolTable(*this); }
  virtual string Find(int64 key) const OVERRIDE {
    Sym id = {(unsigned)key};
    return pVoc->str(id);
  }

  virtual int64 AddSymbol(string const&) OVERRIDE {
    SDL_THROW0(UnimplementedException);
    return 0;
  }
  virtual int64 AddSymbol(string const&, int64) OVERRIDE {
    SDL_THROW0(UnimplementedException);
    return 0;
  }
  void AddTable(const SymbolTable& table) OVERRIDE { SDL_THROW0(UnimplementedException); }
  virtual string CheckSum() const OVERRIDE {
    SDL_THROW0(UnimplementedException);
    return string();
  }
  virtual string LabeledCheckSum() const OVERRIDE {
    SDL_THROW0(UnimplementedException);
    return string();
  }
  virtual bool Write(std::ostream& strm) const OVERRIDE {
    SDL_THROW0(UnimplementedException);
    return false;
  }
  virtual bool WriteText(std::ostream& strm) const OVERRIDE {
    SDL_THROW0(UnimplementedException);
    return false;
  }
  virtual int64 Find(const string& symbol) const OVERRIDE {
    SDL_THROW0(UnimplementedException);
    return 0;
  }
  virtual int64 Find(const char* symbol) const OVERRIDE {
    SDL_THROW0(UnimplementedException);
    return 0;
  }
  virtual int64 AvailableKey(void) const OVERRIDE {
    SDL_THROW0(UnimplementedException);
    return 0;
  }
  virtual ::size_t NumSymbols(void) const OVERRIDE {
    SDL_THROW0(UnimplementedException);
    return 0;
  }
  virtual int64 GetNthKey(ssize_t pos) const OVERRIDE {
    SDL_THROW0(UnimplementedException);
    return 0;
  }
};

struct StateNamesSymbolTable : public IVocabularySymbolTable {
  Syms ssym;

  template <class A>
  explicit StateNamesSymbolTable(IHypergraph<A> const& h, LabelType labelType = kInput,
                                 bool allowLexical = false)
      : IVocabularySymbolTable(h.getVocabulary()), ssym(h.size(), NoSymbol) {
    for (StateId s = 0, ns = ssym.size(); s != ns; ++s) {
      Sym i = h.label(s, labelType);
      if (allowLexical || !i.isLexical()) ssym[s] = i;
    }
  }

  virtual string Find(int64 key) const OVERRIDE {
    Sym l = ssym[key];
    if (l == NoSymbol) return sdl::lexical_cast<string>(key);
    return pVoc->str(l);
  }
};

template <class Arc, class FArc = fst::ArcTpl<typename Arc::Weight> >
struct ToOpenFst : boost::noncopyable {
  IVocabularySymbolTable syms;
  typedef shared_ptr<StateNamesSymbolTable> Ssymp;
  Ssymp pssyms;
  typedef fst::script::FstClass FstClass;
  typedef typename Arc::Weight Weight;
  typedef FArc FstArc;
  typedef typename FstArc::Weight FWeight;
  IHypergraph<Arc> const& h;
  fst::VectorFst<FstArc> fst;

  void operator()(Arc const* a) {
    LabelPair io = h.labelPair(a->fsmSymbolState());
    fst.AddArc(a->fsmSrc(),
               FstArc(input(io).id(), output(io).id(), FWeight(a->weight().getValue()), a->head()));
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
    h.forArcs(Util::visitorReference(*this));
  }

  typedef fst::Fst<FstArc> Fst;

  Fst& getFst() { return fst; }

  fst::SymbolTable const& symbolTable() const { return syms; }

  fst::SymbolTable const* stateNames() const { return pssyms.get(); }
};


}}

#endif

#endif
