#define TRANSFORM HgReweight

#define VERSION "v1"
#define HG_TRANSFORM_MAIN





namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // CRTP
  typedef TransformMain<TRANSFORM> Base;

  {



  }

  Reweight rw;


  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc> &h) {
    rw.inplace(h);
    return true;
  }
};




