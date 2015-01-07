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

    function objects mapping Arc * -> Weight const& or Weight

*/

#ifndef ARCWEIGHT_JG20121029_HPP
#define ARCWEIGHT_JG20121029_HPP
#pragma once

#include <sdl/Util/Unordered.hpp>
#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Pool/object_pool.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/make_shared.hpp>
#include <boost/type_traits/remove_reference.hpp>

namespace sdl { namespace Hypergraph {

template <class Wt>
struct ArcWeight {
  typedef Wt Weight;
  template <class Arc>
  Wt const& operator()(Arc *a) const { return a->weight(); }
};

template <class Wt>
struct OneArcWeight {
  typedef Wt Weight;
  template <class Arc>
  Weight operator()(Arc *) const { return Weight::one(); }
};

template <class Wt>
struct ZeroArcWeight {
  typedef Wt Weight;
  template <class Arc>
  Weight operator()(Arc *) const { return Weight::zero(); }
};

template<class Wt>
struct StateToWeight {
  typedef Wt Weight;
  virtual ~StateToWeight() {}
  virtual Weight operator()(StateId sid) const = 0;
  void timesByState(StateId sid, Weight &w) const {
    timesBy((*this)(sid, w));
  }
};

// to pass an abstract StateToWeight (unknown concrete type) by value as a StateWtFn. Pimpl may be shared_ptr<StateToWeight<Wt> *> instead
template <class Wt, class Pimpl = StateToWeight<Wt> *>
struct CallStateToWeight {
  typedef Wt Weight;
  Pimpl p;
  Weight operator()(StateId sid) const {
    return (*p)(sid);
  }
  void timesByState(StateId sid, Weight &w) const {
    timesBy((*this)(sid, w));
  }

  CallStateToWeight() : p() {}
  template <class Ptr>
  CallStateToWeight(Ptr const& ptr)
      : p(ptr) {}
  CallStateToWeight(CallStateToWeight const& o)
      : p(o.p) {}
};


/**
   if your WtFn returns by ref, set Wt to that ref type. such is life without C++11
*/
template <class WtFn, class Wt = typename WtFn::Weight>
struct RefWtFn {
  WtFn *pimpl;
  typedef Wt Weight;
  template <class Arc>
  Wt operator()(Arc *a) const { return (*pimpl)(a); }
};

template <class WtFn>
RefWtFn<WtFn> refWtFn(WtFn &wf) {
  return RefWtFn<WtFn>(wf);
}

template <class WtFn>
RefWtFn<WtFn> refWtFnCref(WtFn &wf) {
  return RefWtFn<WtFn>(wf);
}

/**
   CRTP caching of an ArcWeight fn ; friendlier constructor forwarding. note: you won't get caching unless you access through a CachedArcWeight ref/ptr.

   "frozen" means it's an error to call on any arc that wasn't already evaluated earlier (e.g. with populate(hg))
*/
template <class Impl, class Wt = typename Impl::Weight>
struct CacheArcWeight
{
  Impl const& impl() const { return *static_cast<Impl const*>(this); }
  Impl & impl() { return *static_cast<Impl *>(this); }
  typedef Wt Weight;
  Pool::object_pool<Wt> wtPool;
  typedef typename boost::remove_reference<Wt>::type * CachedWt;
  unordered_map<intptr_t, CachedWt> cache;
  template <class Arc>
  Wt const& operator()(Arc *a) {
    CachedWt *pw;
    if (Util::update(cache, (intptr_t)a, pw))
      return *(*pw = wtPool.construct(impl()(a)));
    return **pw;
  };
  typedef CacheArcWeight<Impl, Wt> Self;
  typedef shared_ptr<Self> SharedPtr;
  SharedPtr sharedPtr() {
    return boost::static_pointer_cast<Self>(make_shared<Impl>(impl()));
  }
  struct Shared {
    typedef Wt Weight;
    SharedPtr p;
    template <class Arc>
    Wt const& operator()(Arc *a) const {
      return (*p)(a);
    }
  };
  struct Ptr {
    typedef Wt Weight;
    Self *p;
    template <class Arc>
    Wt const& operator()(Arc *a) const {
      return (*p)(a);
    }
  };
  Ptr ptr() { Ptr r; r.p = this; return r; }
  /**
     use this fn object if you already visited all the arcs that it will be used for to fill cache
  */
  struct FrozenPtr {
    typedef Wt Weight;
    Self *p;
    template <class Arc>
    Wt const& operator()(Arc *a) const {
      return *p->cache.find((intptr_t)a)->second;
    }
  };
  Ptr frozenPtr() { FrozenPtr r; r.p = this; return r; }
  struct FrozenShared {
    typedef Wt Weight;
    SharedPtr p;
    template <class Arc>
    Wt const& operator()(Arc *a) const {
      return *p->cache.find((intptr_t)a)->second;
    }
  };

