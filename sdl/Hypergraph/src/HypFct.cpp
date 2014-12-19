#define TRANSFORM HgFct


#define VERSION "v1"

#define HG_TRANSFORM_MAIN



namespace Hypergraph {





struct TRANSFORM : TransformMain<TRANSFORM> { // note base class CRTP (google it)

  typedef TransformMain<TRANSFORM> Base;

  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION) {}



  }





  enum { has_inplace_transform1 = true };

  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc> &h) {
    testFct(h);
    return true;
  }
};





