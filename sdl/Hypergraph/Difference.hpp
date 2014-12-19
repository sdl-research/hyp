















namespace Hypergraph {

template <class A>




  ASSERT_VALID_HG(a);
  ASSERT_VALID_HG(b);
  r->offerVocabulary(a);
  r->offerVocabulary(b);
  if (a.getVocabulary() != b.getVocabulary()) {

  }

  if (empty(a)) {
    r->setEmpty();
    return;
  }

  if (!b.isFsm())

  MutableHypergraph<A> c(kStoreOutArcs);
  fs::complement(b, &c);
  sortArcs(&c);

  ASSERT_VALID_HG(*r);
}




#endif
