








#include <sstream>
#include <utility>
#include <functional>

#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>













namespace Hypergraph {


template<class Arc>
std::string getString(IHypergraph<Arc> const& hg,

  return getStringWt(hg, opts).first;
}



template <class A>


                           IHypergraph<A> const& hg,





      result.push_back(sym);
  }
  return result;
}





                          IHypergraph<Arc> const& hg,
                          DerivationStringOptions const& opts = DerivationStringOptions())
{












  return result;
}



                   IHypergraph<Arc> const& hg,
                   DerivationStringOptions const& opts = DerivationStringOptions())
{



}

inline

                         IVocabularyPtr const& pVoc,
                         char const* space = " ",
                         SymbolQuotation quote = kQuoted)
{
  std::ostringstream out;
  print(out, str, pVoc, space, quote);
  return out.str();
}



                                  IVocabularyPtr const& pVoc,
                                  char const* space = " ",
                                  SymbolQuotation quote = kQuoted)
{







                         char const* space = " ",
                         SymbolQuotation quote = kQuoted)
{
  std::ostringstream out;

  return out.str();
}





                                  char const* space = " ",
                                  SymbolQuotation quote = kQuoted)
{































std::string textFromDeriv(typename Derivation<Arc>::child_type const& pDerivation,
                          IHypergraph<Arc> const& hg,
                          DerivationStringOptions const& opts = DerivationStringOptions())
{





                                   IHypergraph<Arc> const& hg,
                                   DerivationStringOptions const& opts = DerivationStringOptions())
{

}



std::string textFromStates(StateString const& ss,
                           IHypergraph<Arc> const &hg,
                           DerivationStringOptions const& opts = DerivationStringOptions())
{






                                    DerivationStringOptions const& opts = DerivationStringOptions())
{

}


template<class Arc>

getStringWt(IHypergraph<Arc> const& hg,





  typename Derivation<Arc>::child_type deriv = singleDerivation(hg);
  if (!deriv)










































#endif
