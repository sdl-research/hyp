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

    like Union, except start/final of cfg 2 into states a/b (user choice) of cfg
    1.

    for two fsm, if you first ensure no incoming arcs for either start state and no
    outgoing arcs of fsm end states, then you can implement a more efficient fsm
    union by choosing a = 1.start and b = 1.final.

    in fact you can also choose any two states from cfg 2, but you'll end up
    copying all the states/arcs, some of which will be dangling. presumably if you
    use start/final you've pruned arcs/states that aren't useful in any derivation.

*/

#ifndef SPLICE_LW2012420_HPP
#define SPLICE_LW2012420_HPP
#pragma once


#include <sdl/Util/LogHelper.hpp>
#include <sdl/Hypergraph/Transform.hpp>
#include <sdl/Hypergraph/LexicalOrEpsilon.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Hypergraph/Adjacency.hpp>
#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/WeightUtil.hpp>
#include <sdl/Hypergraph/HypergraphCopy.hpp>

namespace sdl {
namespace Hypergraph {

/// this prefix gets added to the start of the splice. instead of using this
/// facility, you could just add a dangling target splice-start -> prefix arc
/// themselves, and pass that new state as target splice-start; however, we do
/// something clever when there's a weight but not a prefix, when source
/// start = final ...
struct SplicePrefixOptions {
  // TODO@JG: a separate prefixing transform (in place) is just as efficient and the optimization described
  // above could be implemented more generally (recognizing the structure of "start (single arc) -> rest of
  // fsm""
  std::string weight;
  std::string prefix;
  template <class Config>
  void configure(Config& c) {
    c.is("prefix/weight for recognized AutoRule tokens");
    c("prefix", &prefix)(
        "entity-FSA accepted tokens will be prefixed by this string (the open/close token tags are the "
        "same). empty=<eps>")
        .self_init();
    c("weight",
      &weight)("entering the entity-FSA as opposed to using a dictionary token has this cost (Weight).")
        .self_init();
  }
  SplicePrefixOptions() { defaults(); }
  void defaults() {
    prefix = "";
    weight = "0";
  }
};

/// parsed SplicePrefixOptions
template <class W>
struct SplicePrefix {
  typedef W Weight;
  Weight weight;
  Sym prefix;
  void parse(SplicePrefixOptions const& opt, IVocabulary& voc) {
    prefix = lexicalOrEpsilon(opt.prefix, voc);
    weight = weightOrOne<W>(opt.weight);
  }
  bool trivial() const { return weight == Weight::one() && prefix == EPSILON::ID; }
  template <class A>
  void addPrefix(IMutableHypergraph<A>& tar, StateId& start) {
    if (!trivial()) {
      StateId oldStart = start;
      start = tar.addState();
      tar.addArcFst(oldStart, start, EPSILON::ID, prefix, weight);
    }
  }
};

/// which states to splice to/from. note: defaults to start/final
struct SpliceStateOptions {
  template <class Out>
  void print(Out& o) const {
    for (int i = 0; i < 2; ++i)
      for (int j = 0; j < 2; ++j) o << hgName(i) << '-' << stateName(j) << "=" << sf[i][j] << '\n';
  }
  template <class C, class T>
  friend std::basic_ostream<C, T>& operator<<(std::basic_ostream<C, T>& o, SpliceStateOptions const& self) {
    self.print(o);
    return o;
  }

  enum {
    kSourceHg = 0,
    kTargetHg = 1,
  };
  enum { kStart = 0, kFinal = 1 };
  StateId sf[2][2];  // [source or target][start or final]
  SpliceStateOptions() { defaults(); }
  void defaults() {
    for (int i = 0; i < 2; ++i)
      for (int j = 0; j < 2; ++j) sf[i][j] = kNoState;
  }
  static std::string hgName(int i) { return i == kTargetHg ? "target" : "source"; }
  static std::string stateName(int j) { return j == kFinal ? "final" : "start"; }
  static std::string noStateString() { return sdl::lexical_cast<std::string>(kNoState); }
  static std::string stateHelp() { return " (for default (start/final): " + noStateString() + ")"; }

  static std::string caption() { return "State Ids for splice" + stateHelp(); }

  template <class Config>
  void configure(Config& c) {
    c.is("Splice (insert one hg inside part of another)");
    c(caption());
    for (int i = 0; i < 2; ++i) {
      std::string prei(hgName(i));
      prei.push_back('-');
      for (int j = 0; j < 2; ++j) {
        std::string optName(prei);
        optName += stateName(j);
        c(optName, &sf[i][j])("state id for " + optName + stateHelp());
      }
    }
  }

  static StateId getStartFinal(HypergraphBase const& hg, int sf = kStart) {
    return sf == kFinal ? hg.final() : hg.start();
  }

