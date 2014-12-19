#include <sdl/LexicalCast.hpp>
#include <sdl/Hypergraph/Types.hpp>

namespace sdl {
namespace Hypergraph {

std::string featureIdRangeDescription(FeatureId begin, FeatureId end) {
  return sdl::lexical_cast<std::string>(begin)+"<=id<"+sdl::lexical_cast<std::string>(end);
}

}}
