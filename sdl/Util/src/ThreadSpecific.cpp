




struct DummyTssCleanupFunction : public impl::tss_cleanup_function {

};

static shared_ptr<impl::tss_cleanup_function> gDummyTssCleanupFunction(new DummyTssCleanupFunction());

shared_ptr<impl::tss_cleanup_function> makeDummyTssCleanupFunction() {
  return gDummyTssCleanupFunction;
}
#endif