  template <class HG>
  void populate(HG const& hg) {
    hg.forArcsAllowRepeats(ptr());
  }

  template <class HG>
  FrozenPtr freezePtr(HG const& hg) {
    FrozenPtr r;
    populate(hg);
    r.p = this;
    return r;
  }
};

template <class Wt>
struct OneStateWeight {
  typedef Wt Weight;
  Weight operator()(StateId) const { return Weight::one(); }
  template <class Weight> void timesByState(StateId, Weight &) const {}
};

template <class Wt>
struct ZeroStateWeight {
  typedef Wt Weight;
  Weight operator()(StateId) const { return Weight::zero(); }
  template <class Weight> void timesByState(StateId, Weight & w) const { w = Weight::zero(); }
};

/**
   for sorted (nonaxioms states first) hgs that you want to compute additional
   weights (for Inside/AllPairs paths) for. alternative to more expensive
   AxiomWeightHypergraph

   alternatively you can just say all states are axioms and give some states a zero weight
*/
template <class StateWtFn, class Wt = typename StateWtFn::Weight>
struct StateWeightsCached : StateWtFn {
  typedef Wt Weight;
  StateId axiomsStart;
  typedef boost::ptr_vector<Wt> Weights;
  Weights weights; // [0] = state axiomsStart
  bool isAxiom(StateId s) const { return s>=axiomsStart; }
  StateId weightsIndex(StateId s) { return s-axiomsStart; }
  Wt & axiomWt(StateId s) {
    assert(isAxiom(s));
    StateId i = s-axiomsStart;
    if (weights.is_null(i))
      return weights[i] = StateWtFn::operator()(i);
    return weights[i];
  }
  Wt operator()(StateId s) { return isAxiom(s) ? axiomWt(s) : Wt::one(); }
  void timesByState(StateId sid, Weight &w) {
    if (isAxiom())
      timesBy(axiomWt(sid), w);
  }
  StateWeightsCached() : axiomsStart() {}
  StateWeightsCached(StateId axiomsStart, StateId axiomsEnd)
      : axiomsStart(axiomsStart)
      , weights(axiomsEnd-axiomsStart)
  {}
  void init(StateId axiomsStart_, StateId axiomsEnd) {
    axiomsStart = axiomsStart_;
    weights.clear();
    weights.resize(axiomsEnd-axiomsStart);
  }
};

/**
   StateWtFn could be a StateToWeight<Weight>, or a table (StateWtFn)

   arcOnRight gives arc wt'=axioms*arcwt. (suitable for ngram, block weight)
   !arcOnRight gives wt'=arcwt*axioms
*/
template <class Wt, class StateWtFn = OneStateWeight<Wt>, class ArcWtFn = ArcWeight<Wt>, bool arcOnRight = true>
struct AxiomArcWeightImpl : StateWtFn, ArcWtFn {
  typedef Wt Weight;
  template <class Arc>
  Wt operator()(Arc *a) const {
    Wt const& arcw = ArcWtFn::operator()(a);
    Wt w(arcOnRight ? Wt::one() : arcw);
    StateIdContainer const& tails = a->tails();
    for (StateIdContainer::const_iterator i = tails.begin(), e = tails.end(); i!=e; ++i)
      StateWtFn::timesByState(*i, w);
    if (arcOnRight)
      timesBy(arcw, w);
    return w;
  }
};

}}

#endif
