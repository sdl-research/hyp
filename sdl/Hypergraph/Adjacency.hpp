// Copyright 2014-2015 SDL plc
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

    Count adjacent arcs even if a hg doesn't store that type of adjacency.
*/

#ifndef ADJACENCY_LW20111227_HPP
#define ADJACENCY_LW20111227_HPP
#pragma once

namespace sdl {
namespace Hypergraph {

namespace detail {
struct nIn
{
  StateId s;
  ArcId n;
  nIn(StateId s) : s(s), n() {}
  template <class A>
  void operator()(A const* a)
  {
    if (a->head()==s)
      ++n;
  }
};

struct nOut
{
  StateId s;
  ArcId n;
  nOut(StateId s) : s(s), n() {}
  template <class A>
  void operator()(A const* a)
  {
    if (a->fsmSrc()==s)
      ++n;
  }
};

}

template <class A>
ArcId countInArcs(IHypergraph<A> const& h, StateId s)
{
  if (h.storesInArcs())
    return h.numInArcs(s);
  detail::nIn v(s);
  h.forArcs(Util::visitorReference(v));
  return v.n;
}

template <class A>
ArcId countOutArcs(IHypergraph<A> const& h, StateId s)
{
  if (h.storesOutArcs())
    return h.numOutArcs(s);
  detail::nOut v(s);
  h.forArcs(Util::visitorReference(v));
  return v.n;
}


}}

#endif
