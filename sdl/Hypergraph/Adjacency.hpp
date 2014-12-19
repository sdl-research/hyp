




#ifndef ADJACENCY_LW20111227_HPP
#define ADJACENCY_LW20111227_HPP



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

      ++n;
  }
};

}

template <class A>
ArcId countInArcs(IHypergraph<A> const& h, StateId s)
{


  detail::nIn v(s);

  return v.n;
}

template <class A>
ArcId countOutArcs(IHypergraph<A> const& h, StateId s)
{


  detail::nOut v(s);

  return v.n;
}





