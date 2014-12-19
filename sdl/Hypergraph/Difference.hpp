















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


  fs::complement(b, &c);


  ASSERT_VALID_HG(*r);
}




#endif
