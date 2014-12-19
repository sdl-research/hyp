#ifndef SDL_OPTIMIZATION_IORIGINALFEATUREIDS_HPP
#define SDL_OPTIMIZATION_IORIGINALFEATUREIDS_HPP
#pragma once

#include <sdl/Optimization/Types.hpp>

namespace sdl {
namespace Optimization {

struct IOriginalFeatureIds {
  virtual ~IOriginalFeatureIds() {};
  /**
      Maps contiguous feature IDs (used during training) back to
      the original feature IDs, which can be non-contiguous (see
      Hypergraph/FeatureIds.hpp).
   */
  virtual FeatureId getOriginalFeatureId(FeatureId contiguousFeatureId) = 0;
};

struct IdentityOriginalFeatureIds : IOriginalFeatureIds {
  FeatureId getOriginalFeatureId(FeatureId id) {
    return id;
  }
};


}}

#endif
