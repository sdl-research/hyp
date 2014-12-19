
















































*/





/*





















*/

/*TODO (algorithms):

  approx minimize weighted NFA

  exact finite CFG -> fsm (PDA stack construction). lazy might be useful

  approx CFG -> NFA

  parse / check membership of string (without doing seq-fsa -> intersect)

  copy within-alpha of weighted inside*outside

  parse = compose with trivial acceptor - but CKY type parse instead of Earley?

  bottom-up rescoring/splitting of CFG
  left-right

*/

/*TODO (structure):

  OpenFst arc mutator/iterator support (many things marked TODO)

  LAZY cfg (???)




*/

#include <stdexcept>
#include <cstddef>
#include <cassert>




















#include <boost/range/size.hpp>
#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>









namespace Hypergraph {

// Bug: ASSERT_VALID_HG fails on FSMs that do not store a vocab (b/c
// it tests for lexical states). A vocab should not be required in
// general.
#ifdef NDEBUG
#define ASSERT_VALID_HG(hg)
#else
#define ASSERT_VALID_HG(hg)                                                         \
  do {                                                                              \
    if (!(hg).checkValid()) {                                                       \

    }                                                                               \
  } while (0)
#endif




                  "Hypergraph has no arcs (use kStoreInArcs and/or kStoreOutArcs)"); \
  } while (0);






/**

*/
struct IHypergraphStates : Resource {






































































     A Graph in xmt is a left-recursive CFG. the set of all kGraph
     hypergraphs is a superset of the strictly binary kFsm. a
     hypergraph has property kGraph iff all its arcs are graph
     arcs. (kFsm implies kGraph)

     A graph arc has N tails: all but the first have a terminal label


     Note: an arc that has in the first tail a lexical (leaf) state is
     not a graph. graphs must have a defined start and final state or
     else they're considered empty.

     This may run an expensive check O(n), but the next calls w/o
     changing an IMutableHypergraph will be O(1).

































































  /// constant time - because final state is set to kNoState when we detect that there are no
  // derivations. may return false even if there are no derivations.
  bool prunedEmpty() const;









    return kNoState;




















     *

















































































  // first f(ilabel) returing true aborts visiting early
  template <class F>
  bool anyInputLabel(F const& f, bool alsoNolabel = false) const {








  // first f(stateid, ilabel, olabel) returning true aborts visiting early
  template <class F>
  bool anyLabel(F const& f, bool alsoNolabel = false) const {













  /// Note: you can ask for a state that hasn't been created yet -
  /// you'll get in = out = NoSymbol. alternatively, check

























  template <class V>  // note - by ref, and pass *this as first arg
  void forAxioms(V& v) const {



  }

  template <class V>  // note - by ref
  void forAxiomIds(V& v) const {


  }

  template <class V>
  void forAxiomIds(V const& v) const {


  }





  template <class V>

    forStates(v, false, true, true);
  }

  template <class V>
  void forLexicalStates(V const& v) const {
    forStates(v, true, false, false);
  }

  template <class V>
  void forLexicalStates(V const& v) {
    forStates(v, true, false, false);
  }



  /**


  */
  template <class V>

    // start first
    Self& hg = const_cast<Self&>(*this);








    }
  }







  bool storesInArcs() const;





  bool storesOutArcs() const;





  bool storesFirstTailOutArcs() const;




     \return storesOutArcs() && !storesFirstTailOutArcs().



















































};  // end IHypergraphStates




   Implementation is templated so MutableHypergraph can bypass all the






