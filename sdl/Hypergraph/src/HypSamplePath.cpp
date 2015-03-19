#define TRANSFORM HypSamplePath
#define USAGE "Generate sample path from hypergraph. Currently, this binary only supports uniform sampling (but C++ library allows more options)."
#define VERSION "v1"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/SamplePath.hpp>

namespace sdl {

namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // note base class CRTP (google it)
  typedef TransformMain<TRANSFORM> Base;
  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION) {}

  Properties properties(int i) const { // i=0 means output
    return kStoreInArcs; // we'll sample top down
  }

  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc> *o) {
    UniformInArcSampler<Arc> sampler;
    samplePath(i, sampler, o);
    return true;
  }
};

}}

INT_MAIN(sdl::Hypergraph::TRANSFORM)
