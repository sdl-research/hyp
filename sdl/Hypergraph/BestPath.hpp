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
   bottom-up 1-best and (subsequent, top-down lazy) n-best for hypergraphs
   (Derivation trees ordered by arc.weight.getValue() sum, effectively - see
   HypergraphTraits.hpp)

   TODO: call fs/LazyBest for fsm (note: right now this won't preserve original state ids)

   it should be slightly faster by not making a copy of the input hg,
   effectively (the copy is used because we anyway, for the general hg case,
   have to keep track of the number of tails remaining on each arc
   (alternatively we could have forced the hg to have the right index, probably
   kStoreOutArcs, and store the tails remaining externally, e.g. by
   unordered_map<Arc *,unsigned>

   simple usage:

   IHypergraph<Arc> hg;
   unsigned n=10;

   std::vector<DerivationPtr> derivs;
   // (it's better if you can use the visitor interface instead of storing a vector, though)
   visit_nbest(holdNbestDerivations(derivs, n, hg);

   NbestId is kFirstNbestId == 0 for the 1-best
*/

#ifndef HYP__HG_BESTPATH_HPP
#define HYP__HG_BESTPATH_HPP
#pragma once

/// only include lazy_forest_kbest via this file (separate compilation units: differences ok)
#define LAZY_FOREST_KBEST_SIZE ::sdl::Hypergraph::NbestId

/* best tree from HG: (Knuth77 for viterbi cycles w/ negative costs but will
   diverge, or //TODO: mostly-topo-sort->Bellman-Ford until convergence for
   negative costs or non-viterbi semirings)

   also: lazy top-down n-best (uses 1-best bottom-up inside) - should work with cycles

   TODO: use hg.maxNotTerminalState to not waste array space on axioms

   TODO: binarization for nbest don't share any nonterminals. we could use a
   greedy algorithm to maximize sharing, or just do left->right but make sure to
   share suffixes.
*/

#include <stack>
#include <sdl/Hypergraph/PrintOptions.hpp>
#include <sdl/Hypergraph/Derivation.hpp>
#include <sdl/Hypergraph/GetString.hpp>
#include <sdl/Hypergraph/HypergraphTraits.hpp>
#include <sdl/Hypergraph/HypergraphWriter.hpp>
#include <sdl/Hypergraph/SamplePath.hpp>  // placeholder
#include <sdl/Hypergraph/Transform.hpp>
#include <sdl/Types.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Output.hpp>
#include <sdl/Util/Performance.hpp>

#include <graehl/shared/tails_up_hypergraph.hpp>  // djikstra best-first bottom-up
#include <graehl/shared/lazy_forest_kbest.hpp>  // binary forest top-down lazy nbest
// TODO: even better: don't generate lazy_forest nodes for parts of Hg that are high-cost
#include <graehl/shared/os.hpp>
#include <graehl/shared/pool_traits.hpp>
#include <graehl/shared/teestream.hpp>
#include <boost/pool/pool.hpp>
#include <sdl/Hypergraph/AcyclicBest.hpp>
#include <sdl/Config/Init.hpp>
#include <sdl/Hypergraph/Visit.hpp>
#include <sdl/Util/UninitializedArray.hpp>
#include <sdl/Util/Delete.hpp>
#include <sdl/Hypergraph/WeightUtil.hpp>
#include <sdl/Hypergraph/Types.hpp>

namespace sdl {
namespace Hypergraph {

/// instead of nbest, nbest plus up to num-ties tied for nth place
struct NbestPlusTies {
  NbestId nbest, numTies;
  NbestPlusTies(NbestId nbest = 1) : nbest(nbest), numTies() {}
  operator NbestId() const { return nbest; }
  template <class Config>
  void configure(Config& config) {
    config("nbest", &nbest)('n')
        .self_init()("limit hypergraph paths to this many, keeping the lowest totalcost");
    config("num-ties", &numTies)
        .self_init()(
            "allow this many extra same-score derivations with cost tied for the final place (if num-best > "
            "1)");
    config("num-best", &nbest).alias();
  }
  NbestId maxnbest() { return nbest > 1 ? nbest + numTies : 1; }
  bool multiple() const { return nbest > 1; }
  bool enabled() const { return nbest; }
  friend inline std::ostream& operator<<(std::ostream& out, NbestPlusTies const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const {
    out << nbest;
    if (numTies) out << "(+ " << numTies << " ties)";
  }
};

template <class Visitor>
struct VisitPlusTies {
  Visitor const& visitor;
  NbestId nbest;
  mutable SdlFloat lastCost;
  VisitPlusTies(Visitor const& visitor, NbestId nbest) : visitor(visitor), nbest(nbest), lastCost() {}
  template <class Weight>
  bool operator()(DerivationPtr const& deriv, Weight const& wtotal, NbestId n) const {
    SdlFloat thisCost = wtotal.getValue();
    if (n >= nbest && thisCost > lastCost)
      return false;
    else {
      if (n < nbest || thisCost < lastCost) lastCost = thisCost;
      return visitor(deriv, wtotal, n);
    }
  }
};

template <class Visitor>
VisitPlusTies<Visitor> visitPlusTies(Visitor const& visitor, NbestId nbestSoft) {
  return VisitPlusTies<Visitor>(visitor, nbestSoft);
}

template <class Arc>
Properties propertiesForBest(IHypergraph<Arc> const& hg) {
  return hg.isGraph() ? kStoreFirstTailOutArcs : kStoreInArcs;
}

enum { kFirstNbestId = 0 };

enum { kNoThrowEmptySetException = 0, kThrowEmptySetException = 1 };

inline void maybeThrowEmptySetException(bool throwEmptySetException) {
  if (throwEmptySetException) {
    SDL_THROW_LOG(Hypergraph.BestPath, EmptySetException, "no final state, so no best derivation.");
  }
}

VERBOSE_EXCEPTION_DECLARE(BestPathException)

/// example nbest visitor that fills your vector of DerivationPtr with the (up to) nbest. you may prefer to iterate
/// over a vector to writing your own visitor.
template <class Arc>
struct HoldNbestDerivAndWeights {
  typedef typename Arc::Weight Weight;
  typedef Derivation::DerivAndWeight<Arc> DerivAndWeight;
  typedef std::vector<DerivAndWeight> Derivs;
  Derivs& derivs;
  HoldNbestDerivAndWeights(HoldNbestDerivAndWeights const& o) : derivs(o.derivs) {}
  HoldNbestDerivAndWeights(Derivs& derivs) : derivs(derivs) {}
  bool operator()(DerivationPtr const& deriv, Weight const& w, NbestId) const {
    derivs.push_back(Derivation::DerivAndWeight<Arc>(deriv, w));
    return true;
  }
};

template <class Arc>
struct HoldNbestDerivs {
  typedef std::vector<DerivationPtr> Derivs;
  Derivs& derivs;
  typedef typename Arc::Weight Weight;
  HoldNbestDerivs(HoldNbestDerivs const& o) : derivs(o.derivs) {}
  HoldNbestDerivs(Derivs& derivs) : derivs(derivs) {}
  bool operator()(DerivationPtr const& deriv, Weight const&, NbestId) const {
    derivs.push_back(deriv);
    return true;
  }
};

struct PathOutOptions : DerivationStringOptions {
  bool padnbest;
  std::string prefix;
  bool weight;
  bool inyield;
  bool outyield;
  bool print_empty;
  bool outderiv;
  bool keepOriginalStateIds;
  bool outHypergraph;
  bool nbestIndex;
  PathOutOptions(SymbolQuotation quote = kQuoted)
      // TODO: unquoted is probably a better default since tokens can't have
      // spaces (postponed: this will require changing many tests' expected output)
      : DerivationStringOptions(quote, " ") {
    defaults();
  }

  template <class Out>
  void pad(Out& out, NbestId nVisited, NbestId padTo) const {
    if (padnbest)
      for (; nVisited < padTo; ++nVisited) out << '\n';
  }

  void defaults() {
    nbestIndex = true;
    weight = true;
    Config::inits(this);
  }
  void setBare1Best() {
    defaults();
    outyield = true;
    inyield = false;
    types = kLexical_Only;
    leafOnly = kLeafOnly;
    quote = kUnquoted;
    nbestIndex = false;
    weight = false;
  }
  // Derivation visitor and filter for BestPath::Nbest?
  static char const* caption() { return "Nbest Output"; }
  template <class Conf>
  void configure(Conf& c) {
    DerivationStringOptions::configure(c);
    c.is("Nbest output");
    c("formatting N-best derivation lists");
    c("pad-nbest", &padnbest)
        .init(false)(
            "if there are m<n derivations, print n-m blank lines (making a total of n lines, provided you "
            "don't enable additional lines of output).");
    c("weight", &weight)('w').self_init()(
        "before path yields and derivation, print weight, space (if no yield, then newline follows anyway)");
    c("nbest-index", &nbestIndex)
        .self_init()  // because StatisticalTokenizer needs different default
        ("before weight, print 'n = N' where N = 1..[--nbest]");
    c("prefix", &prefix).init("")("prepend this string to each nbest line (include your own trailing space)");
    c("inyield", &inyield)('y').init(true)("just print string (input yield) if it exists (one line)");
    c("outyield", &outyield)('Y').init(false)(
        "print output yield string (one line) - if both inyield and outyield, input comes before output");
    c("outderiv", &outderiv)('d').init(false)("print output derivation tree (on its own line)");
    c("empty-yield", &print_empty)('E')
        .init(true)("if there's no path, print a weight-0 empty-string yield")
        .init(print_empty);
    c("keep-original-stateids", &keepOriginalStateIds)
        .init(false)("true => keep original ids in --outhypergraph (may also affect --outderiv)");
    c("outhypergraph", &outHypergraph)
        .init(false)("for each nbest print hypergraph of derivation tree terminated by 0 byte");
  }
  bool needsOriginalStateIds() const { return keepOriginalStateIds && (outderiv || outHypergraph); }

  template <class Out, class Arc>
  struct PathOutHgNbestVisitor {
    PathOutOptions const& po;
    Out& out;
    IHypergraph<Arc> const& i;
    explicit PathOutHgNbestVisitor(PathOutOptions const& po, Out& out, IHypergraph<Arc> const& i)
        : po(po), out(out), i(i) {}

    /**
       important: return false if you want nbest visiting to stop before n.
    */
    bool operator()(DerivationPtr const& dp, typename Arc::Weight const&, NbestId n) const {
      po.maybe_print_deriv(out, dp, i, n);
      return true;
    }
  };

  template <class Out, class Arc>
  PathOutHgNbestVisitor<Out, Arc> pathOutNbestVisitor(Out& out, IHypergraph<Arc> const& i) const {
    return PathOutHgNbestVisitor<Out, Arc>(*this, out, i);
  }

  // newlines after each part
  template <class Out, class Arc>
  void maybe_print_deriv(Out& out, DerivationPtr const& d, IHypergraph<Arc> const& hg, NbestId n = 0) const {
    if (outderiv) {
      print_header<Arc>(out, d, n);
      out << Util::print(d, hg) << '\n';
    }
    maybe_print_paths(out, hg, d, n);
    if (outHypergraph) {
      MutableHypergraph<Arc> oh(kStoreInArcs);
      d->translateToHypergraph(hg, oh, keepOriginalStateIds);
      out << oh << '\0' << '\n';
    }
  }

  template <class Out, class Arc>
  void maybe_print_paths(Out& out, IHypergraph<Arc> const& hg, DerivationPtr const& d, NbestId n) const {
    if (inyield) maybe_print_path(out, hg, d, kInput, n);
    if (outyield) maybe_print_path(out, hg, d, kOutput, n);
    if (!inyield && !outyield && (weight || nbestIndex)) out << '\n';
  }

  template <class Out, class Arc>
  void maybe_print_path(Out& out, IHypergraph<Arc> const& hg, DerivationPtr const& d, LabelType labelType,
                        NbestId n) const {
    print_path(out, hg, d, labelType, n);
  }

  // prints a line no matter what (even if d is null)
  template <class Out, class Arc>
  void print_path(Out& out, IHypergraph<Arc> const& hg, DerivationPtr const& d, LabelType labelType,
                  NbestId n) const {
    if (d) {
      print_header<Arc>(out, d, n);
      DerivationStringOptions dopt((*this));
      dopt.labelType = labelType;
      out << textFromDeriv(d, hg, dopt);
    }
    out << '\n';
  }

  template <class Arc, class Out>
  void print_header(Out& o, DerivationPtr const& d, NbestId n) const {
    print_header_weight(o, d->weightForArc<Arc>(), n);
  }

  template <class Out, class W>
  void print_header_weight(Out& o, W const& w, NbestId n) const {
    if (nbestIndex) o << "n=" << n + 1 << ' ';
    if (weight) o << w << ' ';
  }
};

struct BestPathOptions : graehl::BestTreeOptions {
  typedef graehl::BestTreeOptions Base;
  bool time1best;
  bool topo;
  bool dupInput;
  bool dupOutput;
  bool acyclic;
  std::size_t acyclicMaxBackEdges;
  LabelType dupLabels() const {
    int f = (dupInput ? kInput : kNo_Label) | (dupOutput ? kOutput : kNo_Label);
    return (LabelType)f;
  }
  unsigned maxPerString;
  bool bestfirst;
  bool random;
  static inline std::string usage() {
    return
        // TODO: implement bellman-ford
        /*
          "Run Bellman-Ford algorithm (iterative relaxing) until convergence. "
          "For Viterbi semiring, up to O(VE) = O(V^3) but correct for negative "
          "(no-cycle) costs. If best-first option is used, run "*/
        "Knuth77 best CFG derivation algorithm, "
        "which is O(E*lg V) but may loop forever if negative weight cycles, and "
        "not correct for non-viterbi semirings.";
  }
  BestPathOptions() { defaults(); }
  void defaults() {
    Base::defaults();
    Config::inits(this);
    // topo = true;
    random = false;
    throw_on_rereach_limit = false;
    validate();
  }
  static char const* caption() { return "Best-Path Computation"; }

  bool single;

  /** YAML or command line config */
  template <class Conf>
  void configure(Conf& c) {
    Base::configure(c);
    c("compute lowest cost derivation tree in hypergraph");
    c.is("Best path");
    c("best-first", &bestfirst)
        .init(true)(
            "Knuth77 bottom-up priority queue - good for viterbi semiring only. may not terminate if "
            "negative cost cycles, controllable by rereach, throw-on-rereach, convergence. --convergence "
            "will avoid requeuing small improvements (less than cost/logprob delta) (this means the cost "
            "returned may be pessimistic, but the better subderivation is indeed recorded)");
    c("nbest-per-string", &maxPerString)
        .init(0)(
            "skip duplicate (same in/out labels), ensuring a full n unique label pairs are listed if they "
            "exist. 0 = unlimited");
    c("per-string-input", &dupInput).init(true)("consider input labels for nbest-per-string");
    c("per-string-output", &dupOutput).init(false)("consider output labels for nbest-per-string");
    c("time-1best", &time1best)
        .init(false)("report (sdl.Performance.Hypergraph.BestPath) time for computing 1best");
    c("acyclic", &acyclic)
        .init(true)(
            "attempt acyclic viterbi best-path even if input is not marked as acyclic, falling back to "
            "best-first bottom-up if it turns out to have cycles - enabling this is slower if you know your "
            "hypergraph has cycles. See WARN messages sdl.Hypergraph.BestPath.acyclic to see whether this "
            "is happening.");
    c("acyclic-max-back-edges", &acyclicMaxBackEdges)
        .init(0)(
            "accept acyclic best-path result if there are this many or fewer cycle-causing edges - the "
            "(n-)best paths are potentially wrong if this is > 0 - see INFO messages "
            "sdl.Hypergraph.BestPath.acyclic for reports that this might be happening.");
    c("allow-rereach", &allow_rereach)
        .init(1000000)
        .verbose()(
            "for non-acyclic hgs, it's necessary in case of negative weight arcs to repeat some best-path "
            "computations. this setting limits that recomputation to ensure termination in reasonable time "
            "(smaller = faster)");
    // TODO: default allow_rereach 0 and use lattice (sub?)-structure detection to avoid
    // priority-queueing problems. or implement convergence + negative-cost
    // cycle detection. for now it's better to give correct results for sure or
    // else loop a long time, so this is huge, not some reasonable value like 10
  }

  /**
     postponed strategies for negative-cost deltas:

     .defaulted("topo, t", &topo, "TODO: (as much as possible) topological order bottom-up - faster
     convergence")
     .defaulted("extra-iter", &extraIter, "TODO: always do V-1 iterations (bellman-ford convergence for
     viterbi). but do up to this many more (for other semirings) or convergence.")
     .defaulted("random", &random, "TODO: instead of best, sample a random path (from normalized tree
     weights)")

     postponed MBR-type options:

     sum-probability best-path decoding (at least note # duplicates as feature)

     keep-going past end of unique-nbest by X cost
  */

  void validate() const {
    if (!bestfirst)
      SDL_THROW_LOG(Hypergraph.BestPath, BestPathException,
                    "best-first is the only implemented best path algorithm, so --best-first = true is "
                    "mandatory.");
    if (maxPerString && !(dupInput || dupOutput))
      SDL_THROW_LOG(BestPath, ConfigException,
                    "if maxPerString is not 0, you must enable per-string-input and/or per-string-output");
    if (random)
      SDL_THROW_LOG(BestPath, UnimplementedException,
                    "random inside-prob sampling not implemented yet in BestPath");
  }
  friend void validate(BestPathOptions& opt) { opt.validate(); }
  bool duplicateCheckingEnabled() const { return maxPerString; }
  bool noFilterNeeded(NbestId nbest) const { return nbest == 1 || !duplicateCheckingEnabled(); }
};

typedef graehl::BestTreeStats BestPathStats;

struct BestPath : TransformBase<Transform::Inplace> {
  BestPathOptions opt;
  BestPath() {}
  BestPath(BestPathOptions const& opt) : opt(opt) {}
  BestPath(BestPath const& o) : opt(o.opt) {}

  /**
     TODO: optimization for Compute<Arc> that detects path-graph (#inarcs = 1 or
     fst #outarcs=1) and skips allocating per-arc and per-state storage.
  */
  template <class A>
  struct Compute {
    typedef A Arc;
    typedef IHypergraph<Arc> HG;
    typedef IMutableHypergraph<Arc> H;
    typedef graehl::property_factory<HG, graehl::edge_tag> EF;
    typedef graehl::property_factory<HG, graehl::vertex_tag> VF;

    BestPathOptions opt;
    HG const& hg;
    VF vf;
    EF ef;
    typedef typename Arc::Weight Weight;
    typedef graehl::path_traits<HG> path_traits;
    typedef typename path_traits::cost_type Cost;
    typedef graehl::built_pmap<graehl::vertex_tag, HG, Cost> MuB;
    MuB mub;
    typedef graehl::built_pmap<graehl::vertex_tag, HG, ArcHandle> PiB;
    Util::AutoDelete<PiB> pib;
    /**
       mu[StateId] = best cost from start/axioms to state.

       why property maps instead of a humble vector? because we can adapt
       IHypergraph to some existing best-tree from (recursive) hypergraph in
       graehl/tails_up_hypergraph. ultimately it compiles to the same thing.

    */
    typedef typename MuB::property_map_type Mu;
    Mu mu;  // initialized by either tails_up_hypergraph or AcyclicBest

    /**
       pi[StateId] = Arc * that can be used to achieve mu[StateId]
    */
    typedef typename PiB::property_map_type Pi;
    Pi pi;  // 0-initialized on construction

    mutable BestPathStats stat;
    typedef graehl::lazy_kbest_stats Nstats;
    mutable Nstats nstat;
    Compute(BestPathOptions opt, HG const& hg, bool storeBackPointers = true)
        : opt(opt), hg(hg), vf(hg), ef(hg), mub(hg, path_traits::unreachable()), mu(mub.pmap) {
      if (storeBackPointers) pi = pib.set(new PiB(hg, (ArcHandle)0))->pmap;
    }

    typedef Derivation::DerivAndWeight<Arc> DerivAndWeight;
    typedef Derivation::Alloc DerivAlloc;
    typedef DerivationChildren DerivChildren;

    struct BinaryDerivation;
    typedef BinaryDerivation D;
    typedef D* Dp;
    // for nbest: (binarized) subderivations

    struct NbestKeyAccum {
      /** this is a stateful function object used by BinaryYield for DupFilter.
          it's called with the lexical states of a binary derivation.

          \return vector of Sym for input and/or output (depending on
          labelType, separated by -1)
      */
      HG const* phg;
      LabelType labelType;
      NbestKeyAccum() {}
      NbestKeyAccum(HG const& hg, LabelType labelType) : phg(&hg), labelType(labelType) {}
      Syms in, out;
      void clear() {
        in.clear();
        out.clear();
      }
      /**
         \return remove-duplicates key - either input, output, or input <sep> output. idempotent
      */
      Syms const& key() {
        if (labelType == kInput) return in;
        if (labelType == kOutput) return out;
        if (out.empty()) return in;
        in.push_back(NoSymbol);
        Util::append(in, out);
        out.clear();
        return in;
      }
      static inline void appendLexical(Syms& str, Sym sym) {
        if (sym.isLexical()) str.push_back(sym);
      }
      void operator()(StateId stateId) {
        // TODO: this could be (with much special case code) be optimized to look at state ids only for
        // input-only hgs (memorize the epsilon stateid first), saving the hg calls
        if (labelType & kInput) appendLexical(in, phg->inputLabel(stateId));
        if (labelType & kOutput) appendLexical(out, phg->outputLabel(stateId));
      }
      template <class Out>
      static inline void word(Out& o, Sym s, IVocabularyPtr const& v) {
        if (v) o << ' ' << v->str(s);
        o << " #" << s;
      }

      template <class Out>
      void print(Out& o) const {
        IVocabularyPtr v = phg->getVocabulary();
        o << "nbest-dup-key [";
        unsigned i = 0, e = (unsigned)in.size();
        for (; i < e; ++i) {
          Sym s = in[i];
          if (s == NoSymbol) {
            o << " /";
            break;
          }
          word(o, s, v);
        }
        for (; i < e; ++i) word(o, in[i], v);
        e = (unsigned)out.size();
        if (e) {
          o << " /";
          for (i = 0; i < e; ++i) word(o, out[i], v);
        }
        o << " ]";
      }
      template <class Ch, class Tr>
      friend std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& o, NbestKeyAccum const& self) {
        self.print(o);
        return o;
      }
    };

    // TODO: pull this out from Compute so it's not templated on Weight by always using float for
    // Weight::FloatT? might save compile time.
    /**
       used in lazy_forest_kbest. POD.

       //TODO@JG: for leaves, it's probably possible to *not* build a lazy
       //forest node at all. to allow mu[leaf] != 0, we'd need some trickery,
       //but for now hypergraphs don't have initial weight. we have the same
       //consideration when duplicate filtering, though - so likely some code
       //semi-duplication to take advantage of the noFilter() case
       */
    struct BinaryDerivation {
      // trivial ctor/dtors so C++11 movable
      Cost w;  // this is an accumulated inside cost
      StateId leafState;  // will be kNoState unless leaf; needed for duplicate skipping
      // TODO: optionally maintain in/out/both yields incrementally for integrated lazy duplicate skipping
      // (hashed?)
      Dp children[2];  // 0-terminated (and not smart ptrs)
      ArcHandle a;  // this is null except after all children are binarized-built-up
      bool leaf() const { return leafState != kNoState; }

      // e.g. D[arc](c1, c2): an arc a(s1, s2, s3) with 3 children becomes:
      // D[a](D[](ds1, ds2), ds3), and a leaf sl becomes TRUTH (not identified with any arc)
      Arc& arc() const { return *(Arc*)a; }
      StateIdContainer& tails() const { return arc().tails(); }
      typedef Dp derivation_type;
      void initChildren() { children[0] = children[1] = NULL; }
      void init() {
        leafState = kNoState;
        children[0] = children[1] = NULL;
      }
      void init(Cost cost, Dp l, Dp r, Arc const* ap) {
        a = (ArcHandle)ap;
        w = cost;
        children[0] = l;
        children[1] = r;
        leafState = kNoState;
      }
      void init(Dp proto) { std::memcpy(this, proto, sizeof(BinaryDerivation)); }
      void init(Arc const* ap, Cost cost) {
        a = (ArcHandle)ap;
        w = cost;
        init();
      }
      void initLeaf(StateId st, Cost cost) {
        a = 0;
        leafState = st;
        w = cost;
        initChildren();
      }

      /**
         to init none and pending dummy values for lazy_forest_kbest. idempotent
         assignments of primitive types so no need for static init or locking
      */
      void initNone() {
        a = (ArcHandle)1;
        w = 0;
        init();
      }
      void initPending() {
        a = (ArcHandle)2;
        w = 0;
        init();
      }

      friend inline bool derivation_better_than(BinaryDerivation const& a, BinaryDerivation const& b) {
        return path_traits::better(a.w, b.w);
      }
      friend inline bool derivation_better_than(Dp a, Dp b) { return derivation_better_than(*a, *b); }
      typedef Derivation FD;

      void yield(NbestKeyAccum& accum) {
        if (this == &none) return;
        return yieldr(accum);
      }

      void yieldr(NbestKeyAccum& accum) {
        if (leaf()) {
          accum(leafState);
          return;
        }
        // your nonterminal may have a label, but we don't care about it. we want leaves only
        if (children[0]) {
          children[0]->yield(accum);
          if (children[1]) children[1]->yield(accum);
        }
      }

      DerivationPtr derivation() const {
        if (this == &none) return 0;
        if (leaf()) return Derivation::kAxiom;
        assert(a);
        StateIdContainer& t = tails();
        DerivationPtr p = FD::construct((Arc*)a, (TailId)t.size());
        unsigned i = 0;
        derivation(p->children, i);
        assert(i == p->children.size());
        return p;
      }
      void rderivation(std::vector<DerivationPtr>& ch, unsigned& i) const {
        if (leaf())
          ch[i++] = Derivation::kAxiom;
        else if (a)
          ch[i++] = derivation();
        else
          derivation(ch, i);
      }
      void derivation(std::vector<DerivationPtr>& ch, unsigned& i) const {
        if (children[0]) {
          children[0]->rderivation(ch, i);
          if (children[1]) children[1]->rderivation(ch, i);
        }
      }

      template <class Out>
      void print(Out& o, unsigned levels = (unsigned)-1) const {
        rprint(o, levels);
      }
      template <class Out>
      void rprint(Out& o, unsigned levels = (unsigned)-1) const {
        if (this == &pending)  // a==pending.a
          o << "PENDING";
        else if (this == &none)  // (a==none.a
          o << "NONE";
        else if (leaf())  //(a==TRUTH.a
          o << "()";
        else {
          if (w) o << '(' << w << ')';
          if (a) o << "{" << arc() << '}';
          if (children[0]) {
            if (!levels) {
              o << "[...]";
              return;
            }
            --levels;
            o << '[';
            children[0]->rprint(o, levels);
            if (children[1]) {
              o << ' ';
              children[1]->rprint(o, levels);
            }
            o << ']';
          }
        }
      }
      template <class Out>
      friend void print(Out& o, BinaryDerivation const& bd, unsigned levels = (unsigned)-1) {
        bd.print(o, levels);
      }
      template <class C, class T>
      friend std::basic_ostream<C, T>& operator<<(std::basic_ostream<C, T>& o, BinaryDerivation const& self) {
        self.print(o);
        return o;
      }
      template <class C, class T>
      friend std::basic_ostream<C, T>& operator<<(std::basic_ostream<C, T>& o, BinaryDerivation const* self) {
        o << "BD@" << (void*)self << ": ";
        if (self) self->print(o, 2);
        return o;
      }
    };  // end BinaryDerivation

    static BinaryDerivation none;  // for lazy kbest impl
    static BinaryDerivation pending;

    /**
       to init none and pending dummy values for lazy_forest_kbest. idempotent
       assignments of primitive types so no need for static init or locking
    */
    static void idempotentSentinelInit() {
      /**
         sentinel values; would be statically initialized but this is clearer
         and lets us use POD. (compound literals aren't part of C++98 but are of
         C99 and C++11)
      */
      none.initNone();
      pending.initPending();
    }

    /**
       storage for BinaryDerivations.
    */
    typedef boost::pool<> Dpool;
    // avoiding boost object_pool since we don't need destructor for BinaryDerivation

    /// a note about nbest ties: either we put in some extra work to make sure the bare 1-best derivation is
    /// the same one chosen by the n-best 1-best, or we allow the 1-best to change whether you print 1 or 2.
    /// this becomes even more difficult in case the 1-best was only approximate due to rereach/convergence
    /// limits and negative cost edges.

    /** stateful function object used by DupFilter for nbest-duplicate-skipping. .*/
    struct BinaryYield {
      NbestKeyAccum accum;  // note: not thread-safe. could make local and return-by-value instead of ref. in
      // the context of lazy-nbest, this doesn't matter. i.e. this state is local to the
      // forest node
      BinaryYield() {}
      //      BinaryYield(BinaryYield const& o) : accum(o.accum) {}
      //      BinaryYield(NbestKeyAccum const& o) : accum(o) {}
      BinaryYield(HG const& hg, LabelType labelType) : accum(hg, labelType) {}
      typedef Syms result_type;
      result_type const& operator()(Dp dp) {
        accum.clear();
        dp->yield(accum);
        SDL_TRACE(Hypergraph.BestPath.BinaryYield, "duplicate nbest filter key for " << *dp << ": " << accum);

        // TODO@JG: code special case for graph: no need to recompute yield and
        // store it as a vector; use linked list w/ canonical suffixes. for
        // syntax we need to flatten yield because the same string can be made
        // with different bracketings

        return accum.key();
      }
      template <class Out>
      void print_projection(Out& o, Dp dp) const {
        o << "yield for " << dp << ": " << accum;
      }
    };

    /** every lazy-nbest-for-state-forest object has a filter that keeps track of already seen BinaryYield. */
    typedef graehl::projected_duplicate_filter<Dp, BinaryYield, graehl::UmapS> DupFilter;
    // could use MapS (maybe smaller hash map but slower) - but then need a lexicographic operator < on
    // Syms

    /** This prototype filter is copied into every lazy-nbest-forest
       object and then filled. note that this is a trivial pass-through (doesn't
       track anything) if opt.maxPerString is 0 (but to save some space, we check for 0 and then use
       noFilter() instead of filter())

       If you have a finite number of derivations, this filtering will always
       terminate. otherwise, if you ask for more unique strings than are
       possible, you might not terminate (this "might" should be revisited
       empirically with various loopy CFGs to see if it's a fact; then we can
       add a limit for # of filtered derivations and terminate early after it's
       reached
    */
    DupFilter filter() { return DupFilter(opt.maxPerString, BinaryYield(hg, opt.dupLabels())); }
    typedef graehl::permissive_kbest_filter NoDuplicateFilter;
    static inline NoDuplicateFilter noFilter() { return NoDuplicateFilter(); }

    /// Don't be surprised if in nbest output, your previously computed 1-best
    /// derivation changes (to something same cost or better) from your
    /// approximate 1-best w/ negative edges (or in ties, which always arise in
    /// nontrivial unweighted lattices).
    struct NbestDerivationFactory {
      Dpool* dpool;
      NbestDerivationFactory() : dpool(NULL) {}  // because some default ctor is required.
      Dp construct_binary(Cost w, Dp l, Dp r, Arc* a) const {
        Dp d = (Dp)dpool->malloc();
        d->init(w, l, r, a);
        return d;
      }
      explicit NbestDerivationFactory(Dpool& dpool) : dpool(&dpool) {}
      typedef Dp derivation_type;
      static inline derivation_type NONE() { return &none; }
      static inline derivation_type PENDING() { return &pending; }
      static inline Cost replaceCost(Dp old, Dp oldc, Dp newc) {
        return path_traits::extend(path_traits::retract(old->w, oldc->w), newc->w);
      }

      void fix_edge(derivation_type const& d, derivation_type c0, derivation_type c1 = 0) const {
        Dp old0 = d->children[0], old1 = d->children[1];
        assert(old0 != &none);
        assert(old1 != &none);
        if (old0 && !old0->leaf()) {
          assert(c0);
          assert(c0 != &none);
          if (old0 != c0) {
            d->children[0] = c0;
            d->w = replaceCost(d, old0, c0);
          }
          if (old1 && !old1->leaf()) {
            assert(c1);
            assert(c1 != &none);
            if (old1 != c1) {
              d->children[1] = c1;
              d->w = replaceCost(d, old1, c1);
            }
          }
        }
      }

      // perhaps change this to recompute the entire weight from rule + children, rather than removing one
      // child then adding new?
      derivation_type make_worse(derivation_type prototype, derivation_type old_child,
                                 derivation_type new_child, NbestId changed_child_index) {
        // this (and the initial 1-best derivation) must be consistent w/ fv[st].pq[0] unless we separate pq
        // from 'successor from last added'?
        // optimization based on path_traits::retract (subtract out cost of old)
        Dp r = (Dp)dpool->malloc();
        r->init(prototype);
        assert(r->w == prototype->w);
        assert(r->children[0] == prototype->children[0]);
        assert(old_child != new_child);
        r->children[changed_child_index] = new_child;
        r->w = replaceCost(r, old_child, new_child);
        return r;
      }
      StateSet seen;
      /// called by lazy_forest_kbest to compensate for wrong 1-best (negative edges)
      bool needs_fixing(Dp d) { return d && d->a && Util::latch(seen, d->arc().head()); }
    };

    /// this is for n>1 best only. 1-best uses less memory/time. gets visited for all Arc*.
    template <class FilterFactory = graehl::permissive_kbest_filter_factory>
    struct BuildLazy {
      typedef graehl::lazy_forest<NbestDerivationFactory, FilterFactory> NbestForest;
      typedef typename NbestForest::Environment Env;
      // typedef NbestForest::hyperedge Nbarc;
      typedef Pool::object_pool<NbestForest> Fpool;  // used for binary-internal nodes
      typedef NbestForest* Fp;

      mutable Env env;

      typedef Util::UnconstructedArray<NbestForest> Forests;
      typedef Util::UninitializedArray<Dp> BestDerivations;  // 1-best derivations for each vertex. by
      // pointer, so we can hook them up (vector going
      // away leaves them alone; they belong to pool)

      /**
         since boost object_pool has a performance bug when you free individual
         objects in advance, i don't free any. due to the lazy memoization in
         lazy_forest_kbest, you couldn't anyway.
      */
      mutable Fpool fpool;
      mutable Dpool dpool;
      Forests fvec;
      BestDerivations dvec;
      NbestForest* fv;
      Dp* dv;  // we initialize these with 1-best subderivs (some of these will be leaves and have leafState)
      Mu mu;
      Cost bestCost(Arc* a) const {
        Cost w = a->weight().getValue();
        StateIdContainer const& tails = a->tails();
        for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i != e; ++i) {
          w = path_traits::extend(mu[*i], w);
        }
        return w;
      }
      /* you'll get e.g. (see regr and unit tests)

         n = 1 -0.1 "(foo" "cud)"
         n = 2 0 "(xyz" "foo" "cud)"
         n = 3 0 "(xyz" "as" "cud)"

         for input hg:

         START <- 0
         FINAL <- 1
         2 <- 0 ("(foo") / -.1
         1 <- 2 ("cud)") / 0
         3 <- 0 ("(xyz") / 0
         2 <- 3 ("foo") / 0
         2 <- 3 ("as") / 0
      */
      StateId final, nStates;
      BuildLazy(IHypergraph<Arc> const& hg, StateId N, FilterFactory const& filterFactory, Mu const& mu, Pi pi)
          : dpool(sizeof(BinaryDerivation))
          , fvec(N)
          , dvec(N)
          , fv(&fvec[0])
          , dv(&dvec[0])
          , mu(mu)
          , final(hg.final())
          , nStates(N) {
        idempotentSentinelInit();

        NbestDerivationFactory df(dpool);
        env.set_derivation_factory(df);

        env.set_filter_factory(filterFactory);

        bool const outarcs = hg.storesOutArcs();

        for (StateId st = 0; st < nStates; ++st) {
          new (&fv[st]) NbestForest(env);  // construct
          fvec.constructed(st);
          if (outarcs) fv[st].reserve(hg.numOutArcs(st));
          Arc* bi = (Arc*)get(pi, st);
          if (bi)
            (dv[st] = (Dp)dpool.malloc())->init(bi, get(mu, st));
          else if (hg.isAxiom(st))
            (dv[st] = (Dp)dpool.malloc())->initLeaf(st, get(mu, st));
          else
            dv[st] = 0;
          // 1best binary derivation - need to fill in binary children later, but cost is correct. note: maybe
          // some v aren't derivable. in that case NULL.
        }
        // add edges to forest - binarized edges are already sorted but the rest need to be
        visitArcs(hg, *this);
        sort();
        StateSet& seen = env.derivation_factory.seen;  // reusing an existing array for efficiency
        seen.clear();
        seen.resize(N);
        goal().fix_edges(env, true);  // in case we have ties in costs or wrong 1-best, make our derivations
        // consistent with nbest forest
        seen.clear();
      }

      NbestForest& goal() const { return fv[final]; }

      template <class Visitor>
      graehl::lazy_kbest_stats enumerate_kbest(graehl::lazy_kbest_index_type n, Visitor const& visit) const {
        return goal().enumerate_kbest(env, n, visit);
      }

      /**
         binarize and add (parallel binarized derivation structure and
         lazy-forest node structure, rooted at existing derivation and forest for
         hypergraph state
      */
      bool operator()(IHypergraph<Arc> const&, Arc* a) const {
        StateId h = a->head();
        NbestForest& fh = fv[h];
        Dp dh = dv[h];
        if (!dh) return true;
        StateIdContainer const& tails = a->tails();
        unsigned N = (unsigned)tails.size();
        for (unsigned i = 0; i < N; ++i) {
          StateId tl = tails[i];
          if (!dv[tl]) return true;
        }
        Dp d;
        Cost w = bestCost(a);
        if (dh->a == a)
          d = dh;
        else {
          d = (Dp)dpool.malloc();
          d->init(a, w);
        }
        if (N == 0)
          fh.add(d);
        else if (N == 1) {
          StateId tl = tails[0];
          d->children[0] = dv[tl];
          fh.add(d, &fv[tl]);
        } else {
          // TODO: test on non-binary HG
          // binarize then final 2-child
          StateId l = tails[0];
          Fp lf = &fv[l];
          Dp ld = dv[l];
          assert(ld);
          // TODO: simplify code by recursion? going reverse order? right now we're going left->right and
          // modifying previous?
          for (unsigned i = 1; i < N - 1;
               ++i) {  // introduce new internal nodes with appropriate inside costs
            StateId prev = tails[i - 1], r = tails[i];
            w = path_traits::retract(w, mu[prev]);  // exclude the part from previous outside left child
            Fp rf = &fv[r];
            Dp rd = dv[r];
            assert(rd);
            Dp d2 = (Dp)dpool.malloc();
            d2->init(w, ld, rd, 0);
            Fp f = fpool.construct(env);
            f->add_first_sorted(env, d2, lf, rf);  // since this is the only thing f will have added, and we
            // will only call sort on vertices' forests
            ld = d2;
            lf = f;
          }
          // TODO: test on N>2 tails (some CFG)
          // update vertex forest
          StateId r = tails[N - 1];
          d->children[0] = ld;
          d->children[1] = dv[r];
          fh.add(d, lf, &fv[r]);
        }
        return true;
      }

     private:
      void sort() const {
        for (StateId st = 0; st < nStates; ++st) fv[st].sort(env);
      }
    };  // end BuildLazy

    template <class DerivVisitor>
    struct BinaryVisitor {
      DerivVisitor v;
      HG const& hg;
      BinaryVisitor(DerivVisitor const& v, HG const& hg) : v(v), hg(hg) {}
      bool operator()(Dp b, NbestId n) const {
        DerivationPtr d = b->derivation();
        SdlFloat cost = b->w;
        Weight const& w = d->weight<Weight>();
        SdlFloat deriv_cost = w.value_;
        if (!graehl::cost_path_traits<SdlFloat>::close_enough(cost, deriv_cost)) {
          SDL_WARN(Hypergraph.BestPath, (n + 1) << "-best cost=" << cost
                                                << " != derivation cost=" << deriv_cost);
          SDL_DEBUG(Hypergraph.BestPath, (n + 1) << "-best cost=" << cost
                                                 << " does not match derivation.weight() cost=" << deriv_cost
                                                 << " deriv=" << Util::print(d, hg));
        }
        return v(d, w, n);
      }
    };

    struct IgnoreVisitor {
      template <class Weight>
      bool operator()(DerivationPtr const&, Weight const&, NbestId) const {
        return true;
      }
    };

    /**
       return true if hg was acyclic and could be handled (non-fsa + non-in-arcs = can't).

       post: mu/pi reset to normal so can use regular best-first bottom-up hypergraph best tree

       TODO: allow kbest w/ inarcs only
    */
    template <bool IsGraph>
    bool acyclicBest() {
      AcyclicBest<Arc, Mu, Pi, IsGraph> acyclic(hg, mu, pi, opt.acyclicMaxBackEdges);
      if (acyclic.back_edges_ <= opt.acyclicMaxBackEdges) {
        if (acyclic.back_edges_)
          SDL_INFO(Hypergraph.BestPath.acyclic,
                   "hypergraph was not acyclic - using almost-acyclic best path since there were "
                       << acyclic.back_edges_ << " <= " << opt.acyclicMaxBackEdges
                       << "(acyclic-max-back-edges) cycle-causing edges");
        else
          SDL_DEBUG(Hypergraph.BestPath, "hypergraph had no back edges (and " << acyclic.self_loops_
                                                                              << " tail=head self-loops)");
        return true;
      } else {  // reset mu for non-acyclic version
        SDL_WARN(Hypergraph.BestPath.acyclic,
                 "You asked for acyclic best-path, but there were cycles due to "
                     << acyclic.back_edges_ << " cycle-causing arcs during topological sort. "
                                               "For greater speed in this case, set acyclic: false or else "
                                               "(risking 1-best inaccuracy) increase the configured "
                                               "acyclic-max-back-edges: " << opt.acyclicMaxBackEdges);
        acyclic.resetPi();  // so we can use !acyclic case in visit_nbest
        return false;
      }
    }

    template <class Pmap>
    void logHeadDebug(char const* name, Pmap const& pmap, StateId N, StateId headN) {
      SDL_DEBUG(Hypergraph.BestPath, name << ": " << Util::printPrefix(&pmap[0], N, headN) << " ... (head "
                                          << headN << ")");
    }

    /**
       calls visitor(DerivationPtr, Cost, n), where n = 0 for the best, n = 1 for 2nd best
       ... skips duplicates (internal to lazy nbest algorithm) according to filter. returns 1best (or throws
       EmptySetException if no derivations).

       if throwEmptySetException, throw EmptySetException if there's no 1best. otherwise return null ptr

       if nVisited, set *nVisited to number of best paths actually visited (for pad-nbest)
    */
    template <class Filter, class DerivVisitor>
    DerivationPtr visit_nbestFilter(NbestId nbest, DerivVisitor const& visitor, Filter const& filter = Filter(),
                             bool throwEmptySetException = false, NbestId* nVisited = 0,
                             bool onlyBestPathIf1best = false) {
      BinaryVisitor<DerivVisitor> binvisitor(visitor, hg);
      DerivationPtr r = 0;
      if (nbest == 0) {
        SDL_INFO(Hypergraph.BestPath, "requested 0-best only - not doing anything.");
        return r;
      }

      StateId N = hg.sizeForHeads();
      StateId final = hg.final();

      bool const simpleGraph = hg.isGraph() && hg.start() != kNoState;
      // TODO@JG: we can have something that's structurally a graph (non-leaf
      // tails may only appear in 1st position) but w/ no start state; in this
      // case all the leaves are axioms (CFG case). acyclic best will still
      // work even w/o a start state (just visit all the tails). however, i'd
      // have to be very certain that isFsm doesn't false positive trigger on
      // S(NP, VP) and checks that VP is a lexical leaf (see
      // isFsmArc). therefore simple graph 1-best is only
      // enabled if there's a start state
      if (final == Hypergraph::kNoState) {  // pruned empty
        maybeThrowEmptySetException(throwEmptySetException);
        return DerivationPtr();
        // find 1best

      } else {
        assert(final < N);
        Util::Performance time1("Hypergraph.BestPath");
        if (!opt.time1best) time1.disableReport();
        typedef ReadEdgeCostMap<Arc> Ecost;
        Ecost ec;

        bool const canAcyclic = simpleGraph
                                && (onlyBestPathIf1best && nbest == 1 || hg.tryForceFirstTailOutArcs());
        // TODO@JG: implement finite CFG case in AcyclicBest
        // TODO@JG: implement non-mutable case in AcyclicBest (data stack for copy of arc adjacencies during
        // dfs)
        bool const isAcyclic
            = hasProperties(hg.properties(), kAcyclic);  // TODO@JG: actually, we can do acyclic 1-best even
        // when there are self-loops. but that's ok because
        // of the acyclic option trying even when we don't
        // have it marked acyclic
        if (isAcyclic) {
          SDL_DEBUG(Hypergraph.BestPath, "hg is marked as acyclic");
          if (!canAcyclic)
            SDL_WARN(Hypergraph.BestPath,
                     "hg is marked as acyclic but is a CFG that doesn't store in-arcs; in the future, store "
                     "in-arcs to enable faster acyclic 1-best");
        }
        bool const tryAcyclic
            = canAcyclic && (opt.acyclic || isAcyclic);  // might be acyclic even though not marked as such
        bool const gotAcyclic = tryAcyclic && (simpleGraph ? acyclicBest<true>() : acyclicBest<false>());
        if (!gotAcyclic) {
          typedef graehl::TailsUpHypergraph<HG> Tails;
          Tails tails(hg, vf, ef);

          typename Tails::BestTreeOptionsParsed parsedOpt((opt));
          typedef typename Tails::template BestTree<Mu, Pi, Ecost> BestTree;
          BestTree b1(tails, mu, pi, ec, parsedOpt);
          b1.init_costs();
          hg.forAxiomIds(b1.axiom_adder());
          b1.finish();
          stat = b1.stat;
          if (stat.n_blocked_rereach || stat.n_converged_inexact) {
            SDL_WARN(
                Hypergraph.BestPath,
                stat.n_blocked_rereach
                    << " blocked improvements (due to max --rereach=" << opt.allow_rereach << "), and "
                    << stat.n_converged_inexact
                    << " convergence-limited (within --convergence=" << opt.convergence_epsilon_str
                    << ") relaxations of already best-first reached states - indicates a negative effective "
                       "cost hyperedge or cycle of them. Expect some out of order (by weight) n-bests.");
          }
          SDL_DEBUG(Hypergraph.BestPath, stat);
        }

        Cost bestCost = get(mu, final);
        SDL_DEBUG(Hypergraph.BestPath, "inside[final]=" << bestCost);

        if (bestCost == path_traits::unreachable()) {
          if (throwEmptySetException)
            SDL_THROW_LOG(Hypergraph.BestPath, EmptySetException, "no paths to final state " << final);
          if (nVisited) *nVisited = 0;
          return DerivationPtr();
        }

        r = derivation(final);

        SDL_DEBUG(Hypergraph.BestPath, "found 1-best; visiting " << (nbest > 1 ? "(up-to) " : "") << nbest
                                                                 << "-best.");
        if (nbest == 1) {
          visitor(r, r->weight<Weight>(), (NbestId)0);  // we don't use saved best cost, because that may be a summary
          // of e.g. FeatureWeightTpl feature vector
          if (nVisited) *nVisited = 1;
          return r;
        }
      }

      // n>1 best:
      typedef graehl::copy_filter_factory<Filter> FF;
      FF ff(filter);
      nstat = BuildLazy<FF>(hg, N, ff, mu, pi).enumerate_kbest(nbest, binvisitor);
      SDL_DEBUG(Hypergraph.BestPath, "(N>1)-best stats: " << nstat);
      if (nVisited) *nVisited = nstat.n_visited;
      return r;
    }

    template <class DerivVisitor>
    DerivationPtr visit_nbest(NbestPlusTies nbest, DerivVisitor const& visitor, bool throwEmptySetException = false,
                       NbestId* nVisited = 0) {
      NbestId maxn = nbest.maxnbest();
      if (opt.noFilterNeeded(nbest.nbest))
        return visit_nbestFilter(maxn, visitPlusTies(visitor, nbest.nbest), noFilter(),
                                 throwEmptySetException, nVisited);
      else
        return visit_nbestFilter(maxn, visitPlusTies(visitor, nbest.nbest), filter(), throwEmptySetException,
                                 nVisited);
    }

    /**
       return 1best or null derivation pointer if there is none.
    */
    DerivationPtr best(bool throwEmptySetException = false, bool onlyBestPathIf1best = false) {
      IgnoreVisitor v;
      return visit_nbestFilter(1, v, noFilter(), throwEmptySetException, 0, onlyBestPathIf1best);
    }

    // if there's a best path, return preecessor property map (best ArcHandle - untyped Arc *) per head
    Pi predecessors() { return best(true) ? pi : Pi(); }

    struct SaveLastVisitor {
      SaveLastVisitor(SaveLastVisitor const& o) : last(o.last) {}
      SaveLastVisitor(DerivAndWeight& last) : last(last) {}
      DerivAndWeight& last;
      bool operator()(DerivationPtr const& d, Weight const& w, NbestId) const {
        last = DerivAndWeight(d, w);
        return true;
      }
    };

    /// mostly for testing - tuple<DerivationPtr, Weight>
    DerivAndWeight bestWeighted(bool throwEmptySetException = false) {
      DerivAndWeight ret;
      SaveLastVisitor v(ret);
      visit_nbestFilter(1, v, noFilter(), throwEmptySetException);
      return ret;
    }

    DerivationPtr derivation(StateId head) const {
      BuildDerivation d(hg, pi);
      return d.derivation(head);
    }

    /// with a little effort, we use a more space-efficient stack for very deep trees (instead of recursion)
    struct BuildDerivation {
      HG const& hg_;
      Pi pi_;  // Pi = best-predecessor property map (Arc *) - lightweight copy
      std::vector<DerivationPtr> deriv_;  // memoize to prevent infinite loop in computing a loopy best-derivation,
      // and create a linear, rather than exponential, size derivation for pathological CFGs like:
      // S->T T; T->U U ; .... ; Z -> z
      BuildDerivation(HG const& hg, Pi pi) : hg_(hg), pi_(pi), deriv_(hg.sizeForHeads()) {}
      typedef std::pair<StateId, DerivationPtr*> StateResult;
      std::stack<StateResult> agenda;
      DerivationPtr derivation(StateId head) {
        if (hg_.isAxiom(head))
          return Derivation::kAxiom;
        else {
          DerivationPtr& d = deriv_[head];
          agenda.push(StateResult(head, &d));
          while (!agenda.empty()) buildOneNode();
          return d;
        }
      }

     private:
      void buildOneNode() {
        StateResult r = agenda.top();
        agenda.pop();
        StateId head = r.first;
        assert(!hg_.isAxiom(head));
        DerivationPtr& d = deriv_[head];
        if (!d) {
          Arc* headBestArc = (Arc*)get(pi_, head);
          if (!headBestArc)
            d = Derivation::kAxiom;
          else {
            StateIdContainer const& tails = headBestArc->tails();
            unsigned t = 0, nTails = (unsigned)tails.size();
            d = Derivation::construct(headBestArc, nTails);
            DerivChildren& derivch = d->children;
            t = nTails;
            while (t) {  // any order is fine
              --t;
              StateId tail = tails[t];
              if (hg_.isAxiom(tail))
                derivch[t] = Derivation::kAxiom;
              else
                agenda.push(StateResult(tails[t], &derivch[t]));
            }
          }
        }
        *r.second = d;
      }
    };  // end BuildDerivation

  };  // end Compute


  template <class Visitor, class Arc>
  DerivationPtr visit_nbest_get_1best(Visitor const& visitor, NbestPlusTies nbest, IHypergraph<Arc> const& hg,
                                      bool throwEmptySetException, NbestId* nVisited = 0) const {
    Compute<Arc> cb(opt, hg);
    return cb.visit_nbest(nbest, visitor, throwEmptySetException, nVisited);
  }

  // returns # visited
  template <class Visitor, class Arc>
  NbestId visit_nbest(Visitor const& visitor, NbestPlusTies nbest, IHypergraph<Arc> const& hg,
                      bool throwEmptySetException) const {
    NbestId nVisited = 0;
    visit_nbest_get_1best(visitor, nbest, hg, throwEmptySetException, &nVisited);
    return nVisited;
  }

  // returns 1best deriv
  template <class Out, class Arc>
  DerivationPtr out_nbest(Out& out, NbestPlusTies nbest, PathOutOptions const& popt,
                          IHypergraph<Arc> const& hg, bool throwEmptySetException) const {
    Compute<Arc> cb(opt, hg);
    NbestId nVisited = 0;
    DerivationPtr r
        = cb.visit_nbest(nbest, popt.pathOutNbestVisitor(out, hg), throwEmptySetException, &nVisited);
    popt.pad(out, nVisited, nbest.maxnbest());
    return r;
  }
};

template <class Visitor1, class Visitor2>
struct VisitBoth {
  Visitor1 const& visitor1;
  Visitor2 const& visitor2;
  VisitBoth(Visitor1 const& visitor1, Visitor2 const& visitor2) : visitor1(visitor1), visitor2(visitor2) {}
  template <class Weight>
  bool operator()(DerivationPtr const& deriv, Weight const& wtotal, NbestId nbest) const {
    return visitor1(deriv, wtotal, nbest) && visitor2(deriv, wtotal, nbest);
  }
};

template <class Visitor1, class Visitor2>
VisitBoth<Visitor1, Visitor2> visitBoth(Visitor1 const& visitor1, Visitor2 const& visitor2) {
  return VisitBoth<Visitor1, Visitor2>(visitor1, visitor2);
}

struct NbestPathOptions : NbestPlusTies, BestPathOptions {
  NbestPathOptions() { Config::inits(this); }
  template <class Config>
  void configure(Config& config) {
    config.is("NbestPathOptions");
    BestPathOptions::configure(config);
    NbestPlusTies::configure(config);
  }
  /// return number of best visited. reminder: visitor returns false if you want nbest visiting to stop before
  /// nbest
  template <class Arc, class Visitor>
  NbestId visit_nbest(IHypergraph<Arc> const& hg, Visitor const& visitor,
                      bool throwEmptySetException = false) const {
    return BestPath(*this).visit_nbest(visitor, *this, hg, throwEmptySetException);
  }
};

struct NbestHypergraphOptions : NbestPathOptions {
  bool nbestHypergraph;
  bool pushWeightValueToStart;
  bool epsilonStart, epsilonFinal;

  NbestHypergraphOptions(bool nbestHypergraphDefault = true) {
    nbestHypergraph = nbestHypergraphDefault;
    pushWeightValueToStart = true;
    epsilonStart = true;
    epsilonFinal = false;
  }

  template <class Conf>
  void configure(Conf& c) {
    NbestPathOptions::configure(c);
    c("nbest-hypergraph", &nbestHypergraph)
        .self_init()("output nbest hypergraph instead of original hypergraph");
    c("push-weight-value-to-start", &pushWeightValueToStart)
        .self_init()(
            "put total cost on first arc from start (so start state out arcs are sorted by increasing "
            "value)");
    c("epsilon-start", &epsilonStart)
        .self_init()(
            "use an epsilon arc leaving start state such that a state u adjacent to start identifies a path. "
            "the push-weight-value-to-start weight will be on the arc leaving u and not the new epsilon arc "
            "leaving start");
    c("epsilon-final", &epsilonFinal)
        .self_init()(
            "add an epsilon arc to final state such that a state v adjacent to final identifies a path");
  }

  template <class Arc>
  struct AcceptToNbestHg {
    typedef typename Arc::Weight Weight;

    AcceptToNbestHg(IHypergraph<Arc> const& in, IMutableHypergraph<Arc>& out,
                    NbestHypergraphOptions const& outOpt)
        : in(in)
        , out(dynamic_cast<MutableHypergraph<Arc>&>(out))  // TODO: why does this cast help?
        , outOpt(outOpt)
        , finalIn(in.final())
        , graph(in.isGraph()) {
      out.setEmpty();
      out.setVocabulary(in.getVocabulary());
      out.forceProperties(kStoreFirstTailOutArcs | kCanonicalLex);
      start = out.addState();
      final = out.addState();
      out.setStart(start);
      out.setFinal(final);
    }

    /// for visit_nbest
    bool operator()(DerivationPtr const& d, Weight const& wtotal, NbestId nbest) const {
      typename Weight::FloatT thisCost = wtotal.getValue();
      valueTotal = wtotal.getValue();
      if (d->axiom()) {  // empty output string
        out.addArcFsa(start, final, EPSILON::ID, wtotal);
      } else {
        weightSince = Weight::one();
        if (outOpt.epsilonStart) {
          lastHead = out.addState();
          added = out.addArcFsa(start, lastHead, EPSILON::ID);
        } else
          lastHead = start;
        added = 0;
        d->visitTree(*this, finalIn);
        if (outOpt.epsilonFinal && added) {
          StateId preFinal = out.addState();
          added->setHead(preFinal);
          out.addArcFsa(preFinal, final, EPSILON::ID);
        } else if (!added)  // empty output string
          out.addArcFsa(start, final, EPSILON::ID, wtotal);
      }
      return true;
    }

    /// for Derivation::visitTree: return true to expand children
    bool open(StateId head, Derivation const& d) const {
      if (!graph)
        takeWeight(d.a);
      else if (d.a) {
        if (d.a->getNumTails() < 1) takeWeight(d.a);
      }
      return true;
    }
    void child(Derivation const& p, Derivation const& c, TailId i) const {
      if (graph && i == 1) takeWeight(p.a);
    }
    void close(StateId s, Derivation const& d) const {
      LabelPair const labelPair = in.labelPair(s);
      if (labelPair.first.isTerminal()) {
        if (added) {
          added->setHead(
              (lastHead
               = out.addState()));  // setHead after add is ok because we have kStoreFirstTailOutArcs only
          if (outOpt.pushWeightValueToStart)
            setOneValue(weightSince);  // because we already put the total value on the first w
        } else if (outOpt.pushWeightValueToStart)
          weightSince.value_ = valueTotal;
        added = out.addArcFst(lastHead, final, labelPair,
                              weightSince);  // we'll change 'final' later if this wasn't the last one
        setOne(weightSince);
      }
    }

   private:
    IHypergraph<Arc> const& in;
    MutableHypergraph<Arc>& out;
    NbestHypergraphOptions const& outOpt;
    StateId start, final, finalIn;
    bool const graph;
    mutable StateId lastHead;
    mutable Weight weightSince;
    mutable typename Weight::FloatT valueTotal;
    mutable Arc* added;
    void takeWeight(ArcBase* arc) const {
      if (arc) timesBy(arc->weight<Weight>(), weightSince);
    }
  };

  template <class Arc>
  NbestId nbestHg(Hypergraph::IHypergraph<Arc> const& hg, Hypergraph::IMutableHypergraph<Arc>* outhg,
                  bool throwEmptySetException = false) const {
    //    BestPath::Compute<Arc> cb(*this, hg);
    //    AcceptToNbestHg<Arc> accept(hg, *outhg, *this);
    //    cb.visit_nbest(nbest, accept, throwEmptySetException, &nVisited);
    return visit_nbest(hg, AcceptToNbestHg<Arc>(hg, *outhg, *this), throwEmptySetException);
  }
};

struct BestPathOutOptions : PathOutOptions, NbestPathOptions {
  template <class Conf>
  void configure(Conf& c) {
    PathOutOptions::configure(c);
    NbestPathOptions::configure(c);
    c.is("best path(s) output");
    c("passes an input hypergraph unchanged, printing its best path(s)");
  }
  template <class Arc>
  DerivationPtr out_nbest(std::ostream& out, IHypergraph<Arc> const& hg) const {
    return BestPath(static_cast<BestPathOptions const&>(*this))
        .out_nbest(out, *this, static_cast<PathOutOptions const&>(*this), hg, !print_empty);
  }
};

/**
   for research use, provide the option to output file.id when translating
   multiple segments.

   options for stdout:

   StringConsumer with lock around ostream - atomic string output. mixing of nbest
   lines isn't that bad if they all have id

   module with vector<string> output (or just string for stopgap) - then xmt shell
   can handle ordering. but loses ability to insert in mid pipeline
*/
struct BestPathOutToOptions : NbestHypergraphOptions, PathOutOptions {

  BestPathOutToOptions(bool nbestHypergraphDefault = true) : NbestHypergraphOptions(nbestHypergraphDefault) {}
  template <class Arc>
  DerivationPtr out_nbest(std::ostream& out, IHypergraph<Arc> const& hg) const {
    return BestPath(static_cast<BestPathOptions const&>(*this))
        .out_nbest(out, *this, static_cast<PathOutOptions const&>(*this), hg, !print_empty);
  }

  std::string outFilePrefix;
  bool addNewline;  // if false, then the string Best module provides to pipeline has no newline after the
  // final nbest.
  template <class Conf>
  void configure(Conf& c) {
    PathOutOptions::configure(c);
    NbestHypergraphOptions::configure(c);
    c("out-file-prefix", &outFilePrefix)(Util::Output::help() + " for best-path output lines").init("-0");
    c("add-newline", &addNewline)("add blank line after the nbest lines - useful if ids aren't used.")
        .init(false);
  }

  bool enabledPerId() const { return !outFilePrefix.empty() && outFilePrefix != "-0"; }

  template <class Arc>
  NbestId outputForId(IHypergraph<Arc> const& hg, std::string const& id = "",
                      shared_ptr<IHypergraph<Arc> >* pOutNbestHg = 0, bool throwEmptySetException = false) {
    bool perid = enabledPerId();
    bool nbesthg = nbestHypergraph && pOutNbestHg;
    NbestId nVisited = 0;
    if (perid || nbesthg) {
      BestPath::Compute<Arc> cb(*this, hg);
      if (nbesthg) {
        MutableHypergraph<Arc>* outhg = new MutableHypergraph<Arc>;
        pOutNbestHg->reset(outhg);
        AcceptToNbestHg<Arc> accept(hg, *outhg, *this);
        if (perid) {
          Util::Output out(outFilePrefix, id);
          cb.visit_nbest(nbest, visitBoth(pathOutNbestVisitor(*out, hg), accept), throwEmptySetException,
                         &nVisited);
        } else
          cb.visit_nbest(nbest, accept, throwEmptySetException, &nVisited);
        if (nVisited == 0) outhg->setEmpty();
      } else {
        Util::Output out(outFilePrefix, id);
        cb.visit_nbest(nbest, pathOutNbestVisitor(*out, hg), throwEmptySetException, &nVisited);
      }
    }
    return nVisited;
  }

  /**
     write (optionally) both per-segment nbest output and store (always) in
     outLines the same string that was written, less the final newline.
  */
  template <class Arc>
  void outputForId(IHypergraph<Arc> const& hg, std::ostream& outStream, std::string const& id = "") {
    if (enabledPerId()) {
      Util::Output fileOut(outFilePrefix, id);
      graehl::teebuf teeStreambuf(fileOut->rdbuf(), outStream.rdbuf());
      std::ostream outBoth(&teeStreambuf);
      out_nbest(outBoth, hg);
      if (addNewline) *fileOut << '\n';
    } else
      out_nbest(outStream, hg);
  }

  template <class Arc>
  void outputForId(IHypergraph<Arc> const& hg, std::string& outLines, std::string const& id = "") {
    std::stringstream stringOut;
    outputForId(hg, stringOut, id);
    outLines = stringOut.str();
    if (outLines.empty()) {  // the handling of addNewline is a bit tricky
      // because we're taking into account xmt shell
      // adding an extra newline after the string we
      // return
      if (addNewline) outLines += '\n';
    } else if (!addNewline)
      outLines.erase(outLines.end() - 1, outLines.end());
  }
};

struct MaybeBestPathOutOptions : BestPathOutToOptions {
  template <class Arc>
  struct TransformFor {
    /// this is a hg -> vector<deriv> transform, not hg->hg (so output type is a dummy)
    typedef BestPath type;
  };

  static inline std::string usage() { return BestPathOptions::usage(); }
  static char const* caption() { return "nbest output and computation"; }
  static char const* type() { return "Best"; }

  void disableNbest() {
    enable = false;
    nbestHypergraph = false;
  }

  bool enable;
  MaybeBestPathOutOptions(bool nbestHypergraphDefault = true)
      : BestPathOutToOptions(nbestHypergraphDefault), enable(false) {}

  template <class Conf>
  void configure(Conf& c) {
    BestPathOutToOptions::configure(c);
    c("enable-best", &enable)("print best path(s) if enabled; otherwise print hypergraph instead").self_init();
  }

  template <class Arc>
  void output(std::ostream& o, IHypergraph<Arc> const& hg, std::string const& id = "") {
    if (enable)
      outputForId(hg, o, id);
    else if (nbestHypergraph) {
      shared_ptr<IHypergraph<Arc> > nbestHg;
      outputForId(hg, id, &nbestHg);
      o << *nbestHg;
    } else
      o << hg;
  }
};

template <class Arc>
bool hasTrivialBestPathInArcsRec(IHypergraph<Arc> const& hg, StateId final, StateId start) {
  assert(final != start);
  assert(hg.storesInArcs());
  if (hg.numInArcs(final) == 1) {
    Arc* arc = hg.inArc(final, 0);
    StateIdContainer const& tails = arc->tails();
    if (tails.size() == 1 && tails[0] == start && hg.hasLexicalLabel(final))
      // hack for syntax-based labels-on-preterminals // TODO: CM-450 then just check hg.isAxiom(final)
      return true;
    for (TailId i = 0, n = tails.size(); i < n; ++i) {
      StateId const tail = tails[i];
      if (tail == final) {
        SDL_WARN(BestPath.computeTrivialBestPath,
                 "only in-arc to state " << final << " is a self-loop (has head in tail #" << i << ")");
        return false;
      }
      if (tail != start && !hasTrivialBestPathInArcsRec(hg, tail, start)) return false;
    }
    return true;
  } else
    return hg.hasLexicalLabel(final);
}


/**
   \return whether computeTrivialBestPath will return a derivation.
*/
template <class Arc>
bool hasTrivialBestPath(IHypergraph<Arc> const& hg, StateId final, StateId start) {
  if (final == kNoState) return false;
  bool const atStart = final == start;
  if (atStart) return true;
  if (!hg.storesInArcs()) {
    if (!hg.isGraph() || start == kNoState)
      SDL_THROW_LOG(Hypergraph.BestPath, ConfigException,
                    "can't check for trivial best path of non-graph without in-arcs: " << withProperties(hg));
    for (;;) {
      if (hg.numOutArcs(start) == 1) {
        StateId checkloop = start;
        start = hg.outArc(start, 0)->head_;
        if (start == final) {
          SDL_TRACE(Hypergraph.BestPath.computeTrivialBestPath, "found graph-out-arc trivial best-path to "
                                                                    << final);
          return true;
        }
        if (start == checkloop) {
          SDL_WARN(Hypergraph.BestPath.computeTrivialBestPath, "only graph-out-arc from state "
                                                                   << checkloop << " is a self-loop");
          return false;
        }
      } else
        return false;
    }
  } else
    return hasTrivialBestPathInArcsRec(hg, final, start);
  return false;
}

/**
   \return NULL if the inarcs into final aren't a tree w/ axiom state leaves, else return the derivation.

   \param w if non-NULL gets the weight of the derivation tree multiplied into
   it (so caller should start w/ w=one)

   doesn't protect against infinite loops from a cycle (except the trivial self-loop)
*/
template <class Arc>
DerivationPtr computeTrivialBestPath(IHypergraph<Arc> const& hg, StateId final, typename Arc::Weight* w,
                                     StateId start) {
  // SDL_TRACE(BestPath.computeTrivialBestPath, "checking for trivial best path in " << hg);
  if (!hg.storesInArcs()) {
    if (!hg.isGraph() || start == kNoState)
      SDL_THROW_LOG(Hypergraph.BestPath, ConfigException,
                    "can't compute trivial best path of non-graph without in-arcs: " << withProperties(hg));

    DerivationPtr r(Derivation::kAxiom);
    for (;;) {
      if (start == final) {
        SDL_TRACE(Hypergraph.BestPath.computeTrivialBestPath, "found graph-out-arc trivial best-path to "
                                                                  << final << ": " << Util::print(r, hg));
        return r;
      }
      if (hg.numOutArcs(start) == 1) {
        Arc* arc = hg.outArc(start, 0);
        if (w) timesBy(arc->weight(), *w);
        r = Derivation::construct(arc, r);
        start = arc->head_;
        SDL_TRACE(Hypergraph.BestPath.computeTrivialBestPath, "taking unambiguous graph-out-arc step "
                                                                  << arcPrint(arc, hg));
      } else
        break;
    }
  } else {
    if (final != kNoState) {
      bool const atStart = final == start;
      if (!atStart && hg.numInArcs(final) == 1) {
        Arc* arc = hg.inArc(final, 0);
        if (w) timesBy(arc->weight(), *w);
        StateIdContainer const& tails = arc->tails();
        if (tails.size() == 1 && tails[0] == start && hg.hasLexicalLabel(final))
          // hack for syntax-based labels-on-preterminals - once CM-450 is fixed we can simply favor axiom
          // case
          return Derivation::kAxiom;
        DerivationPtr r(new Derivation(arc, tails.size()));
        DerivationChildren& children = r->children;
        for (TailId i = 0, n = tails.size(); i < n; ++i) {
          StateId const src = tails[i];
          if (src == final) {
            SDL_WARN(BestPath.computeTrivialBestPath,
                     "only in-arc to state " << final << " is a self-loop (has head in tail #" << i << ")");
            goto none;
          }
          if (!(children[i] = computeTrivialBestPath(hg, src, w, start))) goto none;
        }
        return r;
      } else if (atStart || hg.hasLexicalLabel(final))
        // this is else rather than first check to work around Preorderer (CM-450)
        // placing terminals in non-leaf position
        return Derivation::kAxiom;
    }
  none:
    SDL_TRACE(Hypergraph.BestPath.computeTrivialBestPath, "no single-in-arc derivation for state "
                                                              << final << " in hg: " << hg);
  }
  if (w) setZero(*w);
  return DerivationPtr();
}

template <class Arc>
DerivationPtr computeTrivialGraphBestPathOutArcs(DerivationPtr const& prefix, IHypergraph<Arc> const& hg,
                                                 StateId final, typename Arc::Weight* w, StateId start) {
  if (start != kNoState) {
    if (hg.numOutArcs(start) == 1) {
      Arc* arc = hg.outArc(start, 0);
      if (w) timesBy(arc->weight(), *w);
      TailId const ntails = arc->tails().size();
      DerivationPtr succ(new Derivation(arc, ntails));
      DerivationChildren& children = succ->children;
      children[0] = prefix;
      std::fill(children.begin() + 1, children.end(), Derivation::kAxiom);
      StateId const next = arc->head();
      if (next == final)
        return succ;
      else
        return computeTrivialGraphBestPathOutArcs(succ, hg, final, w, next);
    }
  }
  return DerivationPtr();
}

template <class Arc>
Derivation::DerivAndWeight<Arc> computeBestDerivWeight(IHypergraph<Arc> const& hg,
                                                       BestPathOptions const& opt = BestPathOptions(),
                                                       bool throwEmptySetException = false) {
  BestPath::Compute<Arc> cb(opt, hg);
  return cb.bestWeighted(throwEmptySetException);
}

template <class Arc>
DerivationPtr computeBestPath(IHypergraph<Arc> const& hg, BestPathOptions const& opt = BestPathOptions(),
                              bool throwEmptySetException = false) {
  BestPath::Compute<Arc> cb(opt, hg);
  return cb.best(throwEmptySetException);
}

/// when hg is already a single derivation hg (final state and all reachable
/// states have 0 or 1 inarcs), return that derivation. else return 0
template <class Arc>
DerivationPtr trivialBestPath(IHypergraph<Arc> const& hg, bool throwEmptySetException = false,
                              typename Arc::Weight* w = 0) {
  StateId final = hg.final();
  if (final == kNoState) {
    maybeThrowEmptySetException(throwEmptySetException);
    if (w) setZero(*w);
  } else if (hg.storesInArcs()) {
    SDL_TRACE(verbose.Hypergraph.BestPath.trivialBestPath,
              "checking for single-in-arc derivation to FINAL:" << final << " in hg: " << hg);
    DerivationPtr const& r = computeTrivialBestPath(hg, final, w, hg.start());
    if (r) return r;
  } else if (hg.isGraph() && hg.storesOutArcs()) {
    DerivationPtr r(Derivation::kAxiom);
    r = computeTrivialGraphBestPathOutArcs(r, hg, final, w, hg.start());
    if (r) return r;
  }

  return DerivationPtr();
}

template <class Arc>
DerivationPtr bestPath(IHypergraph<Arc> const& hg, BestPathOptions const& opt = BestPathOptions(),
                       bool throwEmptySetException = false, typename Arc::Weight* w = 0) {
  if (w) setOne(*w);
  DerivationPtr const& r = trivialBestPath(hg, throwEmptySetException, w);
  if (r)
    return r;
  else if (w) {
    Derivation::DerivAndWeight<Arc> const& dw = computeBestDerivWeight(hg, opt, throwEmptySetException);
    *w = weight(dw);
    return derivP(dw);
  } else
    return computeBestPath(hg, opt, throwEmptySetException);
}

template <class Arc>
Derivation::DerivAndWeight<Arc> bestDerivWeight(IHypergraph<Arc> const& hg,
                                                BestPathOptions const& opt = BestPathOptions(),
                                                bool throwEmptySetException = false) {
  Derivation::DerivAndWeight<Arc> r;
  r.deriv = bestPath(hg, opt, throwEmptySetException, &r.weight);
  return r;
}

template <class V, class Arc>
DerivationPtr visitNbest(V const& v, NbestPlusTies n, IHypergraph<Arc> const& hg,
                         BestPathOptions const& opt = BestPathOptions(), bool throwEmptySetException = false,
                         NbestId* nVisited = 0) {
  if (n.nbest == 1) {
    typename Arc::Weight w;
    DerivationPtr const& r = bestPath(hg, opt, throwEmptySetException, &w);  // no sequence point for fn params
    if (r) v(r, w, kFirstNbestId);
    if (nVisited) *nVisited = (bool)r;
    return r;
  } else {
    return BestPath(opt).visit_nbest_get_1best(v, n, hg, throwEmptySetException, nVisited);
  }
}

template <class Arc>
std::vector<Derivation::DerivAndWeight<Arc> > getNbest(NbestId n, IHypergraph<Arc> const& hg,
                                                       BestPathOptions const& opt = BestPathOptions(),
                                                       bool throwEmptySetException = false) {
  std::vector<Derivation::DerivAndWeight<Arc> > derivs;
  derivs.reserve(n);
  visitNbest(HoldNbestDerivAndWeights<Arc>(derivs), n, hg, opt, throwEmptySetException);
  return derivs;
}

template <class Arc>
Derivation::DerivAndWeight<Arc> get1best(IHypergraph<Arc> const& hg,
                                         BestPathOptions const& opt = BestPathOptions(),
                                         bool throwEmptySetException = false) {
  std::vector<Derivation::DerivAndWeight<Arc> > derivs;
  visitNbest(HoldNbestDerivAndWeights<Arc>(derivs), 1, hg, opt, throwEmptySetException);
  return derivs.front();
}

template <class Arc>
DerivationPtr bestPath(IHypergraph<Arc> const& hg, bool throwEmptySetException) {
  return bestPath(hg, BestPathOptions(), throwEmptySetException);
}

/**
   sentinel values; would be statically initialized but this is clearer
   and lets us use POD. (compound literals aren't part of C++98 but are of
   C99 and C++11)
*/
template <class Arc>
typename BestPath::Compute<Arc>::BinaryDerivation BestPath::Compute<Arc>::none;
template <class Arc>
typename BestPath::Compute<Arc>::BinaryDerivation BestPath::Compute<Arc>::pending;


}}

#endif
