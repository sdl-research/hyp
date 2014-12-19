// Copyright 2014 SDL plc
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/** \file

    print a hypergraph.
*/

#ifndef HYP__HYPERGRAPH_HYPERGRAPHWRITER_HPP
#define HYP__HYPERGRAPH_HYPERGRAPHWRITER_HPP
#pragma once

#include <sdl/Hypergraph/HypergraphPrint.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/Empty.hpp>
#include <sdl/Util/Print.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Hypergraph/Weight.hpp>

namespace sdl {
namespace Hypergraph {

template <class A>
inline std::ostream& writeState(std::ostream& out, StateId sid, const IHypergraph<A>& hg) {
  out << sid;
  IVocabularyPtr pVoc = hg.getVocabulary();
  Sym inId = hg.inputLabel(sid);
  Sym outId = hg.outputLabel(sid);
  if (inId || outId) {
    out << '(';
    if (inId) {
      writeLabel(out, inId, pVoc);
      if (outId && outId != inId) writeLabel(out << ' ', outId, pVoc);
    } else
      writeLabel(out << ' ', outId, pVoc);
    // note: could be that we didn't have any input label, but we do have an
    // output label. then we'll see ( out-label) - the space is needed to
    // distinguish this from (in-label)!
    out << ')';
  }
  return out;
}

template <class A>
void print(std::ostream& o, StateId s, IHypergraph<A> const& hg) {
  writeState(o, s, hg);
}

#if !SDL_64BIT_STATE_ID
template <class A>
void print(std::ostream& o, std::size_t s, IHypergraph<A> const& hg) {
  writeState(o, (StateId)s, hg);
}
#endif

/**
   Writes an arc; use this you have the hg (for its vocabulary); otherwise
   operator<< will do.
*/
template <class A>
std::ostream& writeArc(std::ostream& out, const A& arc, const IHypergraph<A>& hg) {
  writeState(out, arc.head(), hg) << " <- ";
  StateIdContainer const& tails = arc.tails();
  for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i)
    writeState(out, *i, hg) << " ";
  out << "/ " << arc.weight();
  return out;
}

template <class A>
void print(std::ostream& o, A const& a, IHypergraph<A> const& hg) {
  writeArc(o, a, hg);
}

template <class Arc>
void print(std::ostream& o, ArcHandle ap, IHypergraph<Arc> const& hg) {
  writeArc(o, *(Arc const*)ap, hg);
}

template <class A>
void print(std::ostream& o, A* ap, IHypergraph<A> const& hg) {
  writeArc(o, *ap, hg);
}

/**
   Write a hypergraph in text format.
*/
template <class A>
struct HypergraphTextWriter {

  typedef A Arc;
  typedef typename Arc::Weight Weight;

  HypergraphTextWriter() {}

  virtual ~HypergraphTextWriter() {}

  virtual void write(std::ostream& out, IHypergraph<Arc> const& hypergraph) const {
    StateId startState = hypergraph.start();
    if (startState != Hypergraph::kNoState) {
      writeStateText(out, hypergraph, startState);
    }
    for (StateId i = 0, N = hypergraph.size(); i < N; ++i)
      if (i != startState) writeStateText(out, hypergraph, i);
  }

  /**
     May write in or out arcs (depending what is stored per
     state in the hypergraph)
  */
  virtual void writeStateText(std::ostream& out, IHypergraph<Arc> const& hypergraph, StateId sid) const = 0;
};

/**
   Writes a hypergraph in text format, bottom-up (can be used
   if the states store out arcs).
*/
template <class A>
class HypergraphTextWriter_Bottomup : public HypergraphTextWriter<A> {

 public:
  typedef typename A::Weight Weight;

  HypergraphTextWriter_Bottomup(IVocabularyPtr pVoc) : pVoc_(pVoc) {}

  void writeStateText(std::ostream& out, IHypergraph<A> const& hypergraph, StateId sid) const {
    if (hypergraph.final() == sid) {
      out << "FINAL <- ";
      writeState(out, sid, hypergraph) << '\n';
    }
    if (hypergraph.start() == sid) {
      out << "START <- ";
      writeState(out, sid, hypergraph) << '\n';
    }
    for (ArcId aid = 0, N = hypergraph.numOutArcs(sid); aid < N; ++aid) {
      A* arc = hypergraph.outArc(sid, aid);
      if (arcsWritten_.find(arc) == arcsWritten_.end()) {
        writeArc(out, *arc, hypergraph) << '\n';
        arcsWritten_.insert(arc);
      }
    }
  }

 private:
  IVocabularyPtr pVoc_;
  mutable std::set<A*> arcsWritten_;
};

/**
   Writes a hypergraph in text format, top-down (can be used
   if the states store in arcs).
*/
template <class A>
class HypergraphTextWriter_Topdown : public HypergraphTextWriter<A> {

 public:
  typedef typename A::Weight Weight;

  HypergraphTextWriter_Topdown(IVocabularyPtr pVoc) : pVoc_(pVoc) {}

  void writeStateText(std::ostream& out, IHypergraph<A> const& hypergraph, StateId sid) const {
    if (hypergraph.final() == sid) {
      out << "FINAL <- ";
      writeState(out, sid, hypergraph) << '\n';
    }
    if (hypergraph.start() == sid) {
      out << "START <- ";
      writeState(out, sid, hypergraph) << '\n';
    }
    for (ArcId aid = 0, N = hypergraph.numInArcs(sid); aid < N; ++aid) {
      // out << *hypergraph.inArc(sid, aid) << '\n';
      writeArc(out, *hypergraph.inArc(sid, aid), hypergraph) << '\n';
    }
  }

 private:
  IVocabularyPtr pVoc_;
};

template <class A>
std::ostream& writeHypergraph(std::ostream& out, const IHypergraph<A>& hypergraph, bool fullEmptyCheck = false) {
  bool e = fullEmptyCheck ? empty(hypergraph) : hypergraph.prunedEmpty();
  if (!e) {
    IVocabularyPtr pVoc = hypergraph.getVocabulary();
    shared_ptr<HypergraphTextWriter<A> > w;
    if (hypergraph.storesInArcs())
      w.reset(new HypergraphTextWriter_Topdown<A>(pVoc));
    else if (hypergraph.storesOutArcs())
      w.reset(new HypergraphTextWriter_Bottomup<A>(pVoc));
    else
      SDL_THROW_LOG(Hypergraph, InvalidInputException, "Hypergraph does not store arcs, cannot write");
    w->write(out, hypergraph);
  }
  return out;
}

template <class Arc>
inline std::ostream& operator<<(std::ostream& out, IHypergraph<Arc> const& hypergraph) {
  writeHypergraph(out, hypergraph);
  return out;
}

/// for gdb
void dumpVHG(IHypergraph<ArcTpl<ViterbiWeight> > const& hg);
void dumpVHG(IHypergraph<ArcTpl<FeatureWeight> > const& hg);

template <class Arc>
struct WithProperties {
  IHypergraph<Arc> const& hg;
  WithProperties(IHypergraph<Arc> const& hg) : hg(hg) {}
  friend inline std::ostream& operator<<(std::ostream& out, WithProperties const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const { out << PrintProperties(hg.properties()) << "\n" << hg; }
};

template <class Arc>
WithProperties<Arc> withProperties(IHypergraph<Arc> const& hg) {
  return WithProperties<Arc>(hg);
}


}}

#endif
