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
#ifndef SDL_SIMPLECRF_CREATESEARCHSPACE_HPP
#define SDL_SIMPLECRF_CREATESEARCHSPACE_HPP
#pragma once

#include <string>
#include <set>
#include <sstream>
#include <boost/functional/hash.hpp>
#include <thread>
#include <mutex>

#include <sdl/Util/Input.hpp>

#include <sdl/Util/StringBuilder.hpp>
#include <sdl/Config/ConfigureYaml.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Vocabulary/HelperFunctions.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/StringToHypergraph.hpp>
#include <sdl/Hypergraph/Compose.hpp>
#include <sdl/Hypergraph/SortArcs.hpp>
#include <sdl/Hypergraph/SortStates.hpp>
#include <sdl/Optimization/ICreateSearchSpace.hpp>
#include <sdl/Optimization/IInput.hpp>
#include <sdl/Optimization/FeatureHypergraphPairs.hpp>
#include <sdl/Optimization/ExternalFeatHgPairs.hpp>
#include <sdl/Optimization/Types.hpp>
#include <sdl/Util/Unordered.hpp>
#include <sdl/Util/Sleep.hpp>
#include <graehl/shared/thread_group.hpp>
#include <functional>

namespace sdl {
namespace CrfDemo {

SDL_ENUM(TransitionModelType, 4, (Unigram, Bigram, Hierarchical, Bihierarchical));

struct CrfDemoConfig {

  template <class Config>
  void configure(Config& config) {
    config("Options for the CrfDemo search space");
    config.is("CrfDemo::CreateSearchSpace");

    config("conll-path", &conllPath)("Name of CoNLL training file");
    config("meaningful-feature-names", &meaningfulFeatureNames)("Use meaningful feature names").init(false);
    config("fst-compose", &fstCompose)("Use FST compose").init(true);
    config("labels-path", &labelsPath)("Labels file path");
    config("labels-per-pos-path", &labelsPerPosPath)("Labels-per-pos file path");
    config("transition-model", &transitionModel)("Transition model").init(kBigram);
    config("num-features", &numFeatures)("Number of features (feature hashing trick)").init(500000);
    config("num-threads", &numThreads)("Number of threads for feature extraction").init(4);

    config("read-train-archive-path", &readTrainArchivePath)("Path to read training archive");
    config("write-train-archive-path", &writeTrainArchivePath)("Path to write training archive");
  }

  std::string conllPath, labelsPath, labelsPerPosPath;
  std::string readTrainArchivePath, writeTrainArchivePath;
  bool meaningfulFeatureNames, fstCompose;
  TransitionModelType transitionModel;
  Hypergraph::FeatureId numFeatures;
  std::size_t numThreads;
};

template <class A>
class CreateSearchSpace : public Optimization::ICreateSearchSpace<A> {
  typedef A Arc;
  typedef typename Arc::Weight Weight;
  typedef Optimization::IFeatureHypergraphPairs<Arc> Pairs;
  typedef typename Pairs::value_type value_type;
  typedef typename Pairs::IHgPtr IHgPtr;
  typedef Optimization::TrainingDataIndex TrainingDataIndex;
  typedef unordered_map<Sym, std::set<Sym>> LabelsPerPosMap;

 public:
  CreateSearchSpace(ConfigNode const& yamlConfig)
      : pVoc_(Vocabulary::createDefaultVocab()), testMode_(false) {
    Config::applyYaml(yamlConfig, &opts_);
  }

  ///

  void readLabelsFile(std::string const& fname) {
    SDL_INFO(CrfDemo, "Reading labels file '" << fname << "'");
    Util::Input input(fname);
    std::string label;
    while (std::getline(*input, label)) {
      allLabels_.insert(pVoc_->add(label, kTerminal));
    }
  }

  void writeLabelsFile(std::string const& fname) {
    SDL_INFO(CrfDemo, "Writing labels file '" << fname << "'");
    Util::Output output(fname);
    for (Sym labelId : allLabels_) {
      *output << pVoc_->str(labelId) << '\n';
    }
  }

