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

    used to treat a LogWeight as a ViterbiWeight or vice versa (so
    FeatureExpectations can use the right insideAlgorithm semiring)
*/

#ifndef HYP__HYPERGRAPH_CAST_HYPERGRAPH_HPP
#define HYP__HYPERGRAPH_CAST_HYPERGRAPH_HPP
#pragma once

#include <sdl/SharedPtr.hpp>
#include <sdl/Hypergraph/IHypergraph.hpp>

#include <sdl/Util/LogHelper.hpp>
#include <sdl/IVocabulary.hpp>

namespace sdl {
namespace Hypergraph {

/**
   Casts each arc on demand to a different arc type. Arc types
   must be convertible using reinterpret_cast (true in typical
   cases). This is the most efficient way to use a hypergraph with a
   different semiring (e.g., summing instead of max'ing by casting
   from ViterbiArc to LogArc).
*/
template <class FromA, class ToA>
class CastHypergraph final : public IHypergraph<ToA> {

 public:
  typedef FromA FromArc;
  typedef ToA ToArc;

  CastHypergraph(IHypergraph<FromA> const& hg) : hg_(hg) {
    this->start_ = hg.start();
    this->final_ = hg.final();
  }

  Properties uncomputedProperties() const override { return hg_.uncomputedProperties(); }

  CastHypergraph<FromArc, ToArc>* clone() const override {
    return new CastHypergraph<FromArc, ToArc>(hg_);
  }

  /**
     Casts the arc using reinterpret_cast and returns it.
  */
  ToArc* inArc(StateId s, ArcId arcid) const override {
    return reinterpret_cast<ToArc*>(hg_.inArc(s, arcid));
  }

  /**
     Casts the arc using reinterpret_cast and returns it.
  */
  ToArc* outArc(StateId s, ArcId arcid) const override {
    return reinterpret_cast<ToArc*>(hg_.outArc(s, arcid));
  }

  // Simple delegation:
  ArcId numInArcs(StateId s) const override { return hg_.numInArcs(s); }
  ArcId numOutArcs(StateId s) const override { return hg_.numOutArcs(s); }
  StateId size() const override { return hg_.size(); }
  Sym inputLabel(StateId s) const override { return hg_.inputLabel(s); }
  Sym outputLabel(StateId s) const override { return hg_.outputLabel(s); }
  Properties properties() const override { return hg_.properties(); }
  IVocabularyPtr getVocabulary() const override { return hg_.getVocabulary(); }

  /** since arc mapping doesn't affect state labels, defer. */
  bool outputLabelFollowsInput(StateId s) const override { return hg_.outputLabelFollowsInput(s); }

  bool outputLabelFollowsInput() const override { return hg_.outputLabelFollowsInput(); }

  IHypergraph<FromArc> const& hg_;
};


}}

#endif
