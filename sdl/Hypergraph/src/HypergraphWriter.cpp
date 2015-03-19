#include <sdl/Hypergraph/HypergraphWriter.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <iostream>

namespace sdl {
namespace Hypergraph {

void dumpVHG(IHypergraph<ArcTpl<ViterbiWeightTpl<float> > > const& hg) {
  std::cout << hg;
}

void dumpFHG(IHypergraph<ArcTpl<FeatureWeight> > const& hg) {
  std::cout << hg;
}


}}
