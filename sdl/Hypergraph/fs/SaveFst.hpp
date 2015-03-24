/** \file

    save fst (or lazy nbest of it) in mutable hypergraph.
*/

#ifndef SAVEFST_JG2013122_HPP
#define SAVEFST_JG2013122_HPP
#pragma once

#include <sdl/Hypergraph/fs/LazyBest.hpp>
#include <sdl/Hypergraph/fs/Fst.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Hypergraph/Project.hpp>
#include <sdl/Hypergraph/PruneNonBest.hpp>

namespace sdl {
namespace Hypergraph {
namespace fs {

struct SaveFstOptions : LazyBestOptions, PruneToNbestOptions {
  std::size_t reserveStates;
  bool forceOutArcs;

  SaveFstOptions() : PruneToNbestOptions(0), reserveStates(1000000), forceOutArcs(true) {}
  template <class Config>
  void configure(Config& config) {
    LazyBestOptions::configure(config);
    PruneToNbestOptions::configure(config);
    config("reserve-states", &reserveStates)(
        "expect about this many result states (too high wastes memory; too low may be ~10% slower from "
        "copying").self_init();
    config("force-out-arcs", &forceOutArcs)("make result store (first-tail-only) fst out arcs").self_init();
  }
  friend inline void validate(SaveFstOptions& x) { x.validate(); }
};

/**
   store a lazy fst in an IMutableHypergraph<Arc>.
*/
template <class Fst>
struct SaveFst {
  typedef typename Fst::Weight Weight;
  typedef typename Fst::State State;
  typedef typename Fst::Arc FstArc;
  typedef typename Fst::Arcs FstArcs;
  typedef unordered_map<State, StateId> StateMap;
  typedef ArcTpl<Weight> HgArc;
  typedef IMutableHypergraph<HgArc> Hg;
  Hg& out;
  Fst& fst;
  StateMap stateMap;
  boost::optional<StateId> outFinal;
  bool projectOutput;
  bool annotations_;
  SaveFst(Fst& fst, Hg& out, SaveFstOptions const& opt)
      : fst(fst), out(out), projectOutput(opt.projectOutput), annotations_(opt.annotations) {
    out.setVocabulary(fst.getVocabulary());
    if (opt.forceOutArcs) out.forceFirstTailOutArcs();
    out.setEmpty();
    Util::reserveUnorderedImpl(stateMap, opt.reserveStates);
    out.setFinal(kNoState);
    out.setStart(outStateFor(fst.startState()));
  }
  void setFinal(StateId s) {
    outFinal = s;
    out.setFinal(s);
  }

  // TODO: could detect emptiness as we save (then clear result if empty)
  StateId outStateFor(State const& state) {
    StateId* outId;
    if (Util::update(stateMap, state, outId)) {
      StateId from = out.addState();
      *outId = from;  // note: we set this before recursing on out arcs of new state (because loops are
      // allowed)
      FstArcs genArcs((fst.outArcs(state)));
      unsigned nOut = 0;
      while (genArcs) {
        FstArc fstArc(genArcs.get());
        genArcs.got();
        if (projectOutput) setInputAsOutput(fstArc.labelPair);
        HgArc* added = addAnnotatedArc(out, outStateFor(fstArc.dst), from, fstArc.labelPair, fstArc.weight,
                                     &fstArc.annotations, annotations_);
        if (!fstArc.annotations.empty())
          SDL_DEBUG(Hypergraph.fs.SaveFst, "added annotated arc " << Util::print(fstArc, *out.getVocabulary())
                                                                  << " as "
                                                                  << Util::print(*added, out));
        ++nOut;
      }
      bool final = fst.final(state);
      if (final) {  // a final state
        if (!outFinal) {
          bool const needsEpsilon = nOut;
          StateId const finalSt = needsEpsilon ? out.addState() : from;
          setFinal(finalSt);
          if (!needsEpsilon) return from;
        }
        out.addArcEpsilon(from, *outFinal);
      }
      return from;
    } else {
      return *outId;
    }
  }
};

/**
   save whole fst.
*/
template <class Fst>
void saveFstComplete(Fst& fst, IMutableHypergraph<ArcTpl<typename Fst::Weight> >& outHg,
                     SaveFstOptions const& opt) {
  SaveFst<Fst> save(fst, outHg, opt);
  if (opt.projectOutput) outHg.projectOutput();
}

/**
   save fst after wrapping with LazyBest if requested in options.
*/
template <class Fst>
void saveFst(Fst& fst, IMutableHypergraph<ArcTpl<typename Fst::Weight> >& outHg, SaveFstOptions const& opt) {
  if (opt.pruneToNbest == 1)
    lazyBestToHg(fst, outHg, opt);
  else
    saveFstComplete(fst, outHg, opt);
}


}}}

#endif
