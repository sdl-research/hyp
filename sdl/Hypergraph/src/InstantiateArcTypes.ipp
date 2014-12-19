/** \file

    USAGE: e.g.
    (at top level, not namespace)

    #define INSTANTIATE_ARC_TYPES(arc) template void invert(IMutableHypergraph<arc> &h);
    #include <sdl/Hypergraph/src/InstantiateArcTypes.ipp>

    note: lack of include guard is essential.
*/

#define INSTANTIATE_WEIGHT_TYPES(x) INSTANTIATE_ARC_TYPES( ArcTpl< x > ) INSTANTIATE_ARC_TYPES( ArcWithDataTpl< x > )
#include <sdl/Hypergraph/src/InstantiateWeightTypes.ipp>
#undef INSTANTIATE_ARC_TYPES
