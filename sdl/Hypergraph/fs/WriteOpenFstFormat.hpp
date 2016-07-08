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
#ifndef HYP__HYPERGRAPH_WRITEOPENFSTFORMAT_HPP
#define HYP__HYPERGRAPH_WRITEOPENFSTFORMAT_HPP
#pragma once

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <ostream>

namespace sdl {
namespace Hypergraph {

namespace WriteOpenFstFormatHelper {

template <class Arc>
struct ArcWriter {
  ArcWriter(std::ostream& out, IHypergraph<Arc> const& hg) : out_(out), hg_(hg) {}

  void operator()(Arc const* arc) const {
    LabelPair l = hg_.getFsmLabelPair(*arc);
    IVocabulary& voc = *hg_.vocab();
    out_ << arc->getTail(0) << " " << arc->head() << " " << voc.str(input(l)) << " " << voc.str(output(l))
         << " " << arc->weight() << '\n';
  }

  std::ostream& out_;
  IHypergraph<Arc> const& hg_;
};
}

template <class Arc>
std::ostream& writeOpenFstFormat(std::ostream& out, IHypergraph<Arc> const& hg) {
  shared_ptr<IHypergraph<Arc> const> hp = ensureProperties(hg, kFsm | kGraph | kStoreOutArcs);
  if (!hg.isFsm()) SDL_THROW_LOG(Hypergraph, InvalidInputException, "WriteOpenFstFormat needs FSM input");
  typedef WriteOpenFstFormatHelper::ArcWriter<Arc> Writer;
  Writer writer(out, *hp);
  StateId st = hp->start();
  if (st == kNoState)
    SDL_THROW_LOG(Hypergraph, InvalidInputException, "WriteOpenFstFormat needs start state");
  hp->forArcsOutFirstTail(st, writer);
  for (StateId s = 0; s < st; ++s) hp->forArcsOutFirstTail(s, writer);
  for (StateId s = st + 1, N = hp->size(); s < N; ++s) hp->forArcsOutFirstTail(s, writer);
  out << hp->final() << '\n';
  return out;
}


}}

#endif
