/** \file

    invert: swap input/output terminal state labels.
*/

#ifndef HYP__HG_INVERT_HPP
#define HYP__HG_INVERT_HPP
#pragma once

namespace sdl {
namespace Hypergraph {

struct IMutableHypergraphBase;

/// result has explicit (input,input) for (input)-only labels
void invertForceLabelPair(IMutableHypergraphBase &);

/// do nothing if labels are all (input)-only, else invertForceLabelPair (semantically
/// both are equivalent until you later chang einput label)
void invert(IMutableHypergraphBase &);

template <class Arc>
bool needsInvert(IHypergraph<Arc> const& h) {
  return h.hasOutputLabels();
}


}}

#endif
