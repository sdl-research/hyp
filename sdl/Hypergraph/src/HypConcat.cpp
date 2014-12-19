#define HG_TRANSFORM_MAIN





namespace Hypergraph {


#define VERSION "v1"

// FIXME: weird problem with TransformMain cascades of inplace transform2




    this->configurable(&opt);

  }

  // TODO: inefficient to always store in and out arcs


  }

  ConcatOptions opt;

  enum { has_transform1 = false, has_inplace_transform2 = true };

  template <class Arc>



    hgConcat(&l, r, opt);

    return true;
  }
};





