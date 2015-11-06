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

    since most of these are trivial wrappers to existing IHypergraph
    virtual fns, they should be inline and not hidden in .cpp; on the other
    hand, since they're not virtual they could well be optional free fns instead
    of part of the IHypergraph interface.

    //TODO: make these inline
    */

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/src/IsGraphArc.ipp>
#include <sdl/Hypergraph/SymbolPrint.hpp>
#include <sdl/Hypergraph/HypergraphWriter.hpp>
#include <sdl/Hypergraph/ContainsEmptyString.hpp>
#include <sdl/Util/Once.hpp>
#include <sdl/Util/PointerSet.hpp>
#include <algorithm>

namespace sdl {
namespace Hypergraph {

StateId maxTail(HypergraphBase const& h) {
  StateId N = h.size();
  if (N == 0) return kNoState;
  if (h.storesOutArcs()) {
    for (StateId state = N; state;) {
      --state;
      if (h.numOutArcs(state)) return state;
    }
    return kNoState;
  }
  StateId r = kNoState;
  if (h.storesInArcs()) {
    for (StateId state = 0; state < N; ++state) {
      for (ArcId t = 0, e = h.numInArcs(state); t != e; ++t) {
        StateIdContainer const& tails = h.inArc(state, t)->tails_;
        if (tails.empty()) continue;
        StateId mt = *std::max_element(tails.begin(), tails.end());
        if (r == kNoState || mt > r) r = mt;
      }
    }
  }
  return r;
}

StateId maxHead(HypergraphBase const& h) {
  StateId N = h.size();
  if (N == 0) return kNoState;
  if (h.storesInArcs()) {
    for (StateId state = N; state;) {
      --state;
      if (h.numInArcs(state)) return state;
    }
    return kNoState;
  }
  StateId r = kNoState;
  if (h.storesOutArcs()) {
    for (StateId state = 0; state < N; ++state) {
      for (ArcId t = 0, e = h.numOutArcs(state); t != e; ++t) {
        StateId hd = h.outArc(state, t)->head_;
        if (r == kNoState || hd > r) r = hd;
      }
    }
  }
  return r;
}

StateId maxArcState(HypergraphBase const& h) {
  return maxState(maxHead(h), maxTail(h));
}

// I don't bother to take a lazy max (stop early if search for one component falls < previously found max)
StateId maxState(HypergraphBase const& h) {
  StateId r = maxArcState(h);
  StateId state = h.start(), f = h.final();
  r = maxState(r, state);
  r = maxState(r, f);
  r = maxState(r, h.maxLabeledState());
  return r;
}

void HypergraphBase::printArc(std::ostream& out, ArcBase const* a, bool inlineLabel) const {
  Hypergraph::printArc(out, a, this, inlineLabel);
}

void HypergraphBase::printArc(std::ostream& out, ArcBase const* a) const {
  this->printArc(out, a, this->hasGraphInlineLabels());
}

struct CallPrintArc {
  HypergraphBase const& hg;
  std::ostream& out;
  bool inlineLabels;
  CallPrintArc(HypergraphBase const& hg, std::ostream& out)
      : hg(hg), out(out), inlineLabels(hg.hasGraphInlineLabels()) {}
  void operator()(ArcBase const* arc) const {
    hg.printArc(out, arc, inlineLabels);
    out << '\n';
  }
};

void HypergraphBase::printStderr() const {
  printUnchecked(std::cerr);
}


struct MoreThanOnceException : public std::exception {
  char const* what() const throw() override { return "pointer visited more than once"; }
  void const* duplicate;
  MoreThanOnceException(void const* duplicate) : duplicate(duplicate) {}
};

struct CheckOnce {
  CheckOnce(Util::PointerSet& p) : p(&p) {}
  Util::PointerSet* p;
  // mutating via pointer p - const helps compiler out?
  bool first(void const* v) const {
    return p->insert((intptr_t)v).second;
  }
  void operator()(void const* v) const {
    if (!first(v)) throw MoreThanOnceException(v);
  }
};

void HypergraphBase::printUnchecked(std::ostream &out) const {
  out << *this;
}

bool HypergraphBase::checkValid() const {
  StateId N = size();
  StateId maxhead = maxHead(*this);
  assert(maxhead == kNoState || maxhead < N);
  StateId maxtail = maxTail(*this);
  assert(maxtail == kNoState || maxtail < N);
  StateId s = start(), f = final();
  assert(s == kNoState || s < N);
  assert(f == kNoState || f < N);
  Util::Once once;
  if (storesInArcs() || storesFirstTailOutArcs()) try {
      forArcs(CheckOnce(once));
    } catch (MoreThanOnceException& e) {
      SDL_ERROR(Hypergraph.CheckValid,
                "arc stored more than once: " << e.duplicate << " = "
                                              << printer(*(ArcBase const*)e.duplicate, *this));
      SDL_DEBUG(Hypergraph.CheckValid, PrintUnchecked(*this));
      return false;
    }
  return true;
}

inline void printStartOrFinal(char const* prefix, std::ostream& out, HypergraphBase const& hg, StateId s) {
  if (s != kNoState) {
    out << prefix << " <- ";
    printState(out, s, hg);
    out << '\n';
  }
}

void printStartAndFinal(std::ostream &out, HypergraphBase const& hg) {
  printStartOrFinal("START", out, hg, hg.start());
  printStartOrFinal("FINAL", out, hg, hg.final());
}

void HypergraphBase::print(std::ostream& out) const {
  printStartOrFinal("START", out, *this, this->start());
  this->forArcs(CallPrintArc(*this, out));
  printStartOrFinal("FINAL", out, *this, this->final());
}

bool HypergraphBase::isGraphArc(ArcBase const& a, bool& fsm, bool& oneLexical) const {
  return detail::isGraphArcImpl(*this, a, fsm, oneLexical);
}

bool HypergraphBase::hasSortedArcs() const {
  Properties p = this->properties();
  assert(storesOutArcs());
  return p & kSortedOutArcs;
}

bool HypergraphBase::isMutable() const {
  return false;
}

// may still be empty even if final state exists (can't be reached
// from start/axioms). also, no longer checks #of arcs since that takes time
bool HypergraphBase::prunedEmpty() const {
  StateId sfinal = final();
  if (sfinal == Hypergraph::kNoState) return true;
  StateId sstart = start();
  if (sfinal == sstart) return false;
  Properties p = properties();
  if (p & kStoreInArcs)
    return numInArcs(sfinal) == 0;
  else if ((p & kStoreAnyOutArcs) && sstart != Hypergraph::kNoState)
    return numOutArcs(sstart) == 0;
  return false;
}

LabelPair HypergraphBase::labelPair(StateId s) const {  // may be faster if overriden
  return LabelPair(inputLabel(s), outputLabel(s));
}


// TODO: allow setting of per-state axiom weights, or just enforce 0-tails arcs for axioms.

bool HypergraphBase::isAxiom(StateId s) const {
  return s == start() || hasTerminalLabel(s);
}

bool HypergraphBase::storesArcs() const {
  return properties() & kStoreAnyArcs;
}

bool HypergraphBase::storesInArcs() const {
  return properties() & kStoreInArcs;
}

bool HypergraphBase::storesOutArcs() const {
  return properties() & kStoreAnyOutArcs;
}

bool HypergraphBase::storesAllOutArcs() const {
  return properties() & kStoreOutArcs;
}

bool HypergraphBase::storesFirstTailOutArcs() const {
  return properties() & kStoreFirstTailOutArcs;
}

bool HypergraphBase::hasLabel(StateId id) const {
  return inputLabel(id) != NoSymbol;
}

/**
   add existing labels to new vocab, and setVocabulary(vocab).
*/
void HypergraphBase::translateToVocabulary(IVocabularyPtr const& pNewVocab) {
  if (!pNewVocab) return;
  IVocabulary& newVocab = *pNewVocab;
  IVocabulary const& oldVocab = *vocab();
  if (&newVocab == &oldVocab) return;
  for (StateId s = 0, n = this->size(); s < n; ++s) {
    if (outputLabelFollowsInput(s)) {
      Sym const in = inputLabel(s);
      if (in) setInputLabel(s, newVocab.add(in, oldVocab));
    } else {
      // TODO: test
      LabelPair inOut = labelPair(s);
      assert(inOut.first);
      assert(inOut.second);
      newVocab.translateSymbol(inOut.first, oldVocab);
      newVocab.translateSymbol(inOut.second, oldVocab);
      setLabelPair(s, inOut);
    }
  }
  setVocabulary(pNewVocab);  // done at end because releasing shared_ptr may destroy old vocab
}

void printState(std::ostream& out, StateId s, HypergraphBase const& hg, bool inlineLabel) {
  out << s;
  IVocabulary const* voc = hg.vocab();
  StateId nstates = hg.size();
  if (inlineLabel || s > nstates) {
    Sym sym;
    sym.id_ = s;
    out << '(';
    writeLabel(out, sym, voc);
    out << ')';
  } else {
    LabelPair io(hg.labelPair(s));
    if (io.first || io.second) {
      out << '(';
      if (io.first) {
        writeLabel(out, io.first, voc);
        if (io.second && io.second != io.first) writeLabel(out << ' ', io.second, voc);
      } else
        writeLabel(out << ' ', io.second, voc);
      // "( out-label)" for no in-label vs "(in-label)"
      out << ')';
    }
  }
}

void printState(std::ostream& out, StateId s, HypergraphBase const* hg, bool inlineLabel) {
  if (hg)
    printState(out, s, *hg, inlineLabel);
  else
    out << s;
}

void printArcTails(std::ostream& out, StateIdContainer const& tails, HypergraphBase const* hg,
                   bool inlineGraphLabels) {
  bool again = false;
  for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
    if (again) out << ' ';
    printState(out, *i, hg, inlineGraphLabels && again);
    again = true;
  }
}

void printArc(std::ostream& out, ArcBase const* arc, HypergraphBase const* hg, bool inlineGraphLabels) {
  printState(out, arc->head_, hg);
  out << " <- ";
  printArcTails(out, arc->tails_, hg, inlineGraphLabels);
}

void printArc(std::ostream& out, ArcBase const* arc, HypergraphBase const* hg) {
  printArc(out, arc, hg, hg ? hg->hasGraphInlineLabels() : false);
}

HypergraphBase::AdjsPtr HypergraphBase::inArcs() const {
  Adjs* p = new Adjs;
  AdjsPtr r(p);
  inArcs(*p);
  return r;
}

HypergraphBase::AdjsPtr HypergraphBase::getFirstTailOutArcs(bool allowNonFirstTail) const {
  Adjs* p = new Adjs;
  AdjsPtr r(p);
  getFirstTailOutArcs(*p, allowNonFirstTail);
  return r;
}

void HypergraphBase::getFirstTailOutArcs(Adjs& adjs, bool allowNonFirstTail) const {
  StateId N = size();
  adjs.resize(N);
  Properties prop = properties();
  if (prop & (kStoreFirstTailOutArcs | kStoreOutArcs)) {
    if (prop & kStoreFirstTailOutArcs) allowNonFirstTail = true;
    for (StateId i = 0; i < N; ++i) getFirstTailOutArcs(i, adjs[i], allowNonFirstTail);
  } else {
    for (StateId i = 0; i < N; ++i)
      for (ArcId a = 0, f = numInArcs(i); a < f; ++a) {
        Arc* pa = inArc(i, a);
        adjs[pa->graphSrc()].push_back(pa);
      }
  }
}

void HypergraphBase::getFirstTailOutArcs(StateId st, ArcsContainer& arcs, bool allowNonFirstTail) const {
  if (allowNonFirstTail || storesFirstTailOutArcs())
    return this->outArcs(st, arcs);
  else {
    ArcId i = 0, N = this->numOutArcs(st);
    arcs.resize(N);
    ArcsContainer::iterator o0 = arcs.begin(), o = o0;
    for (; i < N; ++i) {
      Arc* a = this->outArc(st, i);
      if (a->graphSrc() == st) *o++ = a;
    }
    arcs.resize((ArcId)(o - o0));
  }
}

void HypergraphBase::outArcs(StateId st, ArcsContainer& arcs) const {
  ArcId i = 0, N = this->numOutArcs(st);
  arcs.resize(N);
  for (; i < N; ++i) arcs[i] = this->outArc(st, i);
}

void HypergraphBase::inArcs(Adjs& adjs) const {
  StateId N = size();
  adjs.resize(N);
  if (storesInArcs())
    for (StateId i = 0; i < N; ++i) inArcs(i, adjs[i]);
  else {
    for (StateId i = 0; i < N; ++i)
      for (ArcId a = 0, f = numOutArcs(i); a < f; ++a) {
        Arc* pa = outArc(i, a);
        adjs[pa->head_].push_back(pa);
      }
  }
}

void HypergraphBase::inArcs(StateId st, ArcsContainer& arcs) const {
  ArcId i = 0, N = this->numInArcs(st);
  arcs.resize(N);
  for (; i < N; ++i) arcs[i] = this->inArc(st, i);
}

void HypergraphBase::addStateId(StateId state, LabelPair l) {
  addStateId(state);
  setLabelPair(state, l);
}

bool HypergraphBase::containsEmptyString() const {
  return Hypergraph::containsEmptyString(*this);
}


}}
