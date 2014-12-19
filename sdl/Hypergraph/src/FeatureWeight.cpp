#include <string>

#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Hypergraph/FeatureWeight.hpp>
#include <sdl/Hypergraph/src/FeatureWeight.ipp>

namespace sdl {
namespace Hypergraph {

// Explicit template instantiation of the parseWeightString function
// for FeatureWeight (i.e., FeatureWeightTpl templated on common
// types):
template
void parseWeightString(std::string const& str, FeatureWeight* weight);

// Define parseWeightString for the other float type
#if SDL_FLOAT == 32
template
void parseWeightString(std::string const& str,
                       FeatureWeightTpl<double, std::map<FeatureId, double> >* weight);
#else
template
void parseWeightString(std::string const& str,
                       FeatureWeightTpl<float, std::map<FeatureId, float> >* weight);
#endif


}}
