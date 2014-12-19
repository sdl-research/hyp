/** \df

    USAGE: e.g.
   (at top level, not namespace)

   #define INSTANTIATE_ARC_TYPES(arc) template void invert(IMutableHypergraph<ArcTpl<arc> > &h);
   #include <sdl/Hypergraph/src/InstantiateWeightTypes.ipp>
*/

#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Hypergraph/ExpectationWeight.hpp>
#include <sdl/Hypergraph/FeatureWeight.hpp>
#include <sdl/Hypergraph/TokenWeight.hpp>

namespace sdl {
namespace Hypergraph {

/**
   Any new Weight type must be added here. Hypergraph functions
   and classes will automatically be instantiated with this type, so
   that the code can be compiled into object files (explicit template
   instantiation, see http://stackoverflow.com/a/555349/60628):
 */
INSTANTIATE_WEIGHT_TYPES( ViterbiWeight )
INSTANTIATE_WEIGHT_TYPES( LogWeight )
INSTANTIATE_WEIGHT_TYPES( FeatureWeight )
#if SDL_FLOAT == 32
typedef FeatureWeightTpl<double, std::map<FeatureId, double> > DFeatureWeight;
INSTANTIATE_WEIGHT_TYPES( DFeatureWeight)
#endif
INSTANTIATE_WEIGHT_TYPES( ExpectationWeight )

#undef INSTANTIATE_WEIGHT_TYPES


}}
