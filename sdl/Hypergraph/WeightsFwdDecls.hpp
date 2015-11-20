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

    These forward declarations for weights help avoid compile
    errors because, for example, plus() for one weight class may call
    plus() for another.

    JG concludes that this is a consequence of more-standards-compliant template/fn
    resolution - only things that were declared before may be found?

    also, you may find you need to refer to Hypergraph::plus if someone has using
    std::plus (most likely with 'using namespace std') - suspects are icu
    unicode/std_string.h, irstlm mfstream.h, srilm anything.h
*/

#ifndef HYP__HYPERGRAPH_WEIGHTS_FWD_HPP
#define HYP__HYPERGRAPH_WEIGHTS_FWD_HPP
#pragma once

#include <type_traits>

namespace sdl {
namespace Hypergraph {

struct BlockWeight;

class BooleanWeight;

class NoWeight;

template <class F>
class ViterbiWeightTpl;

template <class F>
class LogWeightTpl;

template <class W>
class NgramWeightTpl;

template <class W>
class TokenWeightTpl;

template <class F, class MapT, class SumT>
class FeatureWeightTpl;
/**
   SumT arguments.
*/
struct TakeMin;
struct Expectation;

template <class W1, class W2, class W3>
class CompositeWeight;

typedef std::true_type IsIdempotent;
typedef std::false_type NotIdempotent;

template <class Weight>
struct WeightIdempotentPlus : NotIdempotent {};

template <class F>
struct WeightIdempotentPlus<ViterbiWeightTpl<F>> : IsIdempotent {};

template <>
struct WeightIdempotentPlus<BooleanWeight> : IsIdempotent {};

template <class FloatT, class MapT>
struct WeightIdempotentPlus<FeatureWeightTpl<FloatT, MapT, TakeMin>> : IsIdempotent {};

// TODO: figure out whether any others are idempotent-plus

// plus/times

BlockWeight plus(BlockWeight const&, BlockWeight const&);
BlockWeight times(BlockWeight const&, BlockWeight const&);

inline BooleanWeight plus(BooleanWeight const&, BooleanWeight const&);
inline BooleanWeight times(BooleanWeight const&, BooleanWeight const&);

template <class F>
ViterbiWeightTpl<F> plus(ViterbiWeightTpl<F> const&, ViterbiWeightTpl<F> const&);
template <class F>
ViterbiWeightTpl<F> times(ViterbiWeightTpl<F> const&, ViterbiWeightTpl<F> const&);

template <class F>
LogWeightTpl<F> plus(LogWeightTpl<F> const&, LogWeightTpl<F> const&);
template <class F>
LogWeightTpl<F> times(LogWeightTpl<F> const&, LogWeightTpl<F> const&);

template <class W>
TokenWeightTpl<W> plus(TokenWeightTpl<W> const&, TokenWeightTpl<W> const&);
template <class W>
TokenWeightTpl<W> times(TokenWeightTpl<W> const&, TokenWeightTpl<W> const&);

template <class W>
NgramWeightTpl<W> plus(NgramWeightTpl<W> const&, NgramWeightTpl<W> const&);
template <class W>
NgramWeightTpl<W> times(NgramWeightTpl<W> const&, NgramWeightTpl<W> const&);

template <class FloatT, class MapT>
FeatureWeightTpl<FloatT, MapT, TakeMin> plus(FeatureWeightTpl<FloatT, MapT, TakeMin> const& w1,
                                             FeatureWeightTpl<FloatT, MapT, TakeMin> const& w2);

template <class FloatT, class MapT>
FeatureWeightTpl<FloatT, MapT, Expectation> plus(FeatureWeightTpl<FloatT, MapT, Expectation> const& w1,
                                                 FeatureWeightTpl<FloatT, MapT, Expectation> const& w2);

template <class FloatT, class MapT>
FeatureWeightTpl<FloatT, MapT, TakeMin> times(FeatureWeightTpl<FloatT, MapT, TakeMin> const& w1,
                                              FeatureWeightTpl<FloatT, MapT, TakeMin> const& w2);

template <class FloatT, class MapT>
FeatureWeightTpl<FloatT, MapT, Expectation> times(FeatureWeightTpl<FloatT, MapT, Expectation> const& w1,
                                                  FeatureWeightTpl<FloatT, MapT, Expectation> const& w2);

template <class FloatT, class MapT>
FeatureWeightTpl<FloatT, MapT, TakeMin> divide(FeatureWeightTpl<FloatT, MapT, TakeMin> const& w1,
                                               FeatureWeightTpl<FloatT, MapT, TakeMin> const& w2);

template <class FloatT, class MapT>
FeatureWeightTpl<FloatT, MapT, Expectation> divide(FeatureWeightTpl<FloatT, MapT, Expectation> const& w1,
                                                   FeatureWeightTpl<FloatT, MapT, Expectation> const& w2);

template <class W1, class W2, class W3>
CompositeWeight<W1, W2, W3> plus(CompositeWeight<W1, W2, W3> const&, CompositeWeight<W1, W2, W3> const&);
template <class W1, class W2, class W3>
CompositeWeight<W1, W2, W3> times(CompositeWeight<W1, W2, W3> const&, CompositeWeight<W1, W2, W3> const&);

// less is recommended by markus for use in best-path algs (graehl's solution
// was to invent a specializable traits class in HypergraphTraits.hpp, akin to
// what was already in use for graehl::lazy_forest_kbest), but this would be
// simpler.

inline bool less(BooleanWeight const&, BooleanWeight const&);
inline BlockWeight less(BlockWeight const&, BlockWeight const&);

template <class F>
bool less(ViterbiWeightTpl<F> const&, ViterbiWeightTpl<F> const&);
template <class F>
bool less(LogWeightTpl<F> const&, LogWeightTpl<F> const&);
template <class W>
bool less(NgramWeightTpl<W> const&, NgramWeightTpl<W> const&);
template <class W>
bool less(TokenWeightTpl<W> const&, TokenWeightTpl<W> const&);

template <class F, class MapT, class SumT>
bool less(FeatureWeightTpl<F, MapT, SumT> const&, FeatureWeightTpl<F, MapT, SumT> const&);

template <class W1, class W2, class W3>
bool less(CompositeWeight<W1, W2, W3> const&, CompositeWeight<W1, W2, W3> const&);


}}

#endif
