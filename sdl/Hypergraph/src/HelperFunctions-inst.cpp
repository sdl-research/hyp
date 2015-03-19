// Include both .cpp and .hpp here:
#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/HelperFunctions.hpp>
#include <sdl/Hypergraph/src/HelperFunctions.cpp>

// Add all functions from HelperFunctions.hpp here:
#define INSTANTIATE_ARC_TYPES(ArcT) \
  template IMutableHypergraph<ArcT>* constructHypergraphFromString(std::string const&, IVocabularyPtr const&);

#include <sdl/Hypergraph/src/InstantiateArcTypes.ipp>
