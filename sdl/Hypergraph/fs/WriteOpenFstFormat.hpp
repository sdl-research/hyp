#ifndef HYP__HYPERGRAPH_WRITEOPENFSTFORMAT_HPP
#define HYP__HYPERGRAPH_WRITEOPENFSTFORMAT_HPP
#pragma once

#include <ostream>
#include <sdl/Hypergraph/IHypergraph.hpp>

namespace sdl {
namespace Hypergraph {

namespace WriteOpenFstFormatHelper {

template <class Arc>
struct ArcWriter {
  ArcWriter(std::ostream& out, const IHypergraph<Arc>& hg) : out_(out), hg_(hg) {}

  void operator()(Arc const* arc) const {
    LabelPair l = hg_.getFsmLabelPair(*arc);
    out_ << arc->getTail(0) << " " << arc->head() << " " << hg_.getVocabulary()->str(input(l)) << " "
         << hg_.getVocabulary()->str(output(l)) << " " << arc->weight() << '\n';
  }

  std::ostream& out_;
  const IHypergraph<Arc>& hg_;
};
}

template <class Arc>
std::ostream& writeOpenFstFormat(std::ostream& out, const IHypergraph<Arc>& hg) {
  shared_ptr<IHypergraph<Arc> const> hp = ensureProperties(hg, kFsm | kStoreOutArcs);
  if (!hg.isFsm()) SDL_THROW_LOG(Hypergraph, InvalidInputException, "WriteOpenFstFormat needs FSM input");
  typedef WriteOpenFstFormatHelper::ArcWriter<Arc> Writer;
  Writer writer(out, *hp);
  StateId st = hp->start();
  if (st == kNoState) SDL_THROW_LOG(Hypergraph, InvalidInputException, "WriteOpenFstFormat needs start state");
  hp->forArcsOutFirstTail(st, writer);
  for (StateId s = 0; s < st; ++s) hp->forArcsOutFirstTail(s, writer);
  for (StateId s = st + 1, N = hp->size(); s < N; ++s) hp->forArcsOutFirstTail(s, writer);
  out << hp->final() << '\n';
  return out;
}


}}

#endif