  void readLabelsPerPosFile(std::string const& fname) {
    SDL_INFO(CrfDemo, "Reading labels-per-pos file '" << fname << "'");
    Util::Input input(fname);
    std::string line;
    while (std::getline(*input, line)) {
      std::stringstream ss(line);
      std::string pos, label;
      ss >> pos;
      std::set<Sym>& s = labelsPerPos_[pVoc_->add(pos, kTerminal)];
      while (ss >> label) {
        s.insert(pVoc_->add(label, kTerminal));
      }
    }
  }

  void writeLabelsPerPosFile(std::string const& fname) {
    SDL_INFO(CrfDemo, "Writing labels-per-pos file '" << fname << "'");
    Util::Output output(fname);
    for (LabelsPerPosMap::const_iterator it = labelsPerPos_.begin(); it != labelsPerPos_.end(); ++it) {
      *output << pVoc_->str(it->first);
      for (Sym labelId : it->second) {
        *output << "\t" << pVoc_->str(labelId);
      }
      *output << '\n';
    }
  }

  ///

  Hypergraph::IMutableHypergraph<Arc>* getUnclampedHypergraph(Optimization::IInput const& observedInput) {
    return NULL;
  }

  shared_ptr<Optimization::IFeatureHypergraphPairs<Arc>> getFeatureHypergraphPairs() const { return pairs_; }

  std::size_t getNumFeatures() { return opts_.numFeatures; }

  void setTestMode() {
    if (opts_.labelsPath.empty()) {
      SDL_THROW_LOG(CrfDemo, InvalidInputException, "Need labels-path option");
    }
    readLabelsFile(opts_.labelsPath);
    readLabelsPerPosFile(opts_.labelsPerPosPath);
    testMode_ = true;
  }

  /////////////////////////////////////////////////////////////////
  struct Sentence {
    std::vector<Sym> words, poss, labels;

    void clear() {
      words.clear();
      poss.clear();
      labels.clear();
    }

    void add(std::string const& line, std::set<Sym>* allLabels, IVocabularyPtr& pVoc, bool testMode) {
      std::stringstream ss(line);
      std::string word, pos, label;
      ss >> word >> pos >> label;
      if (word.empty() || pos.empty() || label.empty()) {
        SDL_THROW_LOG(CrfDemo, InvalidInputException, "Bad line: " << line);
      }
      words.push_back(pVoc->add(word, kTerminal));
      poss.push_back(pVoc->add(pos, kTerminal));
      labels.push_back(pVoc->add(label, kTerminal));
      SDL_DEBUG(CrfDemo, "id(" << word << "): " << words.back());
      SDL_DEBUG(CrfDemo, "id(" << pos << "): " << poss.back());
      SDL_DEBUG(CrfDemo, "id(" << label << "): " << labels.back());
      if (!testMode) {
        allLabels->insert(labels.back());
      }
    }
  };
  /////////////////////////////////////////////////////////////////

  Hypergraph::FeatureId getFeatureId(std::string const& featName) const {
    std::size_t id = 0;
    boost::hash_combine(id, featName);
    id = id % opts_.numFeatures;
    SDL_TRACE(CrfDemo, "getFeatId(" << featName << "): " << id);
    return id;
  }

  std::string createFeatureName(std::string const& prefix, Sym sym) const {
    if (opts_.meaningfulFeatureNames)
      return Util::StringBuilder(prefix)("_")(pVoc_->str(sym)).str();
    else
      return Util::StringBuilder(prefix)("_")(sym).str();
  }

  std::string createFeatureName(std::string const& prefix, Sym sym1, Sym sym2) const {
    if (opts_.meaningfulFeatureNames)
      return Util::StringBuilder(prefix)("_")(pVoc_->str(sym1))("_")(pVoc_->str(sym2)).str();
    else
      return Util::StringBuilder(prefix)("_")(sym1)("_")(sym2).str();
  }

  std::string createFeatureName(std::string const& prefix, Sym sym1, Sym sym2, Sym sym3) const {
    if (opts_.meaningfulFeatureNames)
      return Util::StringBuilder(prefix)("_")(pVoc_->str(sym1))("_")(pVoc_->str(sym2))("_")(pVoc_->str(sym3))
          .str();
    else
      return Util::StringBuilder(prefix)("_")(sym1)("_")(sym2)("_")(sym3).str();
  }

