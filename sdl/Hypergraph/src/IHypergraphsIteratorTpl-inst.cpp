// Include both .cpp and .hpp here:
#include <sdl/Hypergraph/IHypergraphsIteratorTpl.hpp>
#include <sdl/Hypergraph/src/IHypergraphsIteratorTpl.cpp>

#define INSTANTIATE_ARC_TYPES(ArcT) template class IHypergraphsIteratorTpl<ArcT>;
#include <sdl/Hypergraph/src/InstantiateArcTypes.ipp>
