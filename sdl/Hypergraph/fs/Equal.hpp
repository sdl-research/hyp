






   therefore, implement fs-only Compose that supports them? note: openfst doesn't allow them in first position either.
*/











namespace Hypergraph {
namespace fs {

template <class A>
bool equal(IHypergraph<A> const& a, IHypergraph<A> const& b) {
  ASSERT_VALID_HG(a);ASSERT_VALID_HG(b);
  if (&a==&b) return true;
  if (a.prunedEmpty())
    return empty(b);
  if (b.prunedEmpty())
    return empty(a);
  if (!a.isFsm() || !b.isFsm()) {

  }
  if (a.getVocabulary() != b.getVocabulary()) {

  }
  //  return &a==&b; //FIXME

  typedef IHypergraph<A> H;


  // will need both indexes for compose-first-position





#define HG_EQUAL_CHECK_DIFF(a, b, msgv, msgempty) do {                     \

    difference(a, b, &d);                                                 \
    if (!empty(d)) {                                                    \
      return false;                                                     \
    }                                                                   \
  } while(0)


  HG_EQUAL_CHECK_DIFF(a, b,5, "test_fsm_equal empty a-b (may be equal!)");
  HG_EQUAL_CHECK_DIFF(b, a,4, "test_fsm_equal a-b == b-a == EMPTY, so a==b.\n");
#undef HG_EQUAL_CHECK_DIFF
  return true;
}



#endif