  std::string createFeatureName(std::string const& prefix, std::string const& sym1, std::string const& sym2,
                                std::string const& sym3) const {
    return Util::StringBuilder(prefix)("_")(sym1)("_")(sym2)("_")(sym3).str();
  }

  std::string createFeatureName(std::string const& prefix, std::string const& sym1,
                                std::string const& sym2) const {
    return Util::StringBuilder(prefix)("_")(sym1)("_")(sym2).str();
  }

  std::string createFeatureName(std::string const& prefix, std::string const& sym1) const {
    return Util::StringBuilder(prefix)("_")(sym1).str();
  }

  Hypergraph::IHypergraph<Arc>* createUnigramModel() const {
    using namespace Hypergraph;
    IMutableHypergraph<Arc>* model = new MutableHypergraph<Arc>();
    model->setVocabulary(pVoc_);
    model->addState();
    model->setStart(0);
    model->setFinal(0);
    for (Sym sym : allLabels_) {
      model->addArc(new Arc(Head(0), Tails(0, model->addState(sym)), Weight::one()));
    }
    sortArcs(model);
    return model;
  }

  Hypergraph::IHypergraph<Arc>* createBigramModel() const {
    using namespace Hypergraph;
    IMutableHypergraph<Arc>* model = new MutableHypergraph<Arc>();
    model->setVocabulary(pVoc_);

    std::vector<Sym> historyNames;  // just for more meaningful feature names
    historyNames.reserve(allLabels_.size() + 1);
    historyNames.push_back(EPSILON::ID);
    for (Sym label : allLabels_) {
      historyNames.push_back(label);
    }

    std::size_t numStates = allLabels_.size() + 2;
    for (std::size_t i = 0; i < numStates; ++i) {
      model->addState();
    }
    model->setStart(0);
    StateId finalState = numStates - 1;
    model->setFinal(finalState);

    for (StateId s = 0; s < finalState; ++s) {
      std::set<Sym>::const_iterator symIter = allLabels_.begin();
      for (StateId t = 1; t < finalState; ++t, ++symIter) {
        Weight weight(0.0f);
        weight.insert(getFeatureId(createFeatureName("bi", historyNames[s], historyNames[t])), 1.0f);
        model->addArc(new Arc(Head(t), Tails(s, model->addState(*symIter)), weight));

        // Final arcs
        {
          Weight weight(0.0f);
          weight.insert(getFeatureId(createFeatureName("bi", historyNames[s], historyNames[t])),
                        1.0f);  // s, then t
          weight.insert(getFeatureId(createFeatureName("end", historyNames[t])), 1.0f);  // t, then end
          model->addArc(new Arc(Head(finalState), Tails(s, model->addState(*symIter)), weight));
        }
      }
    }
    sortArcs(model);
    SDL_DEBUG(CrfDemo, "Bigram model: " << *model);
    return model;
  }

  // shortcut
  Sym sym(std::string const& str) { return pVoc_->add(str, kNonterminal); }
  Sym termSym(std::string const& str) { return pVoc_->add(str, kTerminal); }

