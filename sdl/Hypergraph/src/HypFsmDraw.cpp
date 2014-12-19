#define HG_TRANSFORM_MAIN











namespace Hypergraph {
#if HAVE_OPENFST
# define USAGEFST " (using openfst draw if fsm)"
#else
# define USAGEFST ""
#endif

#define USAGE "Print graphviz (dot) equivalent of hypergraph unless --out=-0 " USAGEFST

#define VERSION "v1"


  DrawOptions dopt;






  }


    return kStoreInArcs;
  }

  enum { has_transform1 = false, has_transform2 = false, has_inplace_input_transform = true };
  template <class Arc>





    return true;
  }




};




