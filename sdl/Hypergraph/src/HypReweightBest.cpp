#define TRANSFORM HgReweightBest

#define VERSION "v1"
#define HG_TRANSFORM_MAIN

//TODO: include compensating plus cost to all arcs first? saving additional call to HgReweight?





namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // CRTP
  typedef TransformMain<TRANSFORM> Base;





  }


  enum { has_inplace_transform1 = true };
  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc> &h) {
    rw.inplace(h);
    return true;
  }

    rw.opt.validate();
  }
};