  Hypergraph::IHypergraph<Arc>* createHierarchicalModel() {
    using namespace Hypergraph;
    IMutableHypergraph<Arc>* model = new MutableHypergraph<Arc>();
    model->forceCanonicalLex();
    model->setVocabulary(pVoc_);
    // model->setStart(model->addState());

    std::set<Sym> nonterminals;  // NP, VP, ...
    for (Sym label : allLabels_) {
      std::string const& str = pVoc_->str(label);
      if (str.length() > 2 && (str[0] == 'I' || str[0] == 'B') && str[1] == '-') {
        nonterminals.insert(sym(str.substr(2)));
      }
    }

    unordered_map<Sym, StateId> stateId;

    // (NP) <- (B-NP)
    // (NP) <- (B-NP) (NP0)
    // (NP0) <- (I-NP)
    // (NP0) <- (I-NP) (NP0)
    for (Sym nt : nonterminals) {
      std::string const& ntStr = pVoc_->str(nt);
      stateId[nt] = model->addState(nt);

      Sym b = termSym("B-" + ntStr);
      stateId[b] = model->addState(b);

      Sym i = termSym("I-" + ntStr);
      stateId[i] = model->addState(i);

      Sym nt0 = sym(ntStr + "0");
      stateId[nt0] = model->addState(nt0);

      Weight weight1(0.0f);
      weight1.insert(getFeatureId(createFeatureName("short", nt)), 1.0f);
      model->addArc(new Arc(Head(stateId[nt]), Tails(stateId[b]), weight1));  // (NP) <- (B-NP)

      Weight weight2(0.0f);
      weight2.insert(getFeatureId(createFeatureName("long", nt)), 1.0f);
      model->addArc(
          new Arc(Head(stateId[nt]), Tails(stateId[b], stateId[nt0]), weight2));  // (NP) <- (B-NP) (NP0)

      Weight weight3(0.0f);
      weight3.insert(getFeatureId(createFeatureName("short_sub", nt)), 1.0f);
      model->addArc(new Arc(Head(stateId[nt0]), Tails(stateId[i]), weight3));  // (NP0) <- (I-NP)

      Weight weight4(0.0f);
      weight4.insert(getFeatureId(createFeatureName("long_sub", nt)), 1.0f);
      model->addArc(new Arc(Head(stateId[nt0]), Tails(stateId[i], stateId[nt0]), weight4));
    }

    Sym O = termSym("O");  // similar to "B-NP", etc.: it's a terminal symbol
    nonterminals.insert(O);
    stateId[O] = model->addState(O);

    // add (nt1+nt2) states
    for (Sym nt1 : nonterminals) {
      std::string const& nt1Str = pVoc_->str(nt1);
      for (Sym nt2 : nonterminals) {
        std::string const& nt2Str = pVoc_->str(nt2);
        Sym nt12 = sym(nt1Str + "+" + nt2Str);
        stateId[nt12] = model->addState(nt12);
      }
    }

    // Trigram model over nonterminals:
    // (NP+VP) <- (ADJ+NP) (VP)
    // ...
    for (Sym nt1 : nonterminals) {
      std::string const& nt1Str = pVoc_->str(nt1);
      for (Sym nt2 : nonterminals) {
        std::string const& nt2Str = pVoc_->str(nt2);
        Sym nt12 = sym(nt1Str + "+" + nt2Str);
        for (Sym nt3 : nonterminals) {
          std::string const& nt3Str = pVoc_->str(nt3);
          Sym nt23 = sym(nt2Str + "+" + nt3Str);
          Weight weight(0.0f);
          weight.insert(getFeatureId(createFeatureName("tri_nt", nt1Str, nt2Str, nt3Str)), 1.0f);
          weight.insert(getFeatureId(createFeatureName("bi_nt", nt2Str, nt3Str)), 1.0f);
          weight.insert(getFeatureId(createFeatureName("uni_nt", nt3Str)), 1.0f);
          model->addArc(new Arc(Head(stateId[nt23]), Tails(stateId[nt12], stateId[nt3]), weight));
        }
      }
    }

    // (NP+VP) <- (eps+NP) (VP)
    for (Sym nt1 : nonterminals) {
      std::string const& nt1Str = pVoc_->str(nt1);

      Sym epsNt1 = sym("eps+" + nt1Str);
      stateId[epsNt1] = model->addState(epsNt1);

      for (Sym nt2 : nonterminals) {
        std::string const& nt2Str = pVoc_->str(nt2);
        Sym nt12 = sym(nt1Str + "+" + nt2Str);
        Weight weight(0.0f);
        weight.insert(getFeatureId(createFeatureName("tri_nt", "eps", nt1Str, nt2Str)), 1.0f);
        weight.insert(getFeatureId(createFeatureName("bi_nt", nt1Str, nt2Str)), 1.0f);
        weight.insert(getFeatureId(createFeatureName("uni_nt", nt2Str)), 1.0f);
        model->addArc(new Arc(Head(stateId[nt12]), Tails(stateId[epsNt1], stateId[nt2]), weight));
      }
    }

    // (eps+NP) <- START (NP)
    // (eps+VP) <- START (VP)
    // ...
    for (Sym nt : nonterminals) {
      std::string const& ntStr = pVoc_->str(nt);
      Sym epsNt = sym("eps+" + ntStr);
      Weight weight(0.0f);
      weight.insert(getFeatureId(createFeatureName("bi_nt", "eps", ntStr)), 1.0f);
      weight.insert(getFeatureId(createFeatureName("uni_nt", ntStr)), 1.0f);
      model->addArc(new Arc(Head(stateId[epsNt]), Tails(stateId[nt]), weight));
    }

    Sym final0 = sym("FINAL0");
    stateId[final0] = model->addState(final0);

    // (FINAL0) <- (NP+VP)
    for (Sym nt1 : nonterminals) {
      std::string const& nt1Str = pVoc_->str(nt1);
      for (Sym nt2 : nonterminals) {
        std::string const& nt2Str = pVoc_->str(nt2);
        Sym nt12 = sym(nt1Str + "+" + nt2Str);
        Weight weight(0.0f);
        weight.insert(getFeatureId(createFeatureName("tri_nt", nt1Str, nt2Str, "FINAL")), 1.0f);
        weight.insert(getFeatureId(createFeatureName("bi_nt", nt2Str, "FINAL")), 1.0f);
        weight.insert(getFeatureId(createFeatureName("uni_nt", "FINAL")), 1.0f);
        model->addArc(new Arc(Head(stateId[final0]), Tails(stateId[nt12]), weight));
      }
    }

    // (FINAL0) <- (eps+NP)
    for (Sym nt1 : nonterminals) {
      std::string const& nt1Str = pVoc_->str(nt1);
      Sym epsNt1 = sym("eps+" + nt1Str);
      Weight weight(0.0f);
      weight.insert(getFeatureId(createFeatureName("tri_nt", "eps", nt1Str, "FINAL")), 1.0f);
      weight.insert(getFeatureId(createFeatureName("bi_nt", nt1Str, "FINAL")), 1.0f);
      weight.insert(getFeatureId(createFeatureName("uni_nt", "FINAL")), 1.0f);
      model->addArc(new Arc(Head(stateId[final0]), Tails(stateId[epsNt1]), weight));
    }

    model->setFinal(stateId[final0]);

    // sortArcs(model); // doesn't work
    SDL_DEBUG(CrfDemo, "Hierarchical model: " << *model);

    return model;
  }

