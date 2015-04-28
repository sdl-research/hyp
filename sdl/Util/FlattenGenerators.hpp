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

    premise: you have a generator of generators and want a lazy approximately best-first flat generator.

    the top-level generators are presumed ordered best-first according to the cost of their offspring

    the second-level generators are also presumed ordered best-first

    it's like a comb:

    -------------------- (base)
    | | | ..............
    | | (tip)
    |
    (tip)

    the algorithm is a lazy sorted merge

    it's used in finite state lazy composition. note that we don't group
    input-fsm arcs having the same output; we simply take them one at a time
    best-first. so the neighboring columns in the comb don't necessarily have
    the same length or cost-slope.
*/

#ifndef FLATTENGENERATORS_JG20121222_HPP
#define FLATTENGENERATORS_JG20121222_HPP
#pragma once

#include <sdl/Util/Generator.hpp>
#include <sdl/SharedPtr.hpp>
#include <sdl/Util/RefCount.hpp>
#include <sdl/Util/PriorityQueue.hpp>
#include <boost/property_map/property_map.hpp>
#include <sdl/Pool/object_pool.hpp>

namespace sdl { namespace Util {

template <class GenGen, class WeightFn
          , class Result = typename GenGen::result_type::result_type
          , class WeightT = typename WeightFn::result_type
          >
struct FlattenGenerators : WeightFn
    , GeneratorTraits<FlattenGenerators<GenGen, WeightFn, Result, WeightT>, Result, PeekableT>
    , intrusive_refcount<FlattenGenerators<GenGen, WeightFn, Result, WeightT>, RefCount>
{
  typedef boost::intrusive_ptr<FlattenGenerators> Ptr;
  GenGen gengen;
  typedef typename GenGen::result_type Generator;
  typedef typename Generator::reference reference;
  typedef typename Generator::value_type value_type;
  typedef Result result_type;
  typedef WeightT Weight;
  struct Tip {
    bool base; // base==0 means it's not on base
    Generator gen;
    value_type val;
    Weight weight; //TODO: object pool
    bool operator<(Tip const& other) const {
      return weight < other.weight;
    }
  };
  struct TipPriorityPmap {
    typedef boost::readable_property_map_tag category;
    typedef Weight value_type;
    typedef value_type const& reference;
    typedef Tip* key_type;
    friend inline reference get(TipPriorityPmap const&, key_type pTip) {
      return pTip->weight;
    }
  };
  typedef Pool::object_pool<Tip> TipPool;  // Generator, Weight may not be pod, so we use object_pool not pool
  typedef shared_ptr<TipPool> TipPoolPtr;
  TipPoolPtr pTipPool;
  FlattenGenerators(GenGen const& gengen, WeightFn const& weightFn
                    , TipPoolPtr pTipPool =
#ifdef _MSC_VER
                    TipPoolPtr(new TipPool())
#else
                    // this doesn't compile in msvc
                    make_shared<TipPool>()
#endif
                    )
      : WeightFn(weightFn)
      , gengen(gengen)
      , pTipPool(pTipPool)
  {
    init();
  }
  friend inline std::ostream& operator<<(std::ostream &out,
                                         FlattenGenerators const& self) {
    out << "Flatten[#queued=" << self.queue.size() << "]";
    return out;
  }

  typedef Util::priority_queue<std::vector<Tip*>, 4, TipPriorityPmap> Queue;
  Queue queue;
  operator bool() const {
    return !queue.empty();
  }
  /**
     seed queue with the first (base position) tip.
  */
  void init(GenGen const& gengen_) {
    gengen = gengen_;
    init();
  }
  //TODO: make non-peekable to simplify?
  value_type operator()() {
    value_type r((peek()));
    pop();
    return r;
  }
  /**
     returned values are only valid until next pop (make a copy if you need them).
  */
  value_type const& peek() const {
    assert(!queue.empty());
    return queue.top()->val;
  }
  void pop() {
    Tip &tip=*queue.top();
    bool wasBase = tip.base;
    if (wasBase)
      tip.base = false;
    if (tip.gen) {
      setTip(tip);
      queue.adjust_top();
    } else
      queue.pop();
    if (wasBase) {
      tip.base = false;
      pushBase();
    }
  }
 private:
  void init() {
    pushBase();
  }

  /**
     a new base element is needed after popping a tip in base position (we maintain at most one)
  */
  void pushBase() {
    Tip *pNewTip = pTipPool->construct();
    pNewTip->base = true;
    while (gengen) {
      pNewTip->gen = gengen();
      if (pNewTip->gen) {
        setTip(*pNewTip);
        queue.push(pNewTip);
        return;
      }
    }
    pTipPool->destroy(pNewTip);
  }
  bool nextNonemptyBase(Tip *pNewTip) {
    pNewTip->base = true;
    while (gengen) {
      pNewTip->gen = gengen();
      if (pNewTip->gen) {
        setTip(*pNewTip);
        return true;
      }
    }
    return false;
  }

  void setTip(Tip &tip) {
    tip.weight = WeightFn::operator()(tip.val = tip.gen());
  }
};


}}

#endif
