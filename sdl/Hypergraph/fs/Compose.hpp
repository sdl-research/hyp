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

    ComposeFst<InputFst, MatchFst, Filter, WeightProductFn>

    lazy composition of finite state machines (automata or transducer) M1 * M2

    M1 is an "Input" fst - it must return a lazy (best-first if you can) generator of
    out arcs for a given state. it may have as output labels <eps> or terminal
    labels only.

    M2 is a "Match" fst - it provides generators of out arcs from a state matching a
    given input symbol (including special symbols <eps> <phi> <sigma> <rho>)

    notation:

    states q1 from M1 and q2 from M2

    arc input labels i1 and i2; output labels o1 and o2 for the arcs i1:o1 from M1 and i2:o2 from M2

    the composition is modified by a Filter type which affects the order in which <eps> i2 and o1 are taken,
    so as not to introduce multiple derivations that are essentially the same

    the derivations in the composition take input yields I1 and output yields O2
    from paths in M1 and 2 resp. that have matching inner yields O1==I2 (after
    removing <eps>). the weight of the composed derivation is the product of the
    derivations from M1 and M2.

    this is achieved by having states (q1, filter, q2) in the composition, final
    iff q1 and q2 are both final, and starting in
    (start1, Filter::normal, start2). the outgoing arcs in the composition are
    built from the outgoing arcs of q1 and q2 (described below).

    note that the weight type for the Match fst may be different from that of
    the Lazy M1 and the Lazy composition; a function object serves to define
    the product (e.g. a viterbi hypergraph weight can be multiplied into a
    FeatureWeight by giving it a feature id and weight)

    Lazy composition should be faster and may have a smaller result than the CFG
     FST intersection markus implemented in ../Compose.hpp


    the generation of outgoing arcs in the composition from state (q1, filter, q2) works as follows:

    (see
   http://static.googleusercontent.com/external_content/untrusted_dlcp/research.google.com/en/us/pubs/archive/37568.pdf)

    for M1 arc i1:o1 -> state r1 (from state q1):
    for M2 arc i2:o2 -> state r2 (from state q2):

    the filter state filter' depends on the type of transition taken and the type of filter (described later)

    the weight of the arc is the product of the M1 and M2 arc (with
    Weight::one() serving in case only one arc is mentioned)

    DEFINITION: standard match:

    if o1==i2 are non-special terminal symbols, we get (in composition) i1:o2 -> (r1, Filter::normal, r2)

    now, for all the special (not standard) matches:

    if o1==<eps> and Filter::allowInputEpsilon, we get i1:<eps> -> (r1, filter', q2) (the M2 state is
   unchanged)

    if i2==<eps> and Filter::allowMatchEpsilon, we get <eps>:o2 -> (q1, filter', r2)

    if i2==o1==<eps> and Filter::allowInputMatchEpsilon, we get i1:o2 -> (r1, filter', r2)

    if i2==<sigma> and o1 is a terminal symbol, we get i1:(o1 if o2==<sigma> else o2) -> (r1, Filter::normal,
   r2)

    if i2==<rho> and o1 is a terminal symbol AND no standard match occurred for
    the arc i1:o1, we get i1:(o1 if o2==<rho> else o2) -> r1, Filter::normal, r2)
    (similar to, and inaddition to, any <sigma> case above)

    finally, if i2==<phi> and Filter::allowMatchEpsilon and no
    standard matches occured for the entire compose state (for *every* input arc), <eps>:o2 -> (q1, filter',
   r2)


    note: in understanding the code below, recall that M1 is an InputFst and M2
    is a MatchFst - that is, types or variable names beginning with "input" do
    not refer to the input label (which is given by labelPair.first) but rather
    to a state, arc, or label of M1.

    TODO: allow lazy fst composition for one-lexical-symbol-per-arc (ignoring
    but preserving any non-lexical labels before the lexical tail)
*/

#ifndef COMPOSE_JG20121225_HPP
#define COMPOSE_JG20121225_HPP
#pragma once

#include <sdl/Hypergraph/fs/Fst.hpp>
#include <sdl/Hypergraph/fs/LazyBest.hpp>
#include <sdl/Hypergraph/WeightsFwdDecls.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Hypergraph/Exception.hpp>
#include <sdl/Hypergraph/WeightUtil.hpp>
#include <sdl/Hypergraph/SortArcs.hpp>
#include <sdl/Util/RefCount.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Valgrind.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/Util/FlattenGenerators.hpp>
#include <sdl/Util/SharedGenerator.hpp>
#include <sdl/Hypergraph/MixFeature.hpp>
#include <boost/functional/hash.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/mpl/and.hpp>
#include <boost/utility/enable_if.hpp>
#include <algorithm>
#include <sdl/Hypergraph/fs/SaveFst.hpp>


namespace sdl {
namespace Hypergraph {
namespace fs {


struct FstComposeOptions : SaveFstOptions {
  FstComposeOptions()
      : fstCompose(true)
      , allowDuplicatePaths(false)
      , sortBestFirst(false)
      , epsilonMatchingFilter(true)
      , allowDuplicatePathsIf1Best(false) {}

  template <class Arc>
  bool willLazyFsCompose(Hypergraph::IHypergraph<Arc> const& hg) const {
    return fstCompose && hg.isFsm();
  }

  template <class Config>
  void configure(Config& config) {
    SaveFstOptions::configure(config);
    config("fst-compose", &fstCompose)
        .self_init()("true: optimized fst1*fst2 compose (false: cfg*fst, which is slower)");
    config("sort-best-first", &sortBestFirst).self_init()(
        "for fst-compose, attempt to sort out-arcs best-first (ignored for non-mutable hypergraphs)");
    config("epsilon-matching-paths", &epsilonMatchingFilter).self_init()(
        "use a three-valued filter state that prefers matching input x:<eps> match <eps>:y directly into "
        "x:y. this result may be larger or smaller. (TODO: seems buggy - ate some paths in testing)");
    config("allow-duplicate-paths", &allowDuplicatePaths).self_init()(
        "for viterbi-like (idempotent plus - i.e. plus(x, x) = x) semirings, allow repeated equivalent "
        "paths involving epsilons. this will result in n-best duplicates, but otherwise may be faster");
    config("allow-duplicate-paths-if-1best", &allowDuplicatePathsIf1Best).self_init()(
        "allow-duplicate-paths only if prune-to-nbest=1 (because the downside of allow-duplicate-paths is "
        "extra paths)");
    config("mix-fst", &mix)(
        "a MixFeature for scaling the fst1 arc weight into the fst2 arc weight (and assigning feature id if "
        "fst1 is FeatureWeight). if fst1 and fst2 are both FHG, then you have fst1*(fst^scale) - the "
        "features of both");
  }

  MixFeature<> mix;
  bool fstCompose;
  bool allowDuplicatePaths, allowDuplicatePathsIf1Best;
  bool sortBestFirst;
  bool epsilonMatchingFilter;
  bool allowDuplicatesEffective() const {
    return allowDuplicatePaths || allowDuplicatePathsIf1Best && pruneToNbest == 1;
  }
};

using Vocabulary::WhichFstComposeSpecials;

/**
   \return symbol doesn't consume input.
*/
inline bool isNonconsuming(Sym matchLabel) {
  return matchLabel == EPSILON::ID || matchLabel == PHI::ID;
}

/**
   symbol always matches.
*/
inline bool isWildcard(Sym matchLabel) {
  return matchLabel == EPSILON::ID || matchLabel == SIGMA::ID;
}

/**
   \return symbol matches only when no explicitly labeled (non-epsilon) arc matches
*/
inline bool isFallback(Sym matchLabel) {
  return matchLabel == PHI::ID || matchLabel == RHO::ID;
}


struct FilterBase {
  void matchEpsilon() {}
  void inputEpsilon() {}
  void normal() {}
  void inputMatchEpsilon() {}
};

/**
   a filter is a modification to the simple cross product of states (q1, q2) for
   M1*M2 - instead we have (q1, f, q2), where the filter state can prevent
   ambiguous paths involving epsilon, or even combine an M1 arc i1:<eps> with an
   M2 arc <eps>:o2 to produce a result i1:o1 directly. NoEpsilonFilter is a dummy
   filter (so you'll get ambiguous paths)

   this filter should give smaller outputs but at the cost of redundant paths
   sequencing consecutive input and match transducer epsilon arcs freely.

   (see
   http://static.googleusercontent.com/external_content/untrusted_dlcp/research.google.com/en/us/pubs/archive/37568.pdf)

   we treat nonconsuming 'else' (<phi>) as epsilon.
*/
struct NoEpsilonFilter : FilterBase {
  bool allowInputEpsilon() const { return true; }
  bool allowMatchEpsilon() const { return true; }
  bool allowInputMatchEpsilon() const { return false; }
  void hashCombine(std::size_t& h) const {}
  bool operator==(NoEpsilonFilter const& o) const { return true; }
  void setStart() const {}
  friend inline std::ostream& operator<<(std::ostream& out, NoEpsilonFilter const& self) {
    return out << '.';
  }
};

/**
   an epsilon sequencing filter that forces taking all the possible epsilons in A first before taking any from
   B

   (see
   http://static.googleusercontent.com/external_content/untrusted_dlcp/research.google.com/en/us/pubs/archive/37568.pdf)

   0 if (o[e1], i[e2]) = (x, x) with x terminal,
   0 if (o[e1], i[e2]) = (<eps>, ) and q3 = 0,
   1 if (o[e1], i[e2]) = (, <eps>),

*/
struct Epsilon1First : FilterBase {
  char s;
  void normal() { s = 0; }
  bool allowInputEpsilon() const { return true; }
  void inputEpsilon() { s = 1; }
  bool allowMatchEpsilon() const { return s == 0; }
  void matchEpsilon() {}  // s = 0
  bool allowInputMatchEpsilon() const { return false; }
  void hashCombine(std::size_t& h) const { h ^= ((std::size_t) - 1) * s; }
  bool operator==(Epsilon1First const& o) const { return s == o.s; }
  void setStart() { s = false; }
  friend inline std::ostream& operator<<(std::ostream& out, Epsilon1First const& self) {
    out << (self.s ? '^' : '_');
    return out;
  }
};

/**
   epsilon-matching filter

   (see
   http://static.googleusercontent.com/external_content/untrusted_dlcp/research.google.com/en/us/pubs/archive/37568.pdf)

   filter like Epsilon1First but preferring to take x:<eps> <eps>:y as a single
   arc x:y. this may be slower or faster than Epsilon1First depending on
   structure - generally, it's possible to save on arcs by introducing states;
   this method benefits from that sharing less. on the other hand, if
   unbalanced/unbounded input/output epsilons aren't common, this could produce
   a smaller output.

   q3' is:

   0 if (o[e1], i[e2]) = (x, x) with x terminal,
   0 if (o[e1], i[e2]) = (<eps>, <eps>) and q3 = 0, 'InputMatchEpsilon'
   1 if (o[e1], i[e2]) = (, <eps>) and q3 != 2, 'MatchEpsilon'
   2 if (o[e1], i[e2]) = (<eps>, ) and q3 != 1, 'InputEpsilon'

   (0,1,2 are all final)

*/
struct EpsilonCombine : Epsilon1First {
  bool allowInputEpsilon() const { return s != 1; }
  void inputEpsilon() { s = 2; }
  bool allowMatchEpsilon() const { return s != 2; }
  void matchEpsilon() { s = 1; }
  bool allowInputMatchEpsilon() const { return s == 0; }
  void inputMatchEpsilon() {
    assert(s == 0);  // because we only go from 0 to 0 for input+match epsilon.
  }
  void hashCombine(std::size_t& h) const { h ^= (h >> (s + 1)) + s; }
  friend inline std::ostream& operator<<(std::ostream& out, EpsilonCombine const& self) {
    out << (self.s == 2 ? '#' : self.s ? '^' : '_');
    return out;
  }
};


/**
   state in composition of input * match.
*/
template <class InputStateId, class Filter = NoEpsilonFilter, class MatchStateId = StateId>
struct Compose2State : public Filter  // inherit rather than member because e.g. NoEpsilonFilter has no state
                       {
  typedef InputStateId InputState;
  typedef MatchStateId MatchState;
  InputState input;
  MatchState match;
  friend inline std::ostream& operator<<(std::ostream& out, Compose2State const& self) {
    out << self.input << (Filter const&)self << self.match;
    return out;
  }

  Filter& filter() const { return *(Filter const*)this; }
  Filter& filter() { return *(Filter*)this; }
  void setFilterStart() { Filter::setStart(); }

  typedef Compose2State self_type;
  friend inline std::size_t hash_value(self_type const& x) {
    std::size_t h = boost::hash<MatchState>()(x.match);
    boost::hash_combine(h, x.input);
    x.hashCombine(h);
    return h;
  }
  bool operator==(Compose2State const& other) const {
    return input == other.input && Filter::operator==(other) && match == other.match;
  }
};

/**
   TimesByFn is for computing result weights.
*/

template <class ArcT>
struct HypergraphMatchFst : HypergraphFst<ArcT> {
  typedef IMutableHypergraph<ArcT> Hg;
  typedef HypergraphFst<ArcT> Base;
  typedef IHypergraph<ArcT> ConstHg;
  typedef typename ArcT::Weight Weight;
  typedef typename Weight::FloatT Distance;
  typedef StateId State;

  /**
     if we know levels of input and match (both are acyclic), then we can do
     perfect beamed search. of course, we might want to use per-input-level
     beams only (which is fine
  */
  Level combinedLevel(Level inputLevel, State st) const {
    return this->nLevels == 1 ? inputLevel : (inputLevel * this->nLevels) + this->level(st);
  }

  Hg const& hg() const { return static_cast<Hg const&>(*this->pHg); }
  /**
     must have kSortedOutArcs already.
  */
  HypergraphMatchFst(shared_ptr<Hg const> const& hg,
                     WhichFstComposeSpecials which = WhichFstComposeSpecials::undefined()) {
    init(hg, which);
  }

  /**
     sorts for you if needed.
  */
  HypergraphMatchFst(shared_ptr<Hg> const& hg,
                     WhichFstComposeSpecials which = WhichFstComposeSpecials::undefined()) {
    init(hg, which);
  }
  HypergraphMatchFst(Hg const& hg, WhichFstComposeSpecials which = WhichFstComposeSpecials::undefined()) {
    init(hg, which);
  }

  /**
     arcs matching input. must call explicitly for epsilon, rho, etc.
  */
  typedef typename Base::Arcs Matches;
  Matches arcsMatchingInput(StateId sid, Sym in) const {
    return Matches(hg().outArcsMatchingInput(sid, in), this->arcFn);
  }

  WhichFstComposeSpecials whichSpecials;

 protected:
  template <class Hg>
  void initSpecials(Hg const& hg, WhichFstComposeSpecials which) {
    whichSpecials = which.defined() ? which : hg.whichInputFstComposeSpecials();
  }
  void init(shared_ptr<Hg> const& pHg, WhichFstComposeSpecials which) {
    pHg->forceProperties(kSortedOutArcs);
    initSpecials(*pHg, which);
    Base::init(boost::static_pointer_cast<Hg const>(pHg));
  }
  void init(shared_ptr<ConstHg const> const& pHg, WhichFstComposeSpecials which) {
    if (!(pHg->properties() & kSortedOutArcs))
      SDL_THROW_LOG(fs.Compose, ConfigException, "match hypergraph must have sorted out arcs");
    initSpecials(*pHg, which);
    this->needMutable(*pHg);
    Base::init(pHg);
  }
  void init(ConstHg const& hg, WhichFstComposeSpecials which) { init(ptrNoDelete(hg), which); }
  void init(Hg& hg, WhichFstComposeSpecials which) { init(ptrNoDelete(hg), which); }
};

/**
   for composing with a MatchFst that has a single score that we wish to mix into an input feature weight,
   using MixFeature. to join FeatureWeight MatchFst, see MixWeights instead.
*/
template <class ResultWeight, class InWeight, class Enable = void>
struct TimesByMix {
  typedef ResultWeight result_type;
  MixFeature<FeatureId, typename ResultWeight::FloatT> mix;

  void timesBy(InWeight const& weight, ResultWeight& result) const { mix(weight.getValue(), result); }

  template <class Weight>
  result_type operator()(ResultWeight const& inputWt, Weight const& matchWt) const {
    result_type r((inputWt));
    timesBy(matchWt, r);
    return r;
  }
};

/**
   this version for featureweight*featureweight.
*/

template <class FeatureWeight>
struct TimesByMix<FeatureWeight, FeatureWeight, typename FeatureWeight::IsFeatureWeight> {
  typedef FeatureWeight result_type;
  MixFeature<FeatureId, typename FeatureWeight::FloatT> mix;

  void timesBy(FeatureWeight const& weight, FeatureWeight& result) const {
    result.timesByScaled(weight, mix.scale);
  }

  result_type operator()(FeatureWeight const& inputWt, FeatureWeight const& matchWt) const {
    result_type r((inputWt));
    timesBy(matchWt, r);
    return r;
  }
};

/**
   InputFst = e.g. HypergraphFst<Arc> or ComposeFst<...>, MatchFst = HypergraphMatchFst<Arc>.

   or InputFst = ComposeFst<...>
*/
template <class InputFst, class MatchFst, class Filter = Epsilon1First,
          class TimesFn = TimesByMix<typename InputFst::Weight, typename MatchFst::Weight> >
struct ComposeFst : TimesFn {
  typedef ComposeFst<InputFst, MatchFst, Filter, TimesFn> Compose;
  typedef TimesFn Times;

  typedef InputFst Input;
  typedef MatchFst Match;


  template <class Init>
  void initInput(Init& seed) {
    if (!input) input.reset(new Input(seed));
  }

  template <class Init>
  void initMatch(Init& seed) {
    if (!match) match.reset(new Match(seed));
  }

  template <class InitInput, class InitMatch>
  void init(InitInput const& initForInput, InitMatch const& initForMatch) {
    initInput(initForInput);
    initMatch(initForMatch);
  }

  IVocabularyPtr getVocabulary() const { return input->getVocabulary(); }

  typedef shared_ptr<Input> InputPtr;
  typedef shared_ptr<Match> MatchPtr;

  typedef typename Input::State InputState;
  typedef typename Match::State MatchState;
  typedef Compose2State<InputState, Filter, MatchState> State;
  typedef typename Input::Arc InputArc;
  typedef typename Match::Arc MatchArc;
  typedef typename Input::Arcs InputArcs;
  typedef typename Input::Distance Distance;
  typedef typename Match::Matches MatchArcs;


  typedef typename Input::Weight InputWeight;
  typedef typename Match::Weight MatchWeight;
  typedef typename Times::result_type Weight;

  typedef FstArc<Weight, State> Arc;

  /**
     overall scheme: generator of generators, best-first

     emit a generator for (ArcsGen) each input arc, before and after input arcs

     note: some of the generators may be empty; this is handled correctly by FlattenGenerators

  */
  struct ArcsGen : Util::GeneratorBase<ArcsGen, Arc, Util::NonPeekableT>, Filter, Times {
    typedef Arc result_type;
    Filter& filter() {  // empty base (NoEpsilonFilter) optimization
      return *(Filter*)this;
    }

    //    enum { kPreSigma, kMidSigma, kDoneSigma };
    /**
       exact determination in advance of whether there are more arcs to come. (we could use a null arc to
       simplify)
    */
    operator bool() const { return hasMore; }
    void setDone() { hasMore = false; }
    Arc operator()() {
      Arc r;  // could make peekable by r as member, init by pop() once when creating. could save us from
      // detecting empty range in advance
      LabelPair const& inLabels = inArc.labelPair;
      r.dst.input = inArc.dst;
      IF_SDL_HYPERGRAPH_FS_ANNOTATIONS(r.annotations = inArc.annotations;)
      r.dst.filter() = *(Filter const*)this;
      LabelPair& rLabels = r.labelPair;
      rLabels.first = inLabels.first;
      assert(hasMore);
      if (inLabels.second == EPSILON::ID) {  // two primary cases: input arc has epsilon output, or not.
        if (matchedArcs) {  // inputMatchEpsilon
          MatchArc const& matchArc = matchedArcs();
          assert(matchArc.labelPair.first == EPSILON::ID);
          rLabels.second = matchArc.labelPair.second;
          r.dst.match = matchArc.dst;
          r.weight = Times::operator()(inArc.weight, matchArc.weight);
          assert(Filter::allowInputEpsilon());  // because allowMatchEpsilon implies allowInputEpsilon
          // after done w/ matchedArcs, take regular input epsilon by 'else' below:
        } else {
          // then take just input epsilon
          assert(Filter::allowInputEpsilon());
          rLabels.second = EPSILON::ID;
          r.dst.filter().inputEpsilon();
          r.weight = inArc.weight;
          r.dst.match = matchSrc;
          hasMore = false;
        }
      } else if (midSigma) {
        assert(matchedArcs);
        MatchArc const& matchArc = matchedArcs();
        LabelPair const& matchLabels = matchArc.labelPair;
        assert(matchLabels.first == SIGMA::ID);  // must be sigma (wildcard)
        Sym m2 = matchLabels.second;
        r.dst.match = matchArc.dst;
        r.weight = Times::operator()(inArc.weight, matchArc.weight);
        rLabels.second = (m2 == SIGMA::ID ? inLabels.second : m2);
        hasMore = matchedArcs;
      } else {
        assert(matchedArcs);
        // first matched, then sigma (if matched only, we have no input arc)
        MatchArc const& matchArc = matchedArcs();
        LabelPair const& matchLabels = matchArc.labelPair;
        Sym m1 = matchLabels.first, m2 = matchLabels.second;
        r.dst.match = matchArc.dst;
        if (m1 == EPSILON::ID || m1 == PHI::ID) {  // phi or eps; no input arc.
          assert(!inLabels.second);  // we'll have set this explicitly to avoid above EPSILON::ID branch
          r.weight = Times::operator()(Weight::one(), matchArc.weight);
          // explicit times vs one since input and match weights are different
          // types. i have the result weight as an input weight.
          rLabels.second = m2 == PHI::ID ? EPSILON::ID : m2;
          if (!matchedArcs) hasMore = false;
        } else {
          r.weight = Times::operator()(inArc.weight, matchArc.weight);
          rLabels.second = (m1 == m2 ? inLabels.second : m2);
          // remember that for rho and sigma we output the input symbol iff
          // the output of the arc is also rho or sigma respectively
          if (!matchedArcs) {
            assert(inLabels.second);  // else must have been phi or eps (nonconsuming) above
            hasMore = initSigma();
          }
        }
      }
      return r;
    }

    bool initSigma() {
      if (match->whichSpecials.test(SIGMA::id)
          && (matchedArcs = match->arcsMatchingInput(matchSrc, SIGMA::ID))) {
        midSigma = true;
        return true;
      } else
        return false;
    }

    bool initRho() {
      return match->whichSpecials.test(RHO::id) && (matchedArcs = match->arcsMatchingInput(matchSrc, RHO::ID));
    }

    ArcsGen() {}
    ArcsGen(Times const& times, Match* match, MatchState matchSrc)
        : Times(times), match(match), matchSrc(matchSrc), hasMore(true), midSigma(false) {}

    Match* match;
    MatchArcs matchedArcs;

    /// contains input epsilon and input src state if match arc is nonconsuming. otherwise it's an actually
    /// arc:
    InputArc inArc;

    MatchState matchSrc;  // used if inArc output label is epsilon, or for initSigma

    bool hasMore, midSigma;
    //, hasMoreSigma, initSigma;  // done or more
    // int sigmabytes[sizeof(MatchArcs)/sizeof(int)];

    friend inline std::ostream& operator<<(std::ostream& out, ArcsGen const& self) {
      out << "ArcsGen {" << self.inArc << " more?=" << self.hasMore;
      out << "}";
      return out;
    }
  };

  /**
     generator of ArcsGen generators, for FlattenGenerators.

     1: for all match <eps>:...
     2: for each inarc, either
     2a: a:<eps> (including possibly using match <eps>:b at same time), or
     2b: a:b b:c => a:c, else sigma:c
     3: (if no matches at all in 2b), <phi>:...
  */
  struct ArcsGenGen : Util::intrusive_refcount<ArcsGenGen, unsigned>,
                      Util::GeneratorBase<ArcsGenGen, ArcsGen, Util::NonPeekableT>,
                      Times
                      // non-atomic count because these are only held by one thread
                      {
    typedef ArcsGen result_type;
    enum ConcatState { matchEpsilon, matchRegular, matchDone };
    operator bool() const { return concatState != matchDone; }

    ArcsGen operator()() {
      // returns the next nonempty generator, or else one with status==done (empty)
      assert(concatState != matchDone);
      ArcsGen r(*this, match.get(), src.match);  // could make peekable by r as member.
      Filter& filter = r.filter();
      filter = src.filter();

      if (concatState == matchEpsilon) {
        concatState = matchRegular;
        if (epsilon) {  // match arc only. set phony self-loop input arc
          r.matchedArcs = epsilon;
          assert(filter.allowMatchEpsilon());
          filter.matchEpsilon();
          r.inArc.dst = src.input;
          r.inArc.labelPair.first = EPSILON::ID;
          r.inArc.labelPair.second
              = NoSymbol;  // intentionally not EPSILON::ID; helps distinguish from real epsilon input arcs
#if SDL_VALGRIND
          r.inArc.weight = Weight::one();
#endif
          // weight is set to one in ArcsGen
          return r;
        }
        // else continue; no eligible
      }

      while (inArcs) {
        Sym matchSym = (r.inArc = inArcs()).labelPair.second;
        if (matchSym) {
          if (matchSym == EPSILON::ID) {
            // input epsilons don't cause us to exclude a phi in the match transducer.
            if (filter.allowInputMatchEpsilon()
                && (r.matchedArcs = match->arcsMatchingInput(src.match, matchSym))) {
              filter.inputMatchEpsilon();
              assert(filter.allowInputEpsilon());
              // will also generate input epsilon (permitted by all 3 filters)
              return r;
            } else if (filter.allowInputEpsilon()) {
              filter.inputEpsilon();
              return r;
            } else
              continue;  // skip this arc; filter doesn't allow input epsilon
          } else {
            if ((r.matchedArcs = match->arcsMatchingInput(src.match, matchSym)))
              anyStandardMatch = true;  // will initSigma inside r after standard are exhausted
            else if (r.initRho() || r.initSigma()) {
            } else
              continue;
            filter.normal();
            return r;
          }
        }
      }

      concatState = matchDone;
      if (!anyStandardMatch && filter.allowMatchEpsilon() && match->whichSpecials.test(PHI::id)
          && (r.matchedArcs = match->arcsMatchingInput(src.match, PHI::ID))) {
        filter.matchEpsilon();
        r.inArc.dst = src.input;
        r.inArc.labelPair.first = EPSILON::ID;
        r.inArc.labelPair.second = NoSymbol;
#if SDL_VALGRIND
        //r.inArc.weight = Weight::one();
#endif
        // weight is set to one in ArcsGen
        // maybe todo: keep track of eps matches also and report dead-state-ness. or caller can just detect
        // empty outarcs range manually
      } else {
        r.setDone();  // returns empty range instead
      }

      // returning a sentinel empty generator is simpler than having a tricky
      // operator bool() accounting for this case
      return r;
    }


    ArcsGenGen(State const& src, InputPtr const& input, MatchPtr const& match, Times const& times)
        : Times(times)
        , src(src)
        , inArcs(input->outArcs(src.input))
        , match(match)
        , concatState(matchEpsilon)
        , anyStandardMatch()
        , epsilon(match->whichSpecials.test(EPSILON::id) && src.allowMatchEpsilon()
                      ? match->arcsMatchingInput(src.match, EPSILON::ID)
                      : MatchArcs())  // nonconsuming wildcard
    {}
    State src;
    InputArcs inArcs;
    MatchPtr match;
    ConcatState concatState;  // because we want to emit 3 different types of arcs
    bool anyStandardMatch;
    MatchArcs epsilon;  // we don't save phi because it's not subject to reuse; it's the last thing we do
    friend inline std::ostream& operator<<(std::ostream& out, ArcsGenGen const& self) {
      out << "ArcsGenGen[";
      out << self.src << " stage=" << self.concatState << " any=" << self.anyStandardMatch;
      out << "]";
      return out;
    }
  };

  typedef DistanceForFstArc<Weight> DistanceFn;
  typedef ArcsGenGen GenGenImpl;
  typedef boost::intrusive_ptr<GenGenImpl> GenGenPtr;
  typedef Util::SharedGenerator<GenGenPtr> GenGen;
  typedef Util::FlattenGenerators<GenGen, DistanceFn> ArcsImpl;
  typedef boost::intrusive_ptr<ArcsImpl> ArcsPtr;
  typedef Util::SharedGenerator<ArcsPtr> Arcs;
  Arcs outArcs(State const& src) const {
    GenGenPtr pGenGen(new GenGenImpl(src, input, match, *this));
    ArcsPtr pArcs(new ArcsImpl(GenGen(pGenGen), DistanceFn()));
    return Arcs(pArcs);
  }

  State startState() const {
    State r;
    r.input = input->startState();
    r.match = match->startState();
    r.setFilterStart();
    return r;
  }

  bool final(State const& st) const { return input->final(st.input) && match->final(st.match); }

  Level level(State const& st) const {
    return input->level(st.input);
    // TODO: return match->combinedLevel(input->level(st.input), st.match);
  }

  Distance heuristic(State const& st) const {
    return input->heuristic(st.input) + match->heuristic(st.match);
  }

  InputPtr input;
  MatchPtr match;
};

// viterbi-like (idempotent plus) semirings do not need redundant paths removed unless you want nbest
template <class Weight, class Enable = void>
struct AllowDuplicateFilter {
  typedef Epsilon1First type;
};

template <class Weight>
struct AllowDuplicateFilter<Weight, typename boost::enable_if<WeightIdempotentPlus<Weight> >::type> {
  typedef NoEpsilonFilter type;
};


template <template <class> class FstForArc, class Filter, class InHg, class MatchHg, class ArcOut>
void composeWithEpsilonFilterImpl(InHg const& inHg, MatchHg& matchHg, IMutableHypergraph<ArcOut>* outHg,
                                  FstComposeOptions const& opt,
                                  WhichFstComposeSpecials which = WhichFstComposeSpecials::undefined()) {
  typedef typename InHg::Arc Arc1;
  typedef typename MatchHg::Arc Arc2;
  IVocabularyPtr const& vocab = matchHg.getVocabulary();
  outHg->setVocabulary(vocab);
  if (vocab != inHg.getVocabulary())
    SDL_THROW_LOG(Hypergraph.fs.Compose, ConfigException, "input*match FST compose vocabularies must match");
  typedef FstForArc<Arc1> Input;
  typedef HypergraphMatchFst<Arc2> Match;
  typedef ComposeFst<Input, Match, Filter, TimesByMix<typename Arc1::Weight, typename Arc2::Weight> > ComposedLazy;
  ComposedLazy composedLazy;
  composedLazy.input.reset(new Input(inHg, opt.annotations));
  composedLazy.match.reset(new Match(matchHg, which));
  composedLazy.mix = opt.mix;
  saveFst(composedLazy, *outHg, opt);
}

/**
   Filter is an epsilon filter type

   to allow duplicates is to use a simpler filter state that doesn't rigorously sequence alternative a:*e* in
   inHg and *e*:b in matchHg - this would be harmful in case of Log/Expectation weight because the number of
   paths is higher than it should be, but might be faster for Viterbi/Feature weight (with no harm done).
*/
template <class Filter, class Arc1, class MatchHg, class ArcOut>
void composeWithEpsilonFilter(IMutableHypergraph<Arc1>& inHg, MatchHg& matchHg,
                              IMutableHypergraph<ArcOut>* outHg, FstComposeOptions const& opt,
                              WhichFstComposeSpecials which = WhichFstComposeSpecials::undefined()) {
  bool makeBestFirst = opt.sortBestFirst;
  if (makeBestFirst) inHg.forceBestFirstArcs();
  composeWithEpsilonFilterImpl<HypergraphFst, Filter>(inHg, matchHg, outHg, opt, which);
}

template <class Filter, class Arc1, class MatchHg, class ArcOut>
void composeWithEpsilonFilter(IMutableHypergraph<Arc1> const& inHg, MatchHg& matchHg,
                              IMutableHypergraph<ArcOut>* outHg, FstComposeOptions const& opt,
                              WhichFstComposeSpecials which = WhichFstComposeSpecials::undefined()) {
  composeWithEpsilonFilterImpl<HypergraphFst, Filter>(inHg, matchHg, outHg, opt, which);
}

template <class Filter, class Arc1, class MatchHg, class ArcOut>
void composeWithEpsilonFilter(IHypergraph<Arc1> const& inHg, MatchHg& matchHg,
                              IMutableHypergraph<ArcOut>* outHg, FstComposeOptions const& opt,
                              WhichFstComposeSpecials which = WhichFstComposeSpecials::undefined()) {
  composeWithEpsilonFilterImpl<ConstHypergraphFst, Filter>(inHg, matchHg, outHg, opt, which);
}

/**
   weight type of arc3 should be same as InHg::Arc.

   if InHg is IHypergraph, then we use ConstHypergraphFst, which is slightly
   slower than if InHg is IMutableHypergraph (HypergraphFst)
*/
template <class InHg, class MatchHg, class ArcOut>
void compose(InHg& inHg, MatchHg& matchHg, IMutableHypergraph<ArcOut>* outHg, FstComposeOptions const& opt) {
  WhichFstComposeSpecials which = matchHg.whichInputFstComposeSpecials();
  SDL_DEBUG(Hypergraph.compose, which);
  if (opt.allowDuplicatesEffective())
    composeWithEpsilonFilter<typename AllowDuplicateFilter<typename InHg::Arc::Weight>::type>(
        inHg, matchHg, outHg, opt, which);
  else if (opt.epsilonMatchingFilter)
    composeWithEpsilonFilter<EpsilonCombine>(inHg, matchHg, outHg, opt, which);
  else
    composeWithEpsilonFilter<Epsilon1First>(inHg, matchHg, outHg, opt, which);
}
}
}
}


namespace boost {
template <class A, class B, class C>
struct hash<sdl::Hypergraph::fs::Compose2State<A, B, C> > {
  std::size_t operator()(sdl::Hypergraph::fs::Compose2State<A, B, C> const& x) const { return hash_value(x); }
};


}

#endif