  void setSourceTarget(HypergraphBase const& source, HypergraphBase const& target) {
    HypergraphBase const* pst[2];
    pst[0] = &source;
    pst[1] = &target;
    for (int i = 0; i < 2; ++i)
      for (int j = 0; j < 2; ++j) {
        StateId& s = sf[i][j];
        if (s == kNoState) s = getStartFinal(*pst[i], j);
      }
  }
};

// allows specification of one or both prefix/state options
struct FullSpliceOptions : SplicePrefixOptions, SpliceStateOptions {
  // nostate=empty
  void setPrefix(SplicePrefixOptions const& prefixOpt) { SplicePrefixOptions::operator=(prefixOpt); }
  void setStates(SpliceStateOptions const& stateOpt) { SpliceStateOptions::operator=(stateOpt); }
  void defaults() {
    SplicePrefixOptions::defaults();
    SpliceStateOptions::defaults();
  }
  static std::string caption() {
    return "Splicing one FSM into the middle of another with optional weight and prefix";
  }
  template <class OD>
  void configure(OD& c) {
    SplicePrefixOptions::configure(c);
    SpliceStateOptions::configure(c);
    c.is(caption());
  }
  FullSpliceOptions() {}
  FullSpliceOptions(SplicePrefixOptions const& spliceOpt) : SplicePrefixOptions(spliceOpt) {}
  FullSpliceOptions(FullSpliceOptions const& o) : SplicePrefixOptions(o), SpliceStateOptions(o) {}
  FullSpliceOptions(SpliceStateOptions const& o) : SpliceStateOptions(o) {}
};


/// embeds the part of the source hg between SpliceStateOptions.getStartFinal(kSourceHg) start->final into
/// target hg getStartFinal(kTargetHg) both hg must have same vocab. note that start->final need not be the
/// actual hg start and final state. actually copies ALL the source states.
template <class A>
struct Splice : TransformBase<Transform::Inplace, Transform::NoProperties> {
  static char const* type() { return "Splice"; }
  // TODO: Reach pruning applied to the specified start->final states for fsm, and ->final for cfg.
  typedef A Arc;
  typedef typename A::Weight Weight;
  typedef IHypergraph<A> HG;
  typedef shared_ptr<HG const> PHG;
  PHG hgSource;  // source hg

  SpliceStateOptions spliceStates;
  SplicePrefixOptions prefixOpt;
  Splice(PHG const& hgSource, FullSpliceOptions const& opt)
      : hgSource(hgSource), spliceStates(opt), prefixOpt(opt) {}
  bool mustCopy(HG const& tar) const {
    copyIfSame(hgSource, tar);
    return false;
  }
  void inplace(IMutableHypergraph<A>& tar) const {
    IVocabulary& voc = *tar.getVocabulary();
    SplicePrefix<Weight> prefix;
    prefix.parse(prefixOpt, voc);
    IHypergraph<A> const& src = *hgSource;
    assert(&voc == src.getVocabulary().get());
    SpliceStateOptions spliceSt(spliceStates);
    spliceSt.setSourceTarget(src, tar);  // set kNoState -> start/final
    StateIdTranslation x(new StateAddMapping<A>(&tar));

    StateId* srcSf = spliceSt.sf[SpliceStateOptions::kSourceHg];
    StateId* tarSf = spliceSt.sf[SpliceStateOptions::kTargetHg];
    const int S = SpliceStateOptions::kStart, F = SpliceStateOptions::kFinal;

    assert(tarSf[S] != kNoState);  // because we already converted kNoState -> start/final
    assert(tarSf[F] != kNoState);
    assert(srcSf[S] != kNoState);
    assert(srcSf[F] != kNoState);

    SDL_DEBUG(Hypergraph.Splice, "splice states: " << spliceSt << '\n');
    SDL_DEBUG(Hypergraph.Splice, "source Hg: " << src << '\n');
    SDL_DEBUG(Hypergraph.Splice, "target Hg: " << tar << '\n');
    StateId &srcStart = srcSf[S], &srcFinal = srcSf[F];
    StateId &tarStart = tarSf[S], &tarFinal = tarSf[F];
    if (srcStart == srcFinal) {
      if (tarStart == tarFinal && prefix.trivial()) {
        // nothing
      } else if (prefix.prefix == EPSILON::ID) {
        tar.addArcFsa(tarStart, tarFinal, EPSILON::ID,
                      prefix.weight);  // since prefix is empty, we can put it on the end
      } else {
        prefix.addPrefix(tar, tarStart);  // updates tarStart
        tar.addArcFsa(tarStart, tarFinal);  // epsilon
      }
    } else {
      prefix.addPrefix(tar, tarStart);  // updates tarStart
      x.add(srcFinal, tarFinal);
    }
    x.add(srcStart, tarStart);
    SDL_DEBUG(Hypergraph.Splice, "final splice states: " << spliceSt << '\n');
    SDL_TRACE(Hypergraph.Splice, "target before splice: " << tar << '\n');
    copyArcs(x, src, &tar);
    SDL_TRACE(Hypergraph.Splice, "target after splice copyArcs: " << tar << '\n');
    x.transferLabelsPartial(src, tar);
    SDL_TRACE(Hypergraph.Splice, "target after splice transferLabels: " << tar << '\n');
  }
};

template <class A>
void splice(IHypergraph<A> const& src, IMutableHypergraph<A>* tar, FullSpliceOptions const& opt) {
  Splice<A> spliceTransform(src, opt);
  inout(*tar, spliceTransform);
}


}}

#endif
