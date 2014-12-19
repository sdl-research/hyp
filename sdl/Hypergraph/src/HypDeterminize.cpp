#define TRANSFORM HgDeterminize

#define VERSION "v1"

#define HG_TRANSFORM_MAIN





namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // note base class CRTP (google it)
  typedef TransformMain<TRANSFORM> Base;





  }

  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc> *o) {

    return true;
  }
};




