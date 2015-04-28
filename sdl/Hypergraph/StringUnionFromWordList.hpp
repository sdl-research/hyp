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

    options for reading a word list and creating a stringUnion. shared between
    HgTrie and StatisticalTokenizerTrain.
*/

#ifndef BUILDSTRINGUNION_JG2013213_HPP
#define BUILDSTRINGUNION_JG2013213_HPP
#pragma once

#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/StringUnion.hpp>
#include <sdl/Hypergraph/WordList.hpp>
#include <sdl/Vocabulary/HelperFunctions.hpp>
#include <sdl/SharedPtr.hpp>

namespace sdl {
namespace Hypergraph {

struct StringUnionFromWordList {
  WordListOptions wordlist;
  StringUnionOptions stringUnion;

  std::string usageSuffix() const {
    return " from "+wordlist.usage();
  }

  template <class Config>
  void configure(Config &config) {
    stringUnion.configure(config);
    wordlist.configure(config);
  }

  template <class Arc>
  void build(std::istream &in, MutableHypergraph<Arc> *outHg) const {
    DoNothing f;
    build(in, outHg, f);
  }

  template <class Arc, class FinishBuilding>
  void build(std::istream &in, MutableHypergraph<Arc> *outHg, FinishBuilding &finish) const {
    IVocabularyPtr vocab = outHg->getVocabulary();
    typedef typename Arc::Weight Weight;
    WeightedStrings<Weight> ws(vocab ? vocab : Vocabulary::createDefaultVocab());

    readWordList(in, ws, wordlist, stringUnion.unweighted);

    outHg->clear(kFsmOutProperties);

    BuildStringUnion<Arc> build(ws, *outHg, stringUnion); //sets vocab
    finish(build);
  }
};


}}

#endif
