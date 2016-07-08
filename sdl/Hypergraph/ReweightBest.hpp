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

/* \file

   mostly for testing: improve (by epsilon ) weights of arcs used in best path -
   serving to make the 1best better than the rest.

   recommend first reweighting EVERYTHING using Reweight to make it epsilon
   worse first - to avoid negative cost cycles, or not change the score output
   for 1best

   TODO (unrelated): different type of reweighting: (well defined for no neg
   cycles?): derivation cost' = cost-cost(best) (sidetrack cost) - but can't use
   nbest visitor for that; need mu in BestPath computation.
*/

#ifndef REWEIGHTBEST_JG_2014_12_18_HPP
#define REWEIGHTBEST_JG_2014_12_18_HPP
#pragma once


#include <sdl/Hypergraph/BestPath.hpp>
#include <sdl/Hypergraph/Reweight.hpp>

namespace sdl {
namespace Hypergraph {

struct ReweightBest : ReweightOptions, SimpleTransform<ReweightBest, Transform::Inplace, false> {
  static char const* type() { return "ReweightBest"; }
  static inline std::string caption() {
    return std::string("(for best derivation arcs only) ") + ReweightOptions::caption();
  }
  template <class Arc>
  void inplace(IMutableHypergraph<Arc>& h) const;
};

// nbest visitor
template <class Arc>
struct ReweightDerivation {
  typedef typename Arc::Weight Weight;
  bool open(Derivation const& d) const { return true; }
  void child(Derivation const& p, Derivation const& c, TailId i) const {}
  void close(Derivation const& d) const {
    if (d.axiom()) return;
    opt.reweight(d.arcwt<Weight>(), rng);
  }

  ReweightBest const& opt;
  typedef ReweightDerivation self_type;
  IMutableHypergraph<Arc>& h;
  ReweightDerivation(IMutableHypergraph<Arc>& h, ReweightBest const& opt) : h(h), opt(opt), rng(opt.seed) {}
  mutable Util::Random01 rng;
  bool operator()(DerivationPtr const& dp, Weight const& w, NbestId n) const {
    SDL_DEBUG(Hypergraph.ReweightBest, "reweighting arcs in 1-best derivation " << printer(*dp, h));
    dp->visitDfs(*this);
    return true;
  }
};


template <class Arc>
void ReweightBest::inplace(IMutableHypergraph<Arc>& h) const {
  ReweightDerivation<Arc> v(h, *this);
  visitNbest(v, 1, h);
}


}}

#endif
