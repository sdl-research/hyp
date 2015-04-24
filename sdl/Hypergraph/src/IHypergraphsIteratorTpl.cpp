// Copyright 2014 SDL plc
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <iostream>
#include <vector>
#include <istream>

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/IHypergraphsIteratorTpl.hpp>
#include <sdl/Hypergraph/StringToHypergraph.hpp>
#include <sdl/Hypergraph/SortStates.hpp>
#include <sdl/Hypergraph/HelperFunctions.hpp>
#include <sdl/Hypergraph/ArcParserFct.hpp>
#include <sdl/Hypergraph/FeaturesPerInputPosition.hpp>

#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Split.hpp>
#include <sdl/Util/Nfc.hpp>
#include <sdl/Util/Enum.hpp>
#include <sdl/Util/LineOptions.hpp>

namespace sdl {
namespace Hypergraph {

template <class Arc>
class FlatStringHypergraphsIterator : public IHypergraphsIteratorTpl<Arc> {
  std::string line;

 public:
  FlatStringHypergraphsIterator(std::istream& in, shared_ptr<IPerThreadVocabulary> const& perThreadVocab,
                                sdl::shared_ptr<IFeaturesPerInputPosition> feats)
      : in_(in), pHg_(NULL), perThreadVocab_(perThreadVocab), done_(false), hgProp_(kStoreOutArcs) {
    opts.doAddUnknownSymbols = true;
    opts.inputFeatures = feats;
  }

  ~FlatStringHypergraphsIterator() {}

  StringToHypergraphOptions opts;

  // TODO: use full xmt/StringToHg options?
  virtual void next() {
    delete pHg_;
    pHg_ = new MutableHypergraph<Arc>(hgProp_);
    pHg_->setVocabulary(*perThreadVocab_);
    using namespace std;
    getline(in_, line);
    Util::normalizeToNfcInPlace(line, Util::kWarnUnlessNfc);
    vector<string> tokens;
    Util::splitSpaces(tokens, line);

    if (tokens.empty()) {
      delete pHg_;
      pHg_ = NULL;
      done_ = true;
    } else {
      stringToHypergraph(tokens, pHg_, opts);
      // sortStates(*pHg_); // already done by stringToHypergraph
      SDL_DEBUG(Hypergraph, "Constructed input hypergraph:\n" << *pHg_);
    }
  }

  virtual IHypergraph<Arc>* value() {
    if (pHg_ == NULL && done_ == false) { next(); }
    return static_cast<IHypergraph<Arc>*>(pHg_);
  }

  virtual void setHgProperties(Properties prop) { hgProp_ = prop; }

  virtual bool done() const { return done_; }

 private:
  std::istream& in_;
  bool done_;
  IMutableHypergraph<Arc>* pHg_;
  shared_ptr<IPerThreadVocabulary> perThreadVocab_;
  Properties hgProp_;
};

namespace {
std::string const kHgSeparator("-----");
std::string const kTextSourceName("<string>");
}

template <class Arc>
class FormattedHypergraphsIterator : public IHypergraphsIteratorTpl<Arc> {

 public:
  FormattedHypergraphsIterator(std::istream& in, shared_ptr<IPerThreadVocabulary> const& perThreadVocab)
      : in_(in), perThreadVocab_(perThreadVocab), pHg_(), done_(), hgProp_(kStoreOutArcs) {}

  ~FormattedHypergraphsIterator() {}

  virtual void next() {

    if (!in_) {
      SDL_DEBUG(Hypergraph, "next(): We're done");
      pHg_ = 0;
      done_ = true;
      return;
    }

    pHg_ = new MutableHypergraph<Arc>(hgProp_);
    pHg_->setVocabulary(*perThreadVocab_);

    Hypergraph::ParsedArcs arcs;
    readArcsUntil(in_, arcs, true);
    parsedArcsToHg(arcs, pHg_, kTextSourceName);
    SDL_DEBUG(Hypergraph, "Read full hypergraph " << *pHg_);
  }

  virtual IHypergraph<Arc>* value() {
    if (!done_ && !pHg_) next();
    return static_cast<IHypergraph<Arc>*>(pHg_);
  }

  virtual bool done() const { return done_; }

  virtual void setHgProperties(Properties prop) { hgProp_ = prop; }

 private:
  std::istream& in_;
  bool done_;
  IMutableHypergraph<Arc>* pHg_;
  shared_ptr<IPerThreadVocabulary> perThreadVocab_;
  Properties hgProp_;
};

/// we read arcs one per line because we want to recognize kHgSeparator lines
/// for inputs that are a seqence of hgs TODO: CM-485 add separator to grammar
/// and use callback objects to parse one arc at a time instead storing all
/// vector<vector<Parser::Arc> > ... (so we keep streaming and even further
/// limit temporary copy outstanding size)
void readArcsUntil(std::istream& in, ParsedArcs& arcs, bool requireNfc) {
  Util::visitChompedLinesUntil(in, kHgSeparator, impl::ParsedArcsConsumer(arcs), requireNfc);
}

template <class Arc>
IHypergraphsIteratorTpl<Arc>* IHypergraphsIteratorTpl<Arc>::create(
    std::istream& in, InputHgType inputType, shared_ptr<IPerThreadVocabulary> const& pVoc,
    shared_ptr<IFeaturesPerInputPosition> feats) {
  if (inputType == kFlatStringsHg)
    return new FlatStringHypergraphsIterator<Arc>(in, pVoc, feats);
  else if (inputType == kDashesSeparatedHg)
    return new FormattedHypergraphsIterator<Arc>(in, pVoc);  // features are already determined
  else
    SDL_THROW_LOG(Hypergraph, ConfigException, "Unsupported input type " << inputType);
};


}}
