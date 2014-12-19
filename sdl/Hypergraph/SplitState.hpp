








#include <boost/array.hpp>










namespace Hypergraph {

/*
  split states based on its glue symbols(__LW_AT__).
*/



enum StateType {kStart, kNoGlueNoSpace, kNoGlueSpace, kGlueLeft, kGlueRight, kGlueBoth, kSpecial, knStateType};

/**
   add space to the beginning of the symbol if no glue indicator
*/

                         StateType currentType,

                         IVocabularyPtr pVoc,
                         bool spaceBetween) {

  if (currentType == kGlueLeft || currentType == kGlueBoth) {
    sym.erase(sym.begin(), sym.begin() + glue_len);
  }
  if (currentType == kGlueRight || currentType == kGlueBoth) {
    sym.erase(sym.end() - glue_len, sym.end());
  }
  if (frontType != kStart) {
    if ( !(frontType == kGlueRight ||
           frontType == kGlueBoth ||
           currentType == kGlueLeft ||
           currentType == kGlueBoth ||
           (frontType == kNoGlueNoSpace && currentType == kNoGlueNoSpace) ||
           (!spaceBetween && (frontType == kNoGlueNoSpace || currentType == kNoGlueNoSpace)))) {
      sym = " " + sym;
    }
  }

}


                               Util::Utf8RangePred const& pred,
                               IVocabularyPtr pVoc) {
  if (symId.isSpecial())
    return kSpecial;

  if (sym.length() < glue_len) return pred(sym)? kNoGlueNoSpace : kNoGlueSpace;
  if (Util::startsWith(sym, GLUE::TOKEN))
    return Util::endsWith(sym, GLUE::TOKEN) ? kGlueBoth : kGlueLeft;
  else if (Util::endsWith(sym, GLUE::TOKEN))
    return kGlueRight;
  else
    return pred(sym)? kNoGlueNoSpace : kNoGlueSpace;
}



template<class Arc>
struct TokenSplitPolicy {


  TokenSplitPolicy(IHypergraph<Arc> const& hg, Util::Utf8RangePred pred, bool spaceBetween):





  /*
    detect the type of the arc's head state

    Start : start state

    NoGlueNoSpace : no __LW_AT__ , no spaces between tokens

    NoGlueSpace : no __LW_AT__ , insert space at left and right

    GlueLeft : __LW_AT__ at the left

    GlueRight : __LW_AT__ at the right

    GlueBoth : __LW_AT__ at both sides



  */



  }















  /*
    connect s1 and s2 with the arc

    first get rid of "__LW_AT__" on the original symbol from arc, then
    based on s1's type, type1, and s2's type, type2,
    an additionaly space may or may not be added to the begining of the symbol.
  */






  }




};


/*
  visitor class that is intended be used with topological traversal.

  it will split a state based on its incomming arcs.
  one state from the input lattice could have multiple incoming arcs, therefore multiple
  types. The policy class decides the types for each state and connects them with
  arcs.


  */
template<class Arc, class SplitPolicy>
struct SplitStateVisitor : public IStatesVisitor, public SplitPolicy {
  typedef typename SplitPolicy::StateSplits StateSplits;


  SplitStateVisitor(IHypergraph<Arc> const& hg,
                    IMutableHypergraph<Arc>* pHgResult,
                    Util::Utf8RangePred pred,
                    bool spaceBetween):
      SplitPolicy(hg, pred, spaceBetween),





  {







    StateSplits splits;



  }







      return;




























      }







        }
      }




    }
  }





  }












};




#endif
