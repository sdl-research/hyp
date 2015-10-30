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

    generic hypergraph traits in the spirit of Boost Graph Library
*/


#ifndef HYP__HYPERGRAPHTRAITS_HPP
#define HYP__HYPERGRAPHTRAITS_HPP
#pragma once


#include <sdl/Hypergraph/IHypergraph.hpp>
#include <graehl/shared/hypergraph.hpp>
#include <graehl/shared/containers.hpp>
#include <boost/property_map/property_map.hpp>
#include <graehl/shared/epsilon.hpp>
#include <sdl/Hypergraph/ExpectationWeight.hpp>
#include <sdl/Hypergraph/WeightUtil.hpp>
#include <sdl/Util/PointerHash.hpp>

namespace sdl {
namespace Hypergraph {

/* following boost graph library conventions, even though hyperarcs aren't really edges (don't have a unique
 * source but instead, several tails) */

struct HypergraphTraversalTag : boost::vertex_list_graph_tag, boost::bidirectional_graph_tag {};

template <class A>
StateId num_vertices(IHypergraph<A> const& h) {
  return h.sizeForHeads();
}

template <class A>
std::size_t num_edges(IHypergraph<A> const& h) {
  return h.getNumEdges();
}


template <class A>
StateIdRange vertices(IHypergraph<A> const& h) {
  return StateIdRange(StateIdIterator(0), StateIdIterator(num_vertices(h)));
}

template <class A>
ArcIdRange in_edges(IHypergraph<A> const& h, StateId s) {
  return h.inArcIds(s);
}

template <class A>
ArcId in_degree(IHypergraph<A> const& h, StateId s) {
  return h.numInArcs(s);
}

template <class A>
ArcId out_degree(IHypergraph<A> const& h, StateId s) {
  return h.numOutArcs(s);
}

template <class A>
ArcId degree(IHypergraph<A> const& h, StateId s) {
  return h.numInArcs(s) + h.numOutArcs(s);
}

template <class A>
ArcIdRange out_edges(IHypergraph<A> const& h, StateId s) {
  return h.outArcIds(s);
}

template <class A>
TailId num_tails(ArcHandle a, IHypergraph<A> const&) {
  return (*(A const*)a).getNumTails();
}

template <class A>
TailIdRange tails(ArcHandle a, IHypergraph<A> const&) {
  return (*(A const*)a).tailIds();
}

template <class A>
StateId tail(TailId t, ArcHandle a, IHypergraph<A> const&) {
  return (*(A const*)a).getTail(t);
}

template <class A>
StateId target(ArcHandle a, IHypergraph<A> const&) {
  return (*(A const*)a).head();
}

template <class A>
StateId source(ArcHandle a, IHypergraph<A> const&) {
  return (*(A const*)a).getTail(0);
}

template <class A>
struct ReadEdgeCostMap {
  typedef ArcHandle key_type;
  typedef typename A::Weight Weight;
  typedef typename Weight::FloatT value_type;
  typedef value_type reference;
  typedef boost::readable_property_map_tag category;
  reference operator[](ArcHandle a) const { return ((A*)a)->weight().getValue(); }
};

template <class A>
typename A::Weight::FloatT get(ReadEdgeCostMap<A> const& m, ArcHandle a) {
  return ((A*)a)->weight().getValue();
}

template <class A>
ReadEdgeCostMap<A> readEdgeCostMap(IHypergraph<A> const& h) {
  return ReadEdgeCostMap<A>();
}
}
}

namespace boost {

template <class A>
struct graph_traits<sdl::Hypergraph::IHypergraph<A>> {
  typedef A Arc;
  typedef sdl::Hypergraph::IHypergraph<A> H;

  typedef directed_tag directed_category;
  typedef allow_parallel_edge_tag edge_parallel_category;
  typedef sdl::Hypergraph::HypergraphTraversalTag traversal_category;

  typedef sdl::Hypergraph::ArcHandle edge_descriptor;
  typedef sdl::Hypergraph::StateId vertex_descriptor;

  typedef sdl::Hypergraph::StateIdIterator vertex_iterator;

  typedef sdl::Hypergraph::StateId vertices_size_type;
  typedef std::size_t edges_size_type;
  typedef sdl::Hypergraph::ArcId degree_size_type;
};
}

namespace graehl {

template <class A>
struct path_traits<sdl::Hypergraph::IHypergraph<A>> : graehl::cost_path_traits<typename A::Weight::FloatT> {};

template <class A>
struct edge_traits<sdl::Hypergraph::IHypergraph<A>> {
  typedef sdl::Hypergraph::IHypergraph<A> G;
  typedef typename path_traits<G>::cost_type cost_type;
  typedef sdl::Hypergraph::TailId tail_descriptor;
  typedef sdl::Hypergraph::TailIdIterator tail_iterator;
  typedef tail_descriptor tails_size_type;
};


// visit is in ns graehl so ADL doesn't cause ambiguity
template <class A, class V>
void visit(edge_tag, sdl::Hypergraph::IHypergraph<A> const& h, V& v) {  // v(edge_desciptor = A*)
  h.forArcsSafe(sdl::Util::visitorReference(v));
}

template <class A, class V>
void visit(edge_tag, sdl::Hypergraph::IHypergraph<A> const& h, V const& v) {
  h.forArcsSafe(sdl::Util::visitorReference(v));
}

// default factory for vertex_tag is fine (descriptors start at 0)
template <class A>
struct property_factory<sdl::Hypergraph::IHypergraph<A>, edge_tag> {
  typedef edge_tag ptag;
  typedef sdl::Hypergraph::IHypergraph<A> G;
  typedef boost::graph_traits<G> GT;
  property_factory() {}
  typename size_traits<G, ptag>::size_type N;
  property_factory(G const& g)
      : N(g.size()) {}  // COMPILER BUG? size(g, ptag()) isn't found (see property_factory.hpp)
  property_factory(property_factory const& o) : N(o.N) {}
  template <class V>
  std::size_t init() const {
    return 2 * N;
  }
  template <class V>
  struct rebind {
    property_factory& p;
    rebind(property_factory& p) : p(p) {}
    rebind(rebind const& o) : p(o.p) {}
    std::size_t init() const { return p.template init<V>(); }
    typedef sdl::unordered_map<sdl::Hypergraph::ArcHandle, V, sdl::Util::PointerHash<A>> impl;

    // TODO: hash_map w/ setEmpty w/ setEmptyKey(map)
    typedef boost::associative_property_map<impl> reference;
  };
};


}

#endif