inline bool isGraphArcImpl(HG const& hg, typename HG::Arc const& a, bool& fsm, bool& oneLexical) {






















































































































































































































































































































  // does s have an outgoing transition for every symbol (aside from
  // epsilon to final, which should be //TODO: multiple final states,
  // so we don't have to distinguish between real and fake epsilons)
  virtual bool hasAllOut(StateId s) const;






























































































  template <class V>
  void forArcsOut(V const& v, bool keepRepeats = false) const {
    if (storesFirstTailOutArcs() || keepRepeats)
      forArcsOutEveryTail(v);
    else
      forArcsOutOnce(v);
  }

  // f(Arc *): Val ; note f by ref
  // note: f(Arc *) because updateBy uses map type
  template <class Val, class F>

    if (storesInArcs())

    else

  }

  // safe even if you update arcs as you go e.g. deleting them
  template <class V>
  void forArcsOutOnceSafe(V const& v) const {


    // TODO: JG: more efficient: small open hash inside per-state loop (only iterating over tails),
    // or just quadratic scan (#tails is always small)
  }

  template <class V>
  void forArcsFsm(V const& v) const {
    if (!getVocabulary()) {
      forArcsOutFirstTail(v);
      return;
    }






    if (st == Hypergraph::kNoState)

    else {
      assert(!hasLexicalLabel(st));
      FORSTATE(st);


#undef FORSTATE
    }
  }

  template <class V>
  void forArcsOutOnce(V const& v) const {
    if (storesFirstTailOutArcs())


      forArcsFsm(v);

      forArcsOutOnceSafe(v);
  }

  template <class V>

    if (firstTailOnly)

    else

  }








  }





  }


  template <class V>




    }
  }
























    if (storesInArcs())











  // prefer in if have, else out
  template <class V>

    if (storesInArcs())

    else

  }

























  template <class V>


  }

  template <class V>
  void forArcsOutEveryTail(V const& v) const {

  }

  template <class V>


  }
  template <class V>
  void forArcsIn(V const& v) const {
    assert(storesInArcs());

  }

  // forArcs visits each Arc * once. for sure. also doesn't mind if there are no
  // arc indices (because this is used by destructor and we may wish to allow
  // arcless "hypergraphs" w/ states+labels only?

  // ok to modify arcs without affecting visit-once property
  template <class V>
  void forArcsSafe(V const& v) const {
    // no repeats
    if (storesInArcs())
      forArcsIn(v);
    else if (storesOutArcs())
      forArcsOutOnceSafe(v);
  }
  template <class V>
  void forArcs(V const& v) const {
    // no repeats
    if (storesInArcs())
      forArcsIn(v);
    else if (storesOutArcs())
      forArcsOutOnce(v);
    else
      THROW_NO_ARCS();
  }

  template <class V>
  void forArcsPreferRepeats(V const& v) const {
    if (storesOutArcs())
      forArcsOutEveryTail(v);
    else if (storesInArcs())
      forArcsIn(v);
    else
      THROW_NO_ARCS();
  }
  template <class V>
  void forArcsAllowRepeats(V const& v) const {
    if (storesInArcs())
      forArcsIn(v);
    else if (storesOutArcs())
      forArcsOutEveryTail(v);
    else
      THROW_NO_ARCS();
  }






    forArcsOut(ArcPrinter(o), keepRepeats);
  }



  WeightCount countArcs() const {
    WeightCount r;

    return r;
  }

    forArcsAllowRepeats(impl::set_weight1());
  }
  // TODO: cache in props?




  virtual IHypergraph<A>* clone() const = 0;

  virtual ~IHypergraph() {}

  /**



  */

  /**


  */






































































































  LabelPair getFsmLabelPair(Arc const& a) const {
    assert(isFsm());


  }

    assert(isFsm());


  }

    assert(isFsm());


  }

    assert(isFsm());


  }





    assert(storesOutArcs());

























 protected:





  template <class Val, class F>

    assert(storesInArcs());


  }

  template <class Val, class F>

    assert(storesOutArcs());


  }

};  // end class IHypergraph


























  if (N == 0) return kNoState;
  if (h.storesOutArcs()) {




    return kNoState;

  StateId r = kNoState;
  if (h.storesInArcs()) {















  if (N == 0) return kNoState;
  if (h.storesInArcs()) {




    return kNoState;

  StateId r = kNoState;
  if (h.storesOutArcs()) {

















































































#endif
