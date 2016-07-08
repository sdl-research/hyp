// Copyright 2014-2015 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/** \file

    'copy' arcs: take all the arcs from one hg and append them to another (used
    to concatenate two hgs), optionally through a stateid translation.

    also, more efficient fns that special-case an identity (or subset)
    StateIdTranslation
*/

#ifndef HYP__HYPERGRAPH_COPY_HPP
#define HYP__HYPERGRAPH_COPY_HPP
#pragma once


#include <sdl/Hypergraph/Exception.hpp>
#include <sdl/Hypergraph/HypergraphCopyBasic.hpp>
#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Hypergraph/StateIdTranslation.hpp>
#include <sdl/Hypergraph/Transform.hpp>
#include <sdl/Util/Once.hpp>


namespace sdl {
namespace Hypergraph {

VERBOSE_EXCEPTION_DECLARE(NonConstStateIdTranslation)

template <class A>
void copyStartFinal(StateIdTranslation& stateRemap, IHypergraph<A> const& from, IMutableHypergraph<A>* to,
                    ClearAndSameProperties clearAndSameProperties = kClearAndSameProperties) {
  if (clearAndSameProperties == kClearAndSameProperties) to->clear(from.properties());
  to->offerVocabulary(from);
  to->setStart(stateRemap.stateFor(from.start()));
  to->setFinal(stateRemap.stateFor(from.final()));
}

template <class A>
void copyStartFinalSubset(StateIdTranslation& stateRemap, IHypergraph<A> const& from, IMutableHypergraph<A>* to,
                          ClearAndSameProperties clearAndSameProperties = kClearAndSameProperties) {
  assert(stateRemap.frozen());
  if (clearAndSameProperties == kClearAndSameProperties) to->clear(from.properties());
  to->offerVocabulary(from);
  to->setStart(stateRemap.existingState(from.start()));
  to->setFinal(stateRemap.existingState(from.final()));
}

template <class A>
void copyArcs(StateIdTranslation& stateRemap, IHypergraph<A> const& i, IMutableHypergraph<A>* o,
              typename A::ArcFilter const& keep = 0) {
  if (keep)
    i.forArcs([&stateRemap, o, keep](A* a) {
      if (keep(a)) {
        A* c = stateRemap.copyArc(*a);
        if (c) o->addArc(c);
      }
    });
  else
    i.forArcs([&stateRemap, o](A* a) {
      A* c = stateRemap.copyArc(*a);
      if (c) o->addArc(c);
    });
}

enum CopyArcs { kNoArcs = 0, kArcs = 1 };
enum CopyStartFinal { kNoStartFinal = 0, kStartFinal = 1 };

template <class A>
void copySubset(StateIdTranslation& stateRemap, IHypergraph<A> const& i, IMutableHypergraph<A>* o,
                CopyArcs arcs = kArcs, CopyStartFinal startFinal = kStartFinal,
                ClearAndSameProperties clearAndSameProperties = kClearAndSameProperties) {
  stateRemap.freeze();
  if (startFinal == kStartFinal) copyStartFinalSubset(stateRemap, i, o, clearAndSameProperties);
  if (arcs == kArcs) copyArcs(stateRemap, i, o);
  stateRemap.transferLabels(i, *o);
}

template <class A>
void copyFilter(StateIdTranslation& stateRemap, IHypergraph<A> const& i, IMutableHypergraph<A>* o,
                typename A::ArcFilter const& keep = 0, CopyArcs arcs = kArcs,
                CopyStartFinal startFinal = kStartFinal,
                ClearAndSameProperties clearAndSameProperties = kClearAndSameProperties) {
  if (startFinal == kStartFinal) copyStartFinal(stateRemap, i, o, clearAndSameProperties);
  if (arcs == kArcs) copyArcs(stateRemap, i, o, keep);
  stateRemap.transferLabels(i, *o);
}

template <class A>
void copy(StateIdTranslation& stateRemap, IHypergraph<A> const& i, IMutableHypergraph<A>* o,
          CopyArcs arcs = kArcs, CopyStartFinal startFinal = kStartFinal) {
  copyFilter(stateRemap, i, o, 0, arcs, startFinal);
}


}}

#endif
