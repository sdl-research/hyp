




#ifndef OPENFSTDRAW_LW201213_HPP
#define OPENFSTDRAW_LW201213_HPP







#if HAVE_OPENFST

#include <fst/script/draw.h>
#include <fst/script/print.h>

#endif






namespace Hypergraph {




  // openfst options:
  std::string title;
  std::string dest;
  bool accep;
  float width;
  float height;
  bool portrait;
  bool vertical;
  float ranksep;
  float nodesep;
  int fontsize;
  int precision;
  bool show_weight_one;


  // our options:
  std::string terminator;
  bool replaceFst;
  graehl::ostream_arg openFstPrintFile;





#define FOR_DRAW_OPTIONS(f) f(title) f(dest) FOR_DRAW_OPTIONS_INT(f)


    replaceFst = false;
    terminator = "\n";
    title = "Fsm";
    dest = "";  // ??     // drawn FST destination name - presumably from final state?

#define FOR_DRAW_OPTIONS_SET_0(n) n = 0;
    FOR_DRAW_OPTIONS_INT(FOR_DRAW_OPTIONS_SET_0);
    // TODO: nonzero defaults
    accep = true;








  }

  graehl::ostream_arg oarg;
  template <class Config>
  void configure(Config& config) {
#if HAVE_OPENFST
    config.is("OpenFst Draw/Print");
#define FOR_DRAW_OPTIONS_ADD_OPTION(n) config(#n, &n)("openfst draw " #n);


    config("replaceFst", &replaceFst)("use ToReplaceFst for fsm");




    config("stateNames", &stateNames)("name states for non-replaceFst openfst translation");
    config("pruneBeam", &pruneBeam)("unless nan, openfst prune with this beam before print/draw");

    ;
#endif
  }








  template <class A>


  }

#if HAVE_OPENFST














  template <class FstArc>



    fst::script::PrintFst<FstArc>(&a);
  }

  template <class FstArc>




      fst::script::DrawFst<FstArc>(&fa);


    if (openFstPrintFile) {
      std::ostream& of = openFstPrintFile.stream();

      of << terminator;
    }
  }

  template <class FstArc>


  }

  template <class FstArc>



  }

  template <class Arc>

    // IVocabularySymbolTable syms(h.getVocabulary());
    typedef typename Arc::Weight Weight;
    typedef fst::ArcTpl<Weight> FstArc;

    fst::SymbolTable fsyms("drawReplaceFst");
    fst::ReplaceFst<FstArc>* r = toReplaceFst<FstArc>(h, &fsyms);

    delete r;
  }
  template <class A>


#if TO_OPENFST_GETVALUE

#else

#endif

    typedef ToOpenFst<A, FstArc> T;



  }
  template <class A>

    if (replaceFst)





  }
#else
  template <class A>



#endif
};





