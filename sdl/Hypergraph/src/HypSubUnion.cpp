#define HG_TRANSFORM_MAIN




namespace Hypergraph {

#define VERSION "v1"



  SubUnionOptions opt;



  {
    opt.requirePathOverlap = false;
    opt.addStandardUnion = false;



  }


    return kDefaultProperties | kStoreInArcs;
  }

  enum { has_inplace_input_transform = true, has_transform1 = false, has_transform2 = true, out_every = false }; // means multiple input files
  template <class Arc>
  bool inputTransformInPlace(IMutableHypergraph<Arc> &hg, int n) {
    return true;
  }

  template <class Arc>
  bool transform2mm(IMutableHypergraph<Arc>& hg1,
                    IMutableHypergraph<Arc>& hg2,
                    IMutableHypergraph<Arc>* o) {
    subUnion(hg1, hg2, o, opt);
    return true;
  }
};





