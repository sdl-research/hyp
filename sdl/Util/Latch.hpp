




























template <class Set, class K>
bool latch(Set &s, K const& k) {
  if (!Util::contains(s, k)) {
    add(s, k);
    return true;
  }
  return false;
}

template <class Set, class I>
void latchAny(bool &anyadded, Set &s, I i, I const& end) {
  for (;i!=end;++i)
    if (latch(s,*i))
      anyadded=true;
}

template <class Set, class I>
void latchAny(bool &anyadded, Set &s, I const& i) {
  latchAny(anyadded, s, i.begin(), i.end());
}

template <class Set>
struct DupCount {
  Set s;
  std::size_t ndup;
  DupCount() : ndup() {}
  std::size_t get() const { return ndup; }
  template <class K>
  bool test(K const& k) const {
    return Util::contains(s, k); //test(s, k);
  }
  template <class K>
  void operator()(K const& k) {
    if (!latch(s, k)) ++ndup;
  }
};

template <class Set>
struct removeDupF {
  Set &s;
  removeDupF(Set &s) : s(s) {}
  removeDupF(removeDupF const& o) : s(o.s) {}
  template <class K>
  bool operator()(K const& k) const {
    return !latch(s, k);
  }
};

template <class Set>
removeDupF<Set> removeDup(Set &s) { return removeDupF<Set>(s); }




#endif
