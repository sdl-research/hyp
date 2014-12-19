#define TRANSFORM HgDeterminize
#define USAGE "Determinize an unweighted FSA hypergraph -- input symbols only. TODO: support sigma, phi, weights, outputs."
#define VERSION "v1"

#define HG_TRANSFORM_MAIN





namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // note base class CRTP (google it)
  typedef TransformMain<TRANSFORM> Base;
  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION)



    this->configurable(&detOpt);
  }

  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc> *o) {

    return true;
  }
};




