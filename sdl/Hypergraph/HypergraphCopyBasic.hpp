

















namespace Hypergraph {

template<class Arc>
struct ArcCopyPreserveStateIdsFct {
  ArcCopyPreserveStateIdsFct(IMutableHypergraph<Arc>* pTarget)





};

// if Keep(Arc const&) return true, copy.
template<class Arc, class Keep>
struct ArcCopySome : public Keep {












template <class A, class Keep>
ArcCopySome<A, Keep> copyArcs(IMutableHypergraph<A> *to, Keep const& r) {
  return ArcCopySome<A, Keep>(to, r);
}

template <class T>
struct TransformHasOutput {
  static const bool hasOutputLabels = T::hasOutputLabels;
};



















}

template <class A, class B, class T>
void copyStatesRelabel(IHypergraph<A> const& from, IMutableHypergraph<B> *to, T const& tr) {
  const bool hasOut = TransformHasOutput<T>::hasOutputLabels;









      added = to->addState(in, oprime);
    } else
      added = to->addState(tr(i, o));
    assert(added==s);
  }





















template <class Arc, class T>
void copyHypergraphRelabel(const IHypergraph<Arc>& from,
                           IMutableHypergraph<Arc>* to,
                           T const& tr
                           )
{

  from.forArcs(from, ArcCopyPreserveStateIdsFct<Arc>(to));
}











  from.forArcs(ArcCopyPreserveStateIdsFct<Arc>(to));
}


















template<class Arc, class Keep>
void copyHypergraphPartial(const IHypergraph<Arc>& from,
                           IMutableHypergraph<Arc>* to,
                           Keep const& r



  from.forArcs(copyArcs(to, r));
}

template<class Arc, class T, class Keep>
void copyHypergraphRelabelPartial(const IHypergraph<Arc>& from,
                                  IMutableHypergraph<Arc>* to,
                                  T const& tr,
                                  Keep const& r
                                  ) {
  copyStatesRelabel(from, to, tr);
  from.forArcs(copyArcs(to, r));
}

template <class A>

  o->forcePropertiesOnOff(forceOn, forceOff);
  copyHypergraph(i, o);
  o->forcePropertiesOnOff(forceOn, forceOff); // because sort has to happen again after, at least
}










































  Properties switchOn = forceOn & ~ip;
  Properties switchOff = forceOff & ip;























  Properties switchOn = forceOn & ~ip;
  Properties switchOff = forceOff & ip;






















































