  Hypergraph::IHypergraph<Arc>* createBihierarchicalModel() {
    using namespace Hypergraph;
    IHypergraph<Arc>* hier = createHierarchicalModel();
    IHypergraph<Arc>* bi = createBigramModel();
    IMutableHypergraph<Arc>* model = new MutableHypergraph<Arc>();
    Hypergraph::compose(*hier, *bi, model);
    delete hier;
    delete bi;
    return model;
  }

  void addLabelArc(Hypergraph::IMutableHypergraph<Arc>* hg, Sym word, Sym pos, Sym label,
                   Hypergraph::StateId from, Hypergraph::StateId to) const {
    Weight weight(0.0f);
    weight.insert(getFeatureId(createFeatureName("word+label", word, label)), 1.0f);
    weight.insert(getFeatureId(createFeatureName("pos+label", pos, label)), 1.0f);
    weight.insert(getFeatureId(createFeatureName("label", label)), 1.0f);

    using namespace Hypergraph;
    hg->addArc(new Arc(Head(to), Tails(from, hg->addState(label)), weight));
  }

  enum HypergraphType { kClamped = 1, kUnclamped = 2 };

  Hypergraph::IHypergraph<Arc>* createSearchSpace(Sentence const& sent,
                                                  Hypergraph::IHypergraph<Arc> const& transitionModel,
                                                  HypergraphType hgType) const {
    using namespace Hypergraph;
    IMutableHypergraph<Arc>* result = new MutableHypergraph<Arc>(kFsmOutProperties);
    result->setVocabulary(pVoc_);
    // Add all structural (non-lexical) states first so they have lower IDs:
    for (std::size_t i = 0; i <= sent.words.size(); ++i) {
      result->addState();
    }
    result->setStart(0);

    for (std::size_t i = 0; i < sent.words.size(); ++i) {
      if (hgType == kClamped) {  // add the observed (gold) label (clamped, nominator)
        addLabelArc(result, sent.words[i], sent.poss[i], sent.labels[i], i, i + 1);
      } else {
        // for (Sym label : allLabels_) { // add all possible labels (unclamped, denominator)
        std::set<Sym> const* labels = &allLabels_;
        LabelsPerPosMap::const_iterator iter = labelsPerPos_.find(sent.poss[i]);
        if (iter != labelsPerPos_.end()) {
          labels = &(iter->second);
        }
        for (Sym label : *labels) {
          addLabelArc(result, sent.words[i], sent.poss[i], label, i, i + 1);
        }
      }
    }

    result->setFinal(sent.words.size());
    IMutableHypergraph<Arc>* composed
        = new MutableHypergraph<Arc>(kCanonicalLex | kStoreOutArcs | kStoreInArcs);

    if (opts_.fstCompose) {
      ComposeTransformOptions cOpt;
      cOpt.fstCompose = true;
      Hypergraph::fs::compose(*result, (IMutableHypergraph<Arc> const&)transitionModel, composed, cOpt);
    } else {
      Hypergraph::compose(transitionModel, *result, composed);
    }

    delete result;
    pruneUnreachable(composed);
    sortStates(*composed);

    return composed;
  }

