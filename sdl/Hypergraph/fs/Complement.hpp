



   construction: roughly speaking, take a determinized transducer with a
   transition for every symbol, then invert final(x)

   input is determinized w/o epsilons (so has 0 or 1 accepting paths for all
   strings). so the first divergence from that accepting path means the string
   isn't in input set, so is in complement. furthermore, any path that reaches
   end-of-string early, and wasn't final in input, should be final in complement.

   if input doesn't contain empty string, then simply add a new sink "failure"
   state with rho self-loop, then from every state, enter that with a rho (else),
   unless that state already has a sigma or rho. if empty string was in original,
   then ensure that we don't have it in complement.

*/

#ifndef HYPERGRAPH_COMPLEMENT_HPP
#define HYPERGRAPH_COMPLEMENT_HPP










#include <algorithm>




namespace Hypergraph {
namespace fs {

template <class SpecialSymbol, class Arc>
struct NonSpecialInput {
  typedef IHypergraph<Arc> H;



};

template <class Arc>

  ASSERT_VALID_HG(inhg);

  result->offerVocabulary(inhg);
  if (empty(inhg)) {
    // result is sigma*
    result->setAllStrings(complementSigma);
    return;
  }

  // deteriminize:
  typedef IHypergraph<Arc> H;
  shared_ptr<H const> det;  // = ptr(inhg); inplace(inhg, Determinize(DETERMINIZE_INPUT, kStoreOutArcs));
  // determinize only if needed:
  H const& hg = determinized(inhg, det, DETERMINIZE_INPUT);

  // store arcs:



  /*

    the construction is: from every state, including the old final state, add a
    rho (else) transition to a new final state, where once you get there, you
    can stay there with any symbol. obviously if there already was a wildcard
    (sigma) or else (rho), then there are no "else" remaining

    if original didn't contain empty string, we need to add it to the result
    set. but we can't go to the "stay there" final state with epsilon, or
    we'd accept everything. so we then need two final states; for that, we
    need a new final state with epsilons from both start (for the empty
    string) and the old sink state

  */

  bool hadempty = containsEmptyStringDet(hg);






  StateId rhoLabelState = result->addState(RHO::ID);
  StateId sigmaLabelState = result->addState(complementSigma);




  assert(pVoc != 0);
  bool reachedNewFinal = false;




    typedef typename Arc::Weight Weight;



      reachedNewFinal = true;
    }
  }
  // stay in final state on any symbol
  if (reachedNewFinal) {



  }


  if (!hadempty) {  // then complement must contain empty.
    StateId epsilonState = result->addState(EPSILON::ID);
    StateId emptyFinal;



  } else if (!reachedNewFinal)
    result->setEmpty();

  ASSERT_VALID_HG(*result);
}
























#endif
