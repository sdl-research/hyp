#include <string>

#include <sdl/Hypergraph/FeatureWeightTpl.hpp>
#include <sdl/Hypergraph/src/FeatureWeight.ipp>
#include <sdl/Hypergraph/ExpectationWeight.hpp>


namespace sdl {
namespace Hypergraph {

// Explicit template instantiation of the parseWeightString function
// for ExpectationWeight (i.e., FeatureWeightTpl templated on certain
// types):
template void parseWeightString(std::string const& str, ExpectationWeight* weight);


}}
