/** \file

    fns for copying arcs without changing stateids (to change ids, see
    HypergraphCopy.hpp)
*/

#ifndef HYP__HYPERGRAPH_COPY_BASIC_HPP
#define HYP__HYPERGRAPH_COPY_BASIC_HPP
#pragma once

#include <sdl/Util/LogHelper.hpp>
#include <sdl/Hypergraph/FwdDecls.hpp>
#include <sdl/Hypergraph/Exception.hpp>
#include <sdl/Hypergraph/Properties.hpp>
#include <boost/pointer_cast.hpp>
#include <sdl/SharedPtr.hpp>

namespace sdl {
namespace Hypergraph {

template<class Arc>
struct ArcCopyPreserveStateIdsFct {
  ArcCopyPreserveStateIdsFct(IMutableHypergraph<Arc>* pTarget)
      : pTarget_(pTarget) {}
  void operator()(Arc const* arc) const {
    pTarget_->addArc(new Arc(*arc));
  }
  IMutableHypergraph<Arc>* pTarget_;
};

// if Keep(Arc const&) return true, copy.
template<class Arc, class Keep>
struct ArcCopySome : public Keep {
  IMutableHypergraph<Arc>* pTarget_;
  ArcCopySome(IMutableHypergraph<Arc>* pTarget, Keep const& r)
      : pTarget_(pTarget), Keep(r) {}
  void operator()(Arc const& arc) const {
    if (Keep::operator()(arc))
      pTarget_->addArc(new Arc(arc));
  }
  void operator()(Arc const* arc) const {
    (*this)(*arc);
  }
};

template <class A, class Keep>
ArcCopySome<A, Keep> copyArcs(IMutableHypergraph<A> *to, Keep const& r) {
  return ArcCopySome<A, Keep>(to, r);
}

template <class T>
struct TransformHasOutput {
  static const bool hasOutputLabels = T::hasOutputLabels;
};

enum ClearAndSameProperties {
  kNoClear = 0, kClearAndSameProperties = 1
};

template <class A, class B>
void copyExceptArcs(IHypergraph<A> const& from, IMutableHypergraph<B> *to, ClearAndSameProperties clearAndSameProperties = kClearAndSameProperties) {
  if (clearAndSameProperties == kClearAndSameProperties)
    to->clear(from.properties());
  to->setVocabulary(from.getVocabulary());
  forall (StateId sid, from.getStateIds()) {
    to->addStateId(sid,
                   from.inputLabel(sid),
                   from.outputLabel(sid));
  }
  to->setStart(from.start());
  to->setFinal(from.final());
  if (from.properties() & kSortedStates)
    to->setSortedStatesNumNotTerminal(from.exactSizeForHeads());
}

template <class A, class B, class T>
void copyStatesRelabel(IHypergraph<A> const& from, IMutableHypergraph<B> *to, T const& tr) {
  const bool hasOut = TransformHasOutput<T>::hasOutputLabels;
  to->setVocabulary(from.getVocabulary());
  Sym oprime = NoSymbol;
  for (StateId s = 0, e = from.size(); s<e; ++s) {
    const LabelPair io = from.labelPair(s);
    const Sym i = input(io);
    const Sym o = output(io);
    StateId added;
    if (hasOut) {
      Sym in = tr(&oprime, i, o);
      added = to->addState(in, oprime);
    } else
      added = to->addState(tr(i, o));
    assert(added==s);
  }
  to->setStart(from.start());
  to->setFinal(from.final());
}

template <class A, class T>
void inplaceRelabel(IMutableHypergraph<A> *phg, T const& tr) {
  const bool hasOut = TransformHasOutput<T>::hasOutputLabels;
  IHypergraph<A> const& from = *phg;
  LabelPair newLabel;
  output(newLabel) = NoSymbol;
  for (StateId s = 0, e = from.size(); s<e; ++s) {
    const LabelPair io = from.labelPair(s);
    if (hasOut) {
      input(newLabel) = tr(&output(newLabel), io.first, io.second);
    } else
      input(newLabel) = tr(io.first, io.second);
    phg->setLabelPair(newLabel);
  }
}

// same structure as original, but modified labels.
template <class Arc, class T>
void copyHypergraphRelabel(const IHypergraph<Arc>& from,
                           IMutableHypergraph<Arc>* to,
                           T const& tr
                           )
{
  copyStatesRelabel(from, to, tr);
  from.forArcs(from, ArcCopyPreserveStateIdsFct<Arc>(to));
}

/**
   copy hypergraph 'from' into 'to'.
*/
template<class Arc>
void copyHypergraph(const IHypergraph<Arc>& from,
                    IMutableHypergraph<Arc>* to,
                    ClearAndSameProperties clearAndSameProperties = kClearAndSameProperties
                    )
{
  copyExceptArcs(from, to, clearAndSameProperties);
  from.forArcs(ArcCopyPreserveStateIdsFct<Arc>(to));
}

/**
   copy hypergraph 'from', storing it (a new MutableHypergraph) in holdNew; return pointer to it.
*/
template<class Arc>
MutableHypergraph<Arc>* copyHypergraphNew(const IHypergraph<Arc>& from,
                                          shared_ptr<IHypergraph<Arc> const> &holdNew,
                                          Properties prop) {
  MutableHypergraph<Arc> *r = new MutableHypergraph<Arc>(prop);
  holdNew.reset(r);
  r->setVocabulary(from.getVocabulary());
  copyHypergraph(from, r, kNoClear);
  return r;
}

/**
   copy hypergraph 'from' into 'to', keeping only a subset of the arcs
*/
template<class Arc, class Keep>
void copyHypergraphPartial(const IHypergraph<Arc>& from,
                           IMutableHypergraph<Arc>* to,
                           Keep const& r
                           , ClearAndSameProperties clearAndSameProperties = kClearAndSameProperties
                           ) {
  copyExceptArcs(from, to, clearAndSameProperties);
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
void copyEnsuringProperties(IHypergraph<A> const& i, IMutableHypergraph<A>* o, Properties forceOn = 0, Properties forceOff = 0) {
  o->forcePropertiesOnOff(forceOn, forceOff);
  copyHypergraph(i, o);
  o->forcePropertiesOnOff(forceOn, forceOff); // because sort has to happen again after, at least
}

enum OnMissingProperties {
  kMissingThrowMask = 1, // not throw implies copy.
  kMissingModifyMask = 2,
  kThrowUnlessProperties = kMissingThrowMask,
  kModifyOrCopyEnsuringProperties = kMissingModifyMask,
  kCopyEnsuringProperties = 0,
  kModifyOrThrowUnlessProperties = kMissingModifyMask|kMissingThrowMask,
};


/**
   \return hypergraph with desired properties - either i or *pMaybeCopyHg (if copy was needed)

   \param out pMaybeCopyHg holds a copy if input hg doesn't already have the desired
   properties on/off.

   if input hg has properties p, then it satisfies our requirement if:

   all the bits in forceOn OR orForceOn are set in p
   AND none of the bits in forceOff are set in p

   the copy, if needed, will have maybeOff properites cleared.

   (e.g. if you want InArcs and don't have them already, don't store OutArcs in
   the copy, call with forceOn=kStoreInArcs and maybeOff=kStoreOutArcs)

   see OnMissingProperties for alternatives to copying the input hg if
   properties aren't satisfied (e.g. kModifyOrThrowUnlessProperties)

   (e.g. if you want, for a graph/fsm, kStoreFirstTailOutArcs or kStoreOutArcs,
   call with forceOn = kStoreFirstTailOutArcs and orForceOn = kStoreOutArcs)
*/
template <class A>
IHypergraph<A> const& ensureProperties(IHypergraph<A> const& i,
                                       shared_ptr<IHypergraph<A> const> &pMaybeCopyHg,
                                       Properties forceOn = 0, Properties maybeOff = 0, Properties forceOff = 0, OnMissingProperties onMissing = kCopyEnsuringProperties, Properties orForceOn = (Properties)-1) {
  Properties ip = i.properties();
  if (forceOn & forceOff)
    SDL_THROW_LOG(Hypergraph.HypergraphCopyBasic, std::logic_error,
                  "ensureProperties conflict: forceOn=" << forceOn << " forceOff=" << forceOff << " overlap=" << (forceOn&forceOff));

  Properties switchOn = forceOn & ~ip;
  Properties switchOff = forceOff & ip;
  bool orSwitchOn = orForceOn & ~ip;
  if ((switchOn && orSwitchOn) || switchOff) {
    if (onMissing&kMissingThrowMask)
      SDL_THROW_LOG(Hypergraph.HypergraphCopyBasic, HypergraphPropertiesException,
                    "copy not requested for input with properties" << PrintProperties(switchOn) << " missing and with unwanted properties " << PrintProperties(switchOff) << ".");

    MutableHypergraph<A> *pm = new MutableHypergraph<A>((ip|switchOn) & ~switchOff & ~maybeOff);
    pMaybeCopyHg.reset(pm);
    copyHypergraph(i, pm, kNoClear);
    return *pm;
  }
  return i;
}


/// see docs to the other ensureProperties above. modifies input hg in place if needed (depending on 'onMissing'
template <class A>
IHypergraph<A>& ensureProperties(IHypergraph<A>& i, shared_ptr<IHypergraph<A> > &p, Properties forceOn = 0, Properties maybeOff = 0, Properties forceOff = 0, OnMissingProperties onMissing = kCopyEnsuringProperties, Properties orForceOn = (Properties)-1) {
  Properties ip = i.properties();
  if (forceOn & forceOff)
    SDL_THROW_LOG(Hypergraph.HypergraphCopyBasic, std::logic_error,
                  "ensureProperties conflict: forceOn=" << forceOn << " forceOff=" << forceOff << " overlap=" << (forceOn&forceOff));

  Properties switchOn = forceOn & ~ip;
  Properties switchOff = forceOff & ip;
  bool orSwitchOn = orForceOn & ~ip;
  if ((switchOn && orSwitchOn) || switchOff) {
    if (i.isMutable() && onMissing|kMissingModifyMask) {
      static_cast<IMutableHypergraph<A> &>(i).forcePropertiesOnOff(switchOn, switchOff);
      return i;
    } else {
      if (onMissing & kMissingThrowMask)
        SDL_THROW_LOG(Hypergraph.HypergraphCopyBasic, HypergraphPropertiesException,
                      "copy not requested for input with properties" << PrintProperties(switchOn) << " missing and with unwanted properties " << PrintProperties(switchOff) << ".");
      MutableHypergraph<A> *pm = new MutableHypergraph<A>((ip|switchOn) & ~switchOff & ~maybeOff);
      p.reset(pm);
      copyHypergraph(i, pm, kNoClear);
      return *pm;
    }

  }
  return i;
}

/// see docs to the other ensureProperties above. \return wraps input hg in a
/// non-deleting shared_ptr, or puts a copy in a deleting shared_ptr
template <class A>
shared_ptr<IHypergraph<A> const> ensureProperties(IHypergraph<A> const& i, Properties forceOn = 0, Properties maybeOff = 0, Properties forceOff = 0, OnMissingProperties onMissing = kCopyEnsuringProperties, Properties orForceOn = (Properties)-1) {
  shared_ptr<IHypergraph<A> const> p;
  ensureProperties(i, p, forceOn, maybeOff, forceOff, onMissing, orForceOn);
  if (!p)
    setNoDelete(p, i);
  return p;
}

template <class A>
shared_ptr<IHypergraph<A> const>
ensureFirstTailOutArcs(IHypergraph<A> const& hg) {
  return ensureProperties(hg,
                          hg.storesOutArcs() ? 0 : kStoreFirstTailOutArcs, kStoreInArcs);
  // also acceptable if hg had kStoreOutArcs
}

template <class A>
shared_ptr<IHypergraph<A> const>
ensureOutArcs(IHypergraph<A> const& hg) {
  return ensureProperties(hg, kStoreOutArcs,
                          kStoreInArcs | kStoreFirstTailOutArcs);
}


/// see docs to the other ensureProperties above. \return wraps input hg in a
/// non-deleting shared_ptr, or modifies input hg in place, or puts a copy in a
/// deleting shared_ptr
template <class A>
shared_ptr<IHypergraph<A> const> ensureProperties(IHypergraph<A> & i, Properties forceOn = 0, Properties maybeOff = 0, Properties forceOff = 0, OnMissingProperties onMissing = kCopyEnsuringProperties, Properties orForceOn = (Properties)-1) {
  shared_ptr<IHypergraph<A> > p;
  ensureProperties(i, p, forceOn, maybeOff, forceOff, onMissing, orForceOn);
  if (!p)
    setNoDelete(p, i);
  return boost::const_pointer_cast<IHypergraph<A> const>(p);
}

template <class A>
struct CopyFsmOutArcs
{
  typedef A Arc;
  IMutableHypergraph<A> *hg;
  StateId tailState;
  void operator()(Arc *a) const
  {
    assert(a->isFsmArc());
    Arc *arcCopy = new Arc(*a);
    arcCopy->tails()[0] = tailState;
    hg->addArc(arcCopy);
  }
};

template <class A>
void copyFsmOutArcs(StateId fromState, IMutableHypergraph<A> *hg, StateId tailState)
{
  CopyFsmOutArcs<A> copyF;
  copyF.hg = hg;
  copyF.tailState = tailState;
  hg->forArcsOut(fromState, copyF, true);
}


}}

#endif
