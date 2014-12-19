#define TRANSFORM HgInvert

#define VERSION "v1"

#define HG_TRANSFORM_MAIN




namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // note base class CRTP (google it)
  typedef TransformMain<TRANSFORM> Base;
  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION) {}





  template <class Arc>


    return true;
  }
};




