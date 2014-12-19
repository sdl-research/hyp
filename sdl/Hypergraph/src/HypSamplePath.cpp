#define TRANSFORM HgSamplePath

#define USAGE "Generate sample path from hypergraph. Currently, this binary only supports uniform sampling (but C++ library allows more options)."
#define VERSION "v1"

#define HG_TRANSFORM_MAIN





namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // note base class CRTP (google it)
  typedef TransformMain<TRANSFORM> Base;
  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION) {}


    return kStoreInArcs; // we'll sample top down
  }

  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc> *o) {
    UniformInArcSampler<Arc> sampler;
    samplePath(i, sampler, o);
    return true;
  }
};




