#define USAGE_HypSamplePath                                                                               \
  "Generate sample path from hypergraph. Currently, this binary only supports uniform sampling (but C++ " \
  "library allows more options)."
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/SamplePath.hpp>

namespace sdl {
namespace Hypergraph {

struct HypSamplePath : TransformMain<HypSamplePath> {
  HypSamplePath() : TransformMain<HypSamplePath>("SamplePath", USAGE_HypSamplePath) {}

  Properties properties(int i) const {  // i=0 means output
    return kStoreInArcs;  // we'll sample top down
  }

  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc>* o) {
    UniformInArcSampler<Arc> sampler;
    samplePath(i, sampler, o);
    return true;
  }
};


}}

HYPERGRAPH_NAMED_MAIN(SamplePath)