  void processTrainingSentence(Sentence const& sent, Hypergraph::IHypergraph<Arc> const& transitionModel,
                               Pairs& pairs) {
    Hypergraph::IHypergraph<Arc>* clamped = createSearchSpace(sent, transitionModel, kClamped);
    Hypergraph::IHypergraph<Arc>* unclamped = createSearchSpace(sent, transitionModel, kUnclamped);
    std::lock_guard<std::mutex> lock(mutex_);
    pairs.push_back(value_type(IHgPtr(clamped), IHgPtr(unclamped)));
  }

  ///

  /// Threads can call this functor
  struct ProcessTrainingExampleRange {

    typedef CreateSearchSpace<A> Outer;
    ProcessTrainingExampleRange(ProcessTrainingExampleRange const&) = delete;
    ProcessTrainingExampleRange(Outer& o, std::size_t begin, std::size_t end,
                                std::vector<Sentence> const& sents,
                                Hypergraph::IHypergraph<A> const& transitionModel,
                                typename Outer::Pairs& trainingExamples, bool doLogProgress)
        : outer_(o)
        , begin_(begin)
        , end_(end)
        , sents_(sents)
        , transitionModel_(transitionModel)
        , trainingExamples_(trainingExamples)
        , doLogProgress_(doLogProgress) {}

    void operator()() {
      SDL_DEBUG(Optimization.ProcessTrainingExampleRange, "Processing sentences " << begin_ << " to " << end_);
      for (std::size_t i = begin_; i < end_; ++i) {
        SDL_DEBUG(Optimization.ProcessTrainingExampleRange, "Processing sentence " << i);
        outer_.processTrainingSentence(sents_[i], transitionModel_, trainingExamples_);

        if (doLogProgress_) {
          std::size_t blockSize = (end_ - begin_) / 10.0f;
          if (blockSize && (i - begin_ + 1) % blockSize == 0) {
            SDL_INFO(CrfDemo, (((i - begin_ + 1.0f) / (end_ - begin_)) * 100.0f) << "% processed");
          }
        }
      }
    }

    Outer& outer_;
    std::size_t begin_, end_;
    std::vector<Sentence> const& sents_;
    Hypergraph::IHypergraph<A> const& transitionModel_;
    typename Outer::Pairs& trainingExamples_;
    bool doLogProgress_;
  };

  ///

