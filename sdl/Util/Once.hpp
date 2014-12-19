









#include <boost/static_assert.hpp>






// wrap pointer-visitors so each pointer gets visited once - mark the pointer seen the first time (via IntSet)

struct Once : public PointerSet {
  BOOST_STATIC_ASSERT(sizeof(intptr_t) == sizeof(void*));
  template <class V>
  inline bool first(V const* p) {
    return this->insert((intptr_t)p).second;
  }
};



template <class V>

  BOOST_STATIC_ASSERT(sizeof(intptr_t) == sizeof(void*));
  // careful: don't pass by val:


};


template <class F>
struct VisitOnce : public F {
  VisitOnce(F const& f, PointerSet* p) : F(f), p(p) {}
  PointerSet* p;
  // mutating via pointer p - const helps compiler out?
  template <class V>
  inline bool first(V const* v) const {
    return p->insert((intptr_t)v).second;
  }
  template <class V>
  void operator()(V* v) const {
    if (first(v)) F::operator()(v);
  }
  template <class V>
  void operator()(V const* v) const {
    if (first(v)) F::operator()(v);
  }
};

template <class F>
VisitOnce<F> makeVisitOnce(F const& f, PointerSet* once) {
  return VisitOnce<F>(f, once);
}







  // mutating via pointer p - const helps compiler out?
  template <class V>
  inline bool first(V const* v) const {
    return p->insert((intptr_t)v).second;
  }



  }
  template <class V>
  void operator()(V const* v) const {












#endif
