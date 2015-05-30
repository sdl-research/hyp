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

    print a hypergraph.
*/

#ifndef HYP__HYPERGRAPH_HYPERGRAPHWRITER_HPP
#define HYP__HYPERGRAPH_HYPERGRAPHWRITER_HPP
#pragma once

#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/Empty.hpp>
#include <sdl/Util/Print.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/FeatureWeight.hpp>

namespace sdl {
namespace Hypergraph {

inline void printState(std::ostream& out, StateId s, IHypergraphStates const* hg = 0) {
  if (hg)
    printState(out, s, *hg);
  else
    out << s;
}

inline void print(std::ostream& o, StateId s, IHypergraphStates const& hg) {
  printState(o, s, hg);
}

inline void print(std::ostream& o, StateId s, IHypergraphStates const* hg) {
  printState(o, s, hg);
}

#if _MSC_VER

template <class A>
void print(std::ostream& o, A const& a, IHypergraph<A> const& hg) {
  printArc(o, &a, hg);
}
template <class A>
void print(std::ostream& o, A* ap, IHypergraph<A> const& hg) {
  printArc(o, ap, hg);
}
template <class A>
void print(std::ostream& o, A const& a, IHypergraph<A> const* hg) {
  printArc(o, &a, hg);
}
template <class A>
void print(std::ostream& o, A* ap, IHypergraph<A> const* hg) {
  printArc(o, ap, hg);
}

#else

#if !SDL_64BIT_STATE_ID
template <class A>
void print(std::ostream& o, std::size_t s, IHypergraphStates const& hg) {
  printState(o, (StateId)s, hg);
}
#endif

template <class A>
void print(std::ostream& o, A const& a, IHypergraphStates const& hg) {
  printArc(o, &a, hg);
}
template <class A>
void print(std::ostream& o, A* ap, IHypergraphStates const& hg) {
  printArc(o, ap, hg);
}
template <class A>
void print(std::ostream& o, A const& a, IHypergraphStates const* hg) {
  printArc(o, &a, hg);
}
template <class A>
void print(std::ostream& o, A* ap, IHypergraphStates const* hg) {
  printArc(o, ap, hg);
}
#endif

/**
   Write a hypergraph in text format.
*/
template <class A>
struct HypergraphTextWriter {

  typedef A Arc;
  typedef typename Arc::Weight Weight;

  HypergraphTextWriter() {}

  virtual ~HypergraphTextWriter() {}

  virtual void write(std::ostream& out, IHypergraph<Arc> const& hg) const {
    StateId startState = hg.start();
    if (startState != Hypergraph::kNoState) {
      printStateText(out, hg, startState);
    }
    for (StateId i = 0, N = hg.size(); i < N; ++i)
      if (i != startState) printStateText(out, hg, i);
  }

  /**
     May write in or out arcs (depending what is stored per
     state in the hypergraph)
  */
  virtual void printStateText(std::ostream& out, IHypergraph<Arc> const& hg, StateId s) const = 0;
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

  void printStateText(std::ostream& out, IHypergraph<A> const& hg, StateId s) const {
    if (hg.final() == s) {
      out << "FINAL <- ";
      printState(out, s, hg);
      out << '\n';
    }
    if (hg.start() == s) {
      out << "START <- ";
      printState(out, s, hg);
      out << '\n';
    }
    for (ArcId aid = 0, N = hg.numOutArcs(s); aid < N; ++aid) {
      A* arc = hg.outArc(s, aid);
      if (arcsWritten_.find(arc) == arcsWritten_.end()) {
        printArc(out, arc, hg);
        out << '\n';
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

  void printStateText(std::ostream& out, IHypergraph<A> const& hg, StateId s) const {
    if (hg.final() == s) {
      out << "FINAL <- ";
      printState(out, s, hg);
      out << '\n';
    }
    if (hg.start() == s) {
      out << "START <- ";
      printState(out, s, hg);
      out << '\n';
    }
    for (ArcId aid = 0, N = hg.numInArcs(s); aid < N; ++aid) {
      printArc(out, hg.inArc(s, aid), hg);
      out << '\n';
    }
  }

 private:
  IVocabularyPtr pVoc_;
};

template <class A>
std::ostream& writeHypergraph(std::ostream& out, IHypergraph<A> const& hg, bool fullEmptyCheck = false) {
  bool e = fullEmptyCheck ? empty(hg) : hg.prunedEmpty();
  SDL_TRACE(Hypergraph.writeHypergraph, "writing hg props=" << PrintProperties(hg.properties()));
  if (!e) {
    IVocabularyPtr pVoc = hg.getVocabulary();
    shared_ptr<HypergraphTextWriter<A> > w;
    if (hg.storesInArcs())
      w.reset(new HypergraphTextWriter_Topdown<A>(pVoc));
    else if (hg.storesOutArcs())
      w.reset(new HypergraphTextWriter_Bottomup<A>(pVoc));
    else
      SDL_THROW_LOG(Hypergraph, InvalidInputException, "Hypergraph does not store arcs, cannot write");
    w->write(out, hg);
  }
  return out;
}

template <class Arc>
inline std::ostream& operator<<(std::ostream& out, IHypergraph<Arc> const& hg) {
  writeHypergraph(out, hg);
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
  void print(std::ostream& out) const { out << PrintProperties(hg.properties()) << '\n' << hg; }
};

template <class Arc>
WithProperties<Arc> withProperties(IHypergraph<Arc> const& hg) {
  return WithProperties<Arc>(hg);
}


}}

#endif
