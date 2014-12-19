#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>

namespace sdl {
namespace Hypergraph {

inline bool needsInvert(IMutableHypergraphBase const& h) {
  return !h.hgOutputLabelFollowsInput();
}

inline LabelPair invert(LabelPair x) {
  return LabelPair(x.second, x.first);
}

void invertForceLabelPair(IMutableHypergraphBase &h) {
  for (StateId s = 0, e = h.hgGetNumStates(); s < e; ++s)
    h.setLabelPair(s, invert(h.hgGetLabelPair(s)));
}

void invert(IMutableHypergraphBase &h) {
  if (needsInvert(h))
    invertForceLabelPair(h);
}


}}