  void prepareTraining() {
    Util::Performance performance("CrfDemo.prepareTraining", std::cerr);

    if (!opts_.writeTrainArchivePath.empty()) {  // Write archive?
      pairs_.reset(new Optimization::WriteFeatureHypergraphPairs<Arc>(opts_.writeTrainArchivePath));
    } else if (!opts_.readTrainArchivePath.empty()) {  // Read archive?
      pairs_.reset(new Optimization::ExternalFeatHgPairs<Arc>(opts_.readTrainArchivePath));
      return;
    } else {  // Construct HGs
      pairs_.reset(new Optimization::InMemoryFeatureHypergraphPairs<Arc>());
    }

    SDL_INFO(CrfDemo, "Loading " + opts_.conllPath);
    std::vector<Sentence> sents;
    std::string line;
    Sentence sent;
    Util::Input input(opts_.conllPath);
    while (std::getline(*input, line)) {
      if (line.empty()) {
        sents.push_back(sent);
        sent.clear();
      } else {
        sent.add(line, &allLabels_, pVoc_, testMode_);
      }
    }
    assert(!allLabels_.empty());
    SDL_INFO(CrfDemo, "Read " << sents.size() << " training examples");
    SDL_INFO(CrfDemo, "Found " << allLabels_.size() << " labels");

    // For pruning
    for (Sentence const& sent : sents) {
      for (std::size_t i = 0; i < sent.poss.size(); ++i) {
        labelsPerPos_[sent.poss[i]].insert(sent.labels[i]);
      }
    }

    Hypergraph::IHypergraph<Arc>* transitionModel
        = opts_.transitionModel == kUnigram
              ? createUnigramModel()
              : opts_.transitionModel == kBigram ? createBigramModel() : opts_.transitionModel == kHierarchical
                                                                             ? createHierarchicalModel()
                                                                             : createBihierarchicalModel();
    SDL_INFO(CrfDemo, "Extracting features");

    if (opts_.numThreads < 2) {
      std::size_t cnt = 0;
      for (Sentence const& sent : sents) {
        processTrainingSentence(sent, *transitionModel, *pairs_);
      }
    } else {  // multi-threaded:
      std::size_t numProducers = opts_.numThreads;
      graehl::thread_group producer_threads;

      std::size_t size = sents.size();
      std::size_t blockSize = size / numProducers;
      std::size_t numProducerThreads = (numProducers * blockSize == size) ? numProducers : numProducers + 1;

      typedef Util::AutoDeleteAll<ProcessTrainingExampleRange> Producers;
      Producers producers;
      producers.reserve(numProducerThreads);
      ProcessTrainingExampleRange* update;
      for (std::size_t i = 0; i < numProducerThreads; ++i) {
        TrainingDataIndex begin = i * blockSize;
        TrainingDataIndex end = (i == numProducerThreads - 1) ? size : (i + 1) * blockSize;
        producers.push_back(update = new ProcessTrainingExampleRange(*this, begin, end, sents,
                                                                     *transitionModel, *pairs_, i == 0));
        producer_threads.create_thread(std::ref(*update));
      }
      producer_threads.join_all();
    }

    delete transitionModel;
    SDL_INFO(CrfDemo, "Number of features: " << opts_.numFeatures);

    if (!testMode_) {
      writeLabelsFile(opts_.labelsPath);
      writeLabelsPerPosFile(opts_.labelsPerPosPath);
    }

    pairs_->setNumFeatures(getNumFeatures());
    pairs_->finish();
    SDL_DEBUG(CrfDemo, "Found " << pairs_->size() << " training examples.");

    if (!opts_.writeTrainArchivePath.empty()
        && !opts_.readTrainArchivePath.empty()) {  // Read after we've just written
      Util::sleepSeconds(1);  // probably not needed
      pairs_.reset(new Optimization::ExternalFeatHgPairs<Arc>(opts_.readTrainArchivePath));
    }
  }

  std::string getName() const { return "CrfDemo"; }

 private:
  IVocabularyPtr pVoc_;
  shared_ptr<Optimization::IFeatureHypergraphPairs<Arc>> pairs_;
  CrfDemoConfig opts_;
  std::set<Sym> allLabels_;
  LabelsPerPosMap labelsPerPos_;
  bool testMode_;
  std::mutex mutex_;
};


}}

#endif
