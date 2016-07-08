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

    integral and container typedefs.
*/

#ifndef HYP__HYPERGRAPH_TYPES_HPP
#define HYP__HYPERGRAPH_TYPES_HPP
#pragma once

#include <sdl/Util/BitSet.hpp>
#include <sdl/Features.hpp>
#include <sdl/Types.hpp>
#include <boost/static_assert.hpp>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <limits>
#include <map>
#include <utility>
#include <vector>

#ifndef SDL_64BIT_FEATURE_ID
#define SDL_64BIT_FEATURE_ID 0
#endif

#ifndef SDL_64BIT_STATE_ID
#define SDL_64BIT_STATE_ID 0
#endif

#ifndef SDL_64BIT_ARC_ID
#define SDL_64BIT_ARC_ID 0
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4244)
// try to include this before other headers
#include <boost/iterator/iterator_adaptor.hpp>
#endif
#include <boost/iterator/counting_iterator.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#include <sdl/SharedPtr.hpp>
#include <boost/config.hpp>
#include <boost/integer_traits.hpp>
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4018)
#pragma warning(disable : 4244)
#endif
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <sdl/Util/SmallVector.hpp>
#include <sdl/Util/Unordered.hpp>
#include <sdl/IVocabulary-fwd.hpp>
#include <sdl/Syms.hpp>
#include <boost/range/iterator_range.hpp>
#include <graehl/shared/is_null.hpp>

namespace sdl {
namespace Hypergraph {

struct HypergraphBase;

using sdl::FeatureId;

std::string featureIdRangeDescription(FeatureId begin, FeatureId end);

typedef std::size_t NbestId;
typedef uint32 TailId;

struct ArcBase;

typedef ArcBase* ArcHandle;

#if SDL_64BIT_STATE_ID
typedef uint64 StateId;
typedef int64 StateIdDifference;
#else
typedef uint32 StateId;
typedef int32 StateIdDifference;
#endif

typedef Util::small_vector<StateId, 2, TailId> StateIdContainer;
// only 2 tails inline: StateId is std::size_t=8 bytes. therefore
// sizeof(StateIdContainer) is 24 bytes. 2 tails is perfect for fsm, though

typedef std::vector<StateId> StateIds;

#if SDL_64BIT_ARC_ID
typedef uint64 ArcId;
#else
typedef uint32 ArcId;
#endif

typedef Util::small_vector<ArcHandle, 1, ArcId> ArcsContainer;

// typedef sdl::function<bool (StateId s)> StateFilter;
typedef std::vector<StateId> StateString;
typedef std::pair<StateId, ArcHandle> StateArc;  // for derivations: leaf states have Arc *==0.
typedef std::vector<StateArc> DerivationYield;

// boost/range/irange.hpp irange(0, size()) might be faster -
// boost::iterator_range<boost::range_detail::integer_iterator<Integer> >
typedef boost::counting_iterator<StateId> StateIdIterator;
typedef boost::counting_iterator<ArcId> ArcIdIterator;
typedef boost::counting_iterator<TailId> TailIdIterator;

// similar use w/ boost range lib as iterator_range
typedef boost::iterator_range<StateIdIterator> StateIdRange;
typedef boost::iterator_range<ArcIdIterator> ArcIdRange;
typedef boost::iterator_range<TailIdIterator> TailIdRange;

// TODO@MD from JG: i noticed some effort in IHypergraph to make these range
// types primary instead of an implicit [0...getNum*), in support of lazy hgs
//(e.g. not-dense stateid space - like nbest hypergraph?). if that's your
// intent, then you can't use counting_iterator; you'd have to use type-erased
// iterators, which would hurt performance.

typedef Util::BitSet StateSet;
static_assert(sizeof(StateSet::size_type) >= sizeof(StateId), "StateSet must hold StateIds");

static constexpr ArcId kNoArc = boost::integer_traits<ArcId>::const_max;
static constexpr StateId kNoState = boost::integer_traits<StateId>::const_max;
static constexpr StateId kImpossibleNotNoState = boost::integer_traits<StateId>::const_max - 1;

using ::set_null;

typedef std::pair<StateId, StateId> StateIdInterval;

inline StateId maxState(StateId a, StateId b) {
  if (a == kNoState) return b;
  if (b == kNoState) return a;
  return a > b ? a : b;
}

typedef SparseFeatures Features;
using sdl::SparseFeatureEntry;

typedef unordered_map<StateId, StateId> StateIdMap;

typedef SdlFloat Distance;


}}

#endif
