



#include <string>
#include <set>
#include <sstream>
#include <boost/functional/hash.hpp>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>





















namespace CrfDemo {



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

template<class A>
class CreateSearchSpace : public Optimization::ICreateSearchSpace<A> {
  typedef A Arc;
  typedef typename Arc::Weight Weight;
  typedef Optimization::IFeatureHypergraphPairs<Arc> Pairs;
  typedef typename Pairs::value_type value_type;
  typedef typename Pairs::IHgPtr IHgPtr;
  typedef Optimization::TrainingDataIndex TrainingDataIndex;


 public:
  CreateSearchSpace(ConfigNode const& yamlConfig)
      : pVoc_(Vocabulary::createDefaultVocab())
      , testMode_(false) {
    Config::applyYaml(yamlConfig, &opts_);
  }

  ///

  void readLabelsFile(std::string const& fname) {

    Util::Input input(fname);
    std::string label;
    while (std::getline(*input, label)) {

    }
  }

  void writeLabelsFile(std::string const& fname) {

    Util::Output output(fname);


    }
  }

  void readLabelsPerPosFile(std::string const& fname) {

    Util::Input input(fname);
    std::string line;
    while (std::getline(*input, line)) {
      std::stringstream ss(line);
      std::string pos, label;
      ss >> pos;

      while (ss >> label) {

      }
    }
  }

  void writeLabelsPerPosFile(std::string const& fname) {

    Util::Output output(fname);
    for (LabelsPerPosMap::const_iterator it = labelsPerPos_.begin();
         it != labelsPerPos_.end(); ++it) {



      }

    }
  }

  ///

  Hypergraph::IMutableHypergraph<Arc>*
  getUnclampedHypergraph(Optimization::IInput const& observedInput) {
    return NULL;
  }

  shared_ptr< Optimization::IFeatureHypergraphPairs<Arc> >
  getFeatureHypergraphPairs() const {
    return pairs_;
  }

  std::size_t getNumFeatures() { return opts_.numFeatures; }

  void setTestMode() {
    if (opts_.labelsPath.empty()) {

    }
    readLabelsFile(opts_.labelsPath);
    readLabelsPerPosFile(opts_.labelsPerPosPath);
    testMode_ = true;
  }

  /////////////////////////////////////////////////////////////////
  struct Sentence {


    void clear() {
      words.clear(); poss.clear(); labels.clear();
    }


             , IVocabularyPtr& pVoc, bool testMode) {
      std::stringstream ss(line);
      std::string word, pos, label;
      ss >> word >> pos >> label;
      if (word.empty() || pos.empty() || label.empty()) {

      }






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

    return id;
  }


    if (opts_.meaningfulFeatureNames)

    else
      return Util::StringBuilder(prefix)("_")(sym).str();
  }


    if (opts_.meaningfulFeatureNames)

    else
      return Util::StringBuilder(prefix)("_")(sym1)("_")(sym2).str();
  }

  std::string createFeatureName(std::string const& prefix,

    if (opts_.meaningfulFeatureNames)

    else
      return Util::StringBuilder(prefix)("_")(sym1)("_")(sym2)("_")(sym3).str();
  }

  std::string createFeatureName(std::string const& prefix,
                                std::string const& sym1,
                                std::string const& sym2,
                                std::string const& sym3) const {
    return Util::StringBuilder(prefix)("_")(sym1)("_")(sym2)("_")(sym3).str();
  }

  std::string createFeatureName(std::string const& prefix,
                                std::string const& sym1,
                                std::string const& sym2) const {
    return Util::StringBuilder(prefix)("_")(sym1)("_")(sym2).str();
  }

  std::string createFeatureName(std::string const& prefix,
                                std::string const& sym1) const {
    return Util::StringBuilder(prefix)("_")(sym1).str();
  }

  Hypergraph::IHypergraph<Arc>* createUnigramModel() const {
    using namespace Hypergraph;
    IMutableHypergraph<Arc>* model = new MutableHypergraph<Arc>();
    model->setVocabulary(pVoc_);
    model->addState();



      model->addArc(new Arc(Head(0), Tails(0, model->addState(sym)), Weight::one()));
    }
    sortArcs(model);
    return model;
  }

  Hypergraph::IHypergraph<Arc>* createBigramModel() const {
    using namespace Hypergraph;
    IMutableHypergraph<Arc>* model = new MutableHypergraph<Arc>();
    model->setVocabulary(pVoc_);


    historyNames.reserve(allLabels_.size() + 1);
    historyNames.push_back(EPSILON::ID);

      historyNames.push_back(label);
    }

    std::size_t numStates = allLabels_.size() + 2;
    for (std::size_t i = 0; i < numStates; ++i) {
      model->addState();
    }

    StateId finalState = numStates - 1;


    for (StateId s = 0; s < finalState; ++s) {

      for (StateId t = 1; t < finalState; ++t, ++symIter) {
        Weight weight(0.0f);
        weight.insert(getFeatureId(createFeatureName("bi", historyNames[s], historyNames[t])),
                      1.0f);
        model->addArc(new Arc(Head(t), Tails(s, model->addState(*symIter)), weight));

        // Final arcs
        {
          Weight weight(0.0f);
          weight.insert(getFeatureId(createFeatureName("bi", historyNames[s], historyNames[t])),
                        1.0f);  // s, then t
          weight.insert(getFeatureId(createFeatureName("end", historyNames[t])),
                        1.0f);                  // t, then end
          model->addArc(new Arc(Head(finalState), Tails(s, model->addState(*symIter)), weight));
        }
      }
    }
    sortArcs(model);

    return model;
  }

  // shortcut


  }


  }

  Hypergraph::IHypergraph<Arc>* createHierarchicalModel() {
    using namespace Hypergraph;
    IMutableHypergraph<Arc>* model = new MutableHypergraph<Arc>();
    model->forceCanonicalLex();
    model->setVocabulary(pVoc_);





      if (str.length() > 2 && (str[0] == 'I' || str[0] == 'B') && str[1] == '-') {
        nonterminals.insert(sym(str.substr(2)));
      }
    }



    // (NP) <- (B-NP)
    // (NP) <- (B-NP) (NP0)
    // (NP0) <- (I-NP)
    // (NP0) <- (I-NP) (NP0)


      stateId[nt] = model->addState(nt);


      stateId[b] = model->addState(b);


      stateId[i] = model->addState(i);


      stateId[nt0] = model->addState(nt0);

      Weight weight1(0.0f); weight1.insert(getFeatureId(createFeatureName("short", nt)), 1.0f);
      model->addArc(new Arc(Head(stateId[nt]), Tails(stateId[b]), weight1)); // (NP) <- (B-NP)

      Weight weight2(0.0f); weight2.insert(getFeatureId(createFeatureName("long", nt)), 1.0f);
      model->addArc(new Arc(Head(stateId[nt]), Tails(stateId[b], stateId[nt0]), weight2)); // (NP) <- (B-NP) (NP0)

      Weight weight3(0.0f); weight3.insert(getFeatureId(createFeatureName("short_sub", nt)), 1.0f);
      model->addArc(new Arc(Head(stateId[nt0]), Tails(stateId[i]), weight3)); // (NP0) <- (I-NP)

      Weight weight4(0.0f); weight4.insert(getFeatureId(createFeatureName("long_sub", nt)), 1.0f);
      model->addArc(new Arc(Head(stateId[nt0]), Tails(stateId[i], stateId[nt0]), weight4));
    }


    nonterminals.insert(O);
    stateId[O] = model->addState(O);

    // add (nt1+nt2) states





        stateId[nt12] = model->addState(nt12);
      }
    }

    // Trigram model over nonterminals:
    // (NP+VP) <- (ADJ+NP) (VP)
    // ...








          Weight weight(0.0f);
          weight.insert(getFeatureId(createFeatureName("tri_nt", nt1Str, nt2Str, nt3Str)), 1.0f);
          weight.insert(getFeatureId(createFeatureName("bi_nt", nt2Str, nt3Str)), 1.0f);
          weight.insert(getFeatureId(createFeatureName("uni_nt", nt3Str)), 1.0f);
          model->addArc(new Arc(Head(stateId[nt23]),
                                Tails(stateId[nt12], stateId[nt3]), weight));
        }
      }
    }

    // (NP+VP) <- (eps+NP) (VP)




      stateId[epsNt1] = model->addState(epsNt1);




        Weight weight(0.0f);
        weight.insert(getFeatureId(createFeatureName("tri_nt", "eps", nt1Str, nt2Str)), 1.0f);
        weight.insert(getFeatureId(createFeatureName("bi_nt", nt1Str, nt2Str)), 1.0f);
        weight.insert(getFeatureId(createFeatureName("uni_nt", nt2Str)), 1.0f);
        model->addArc(new Arc(Head(stateId[nt12]),
                              Tails(stateId[epsNt1], stateId[nt2]), weight));
      }
    }

    // (eps+NP) <- START (NP)
    // (eps+VP) <- START (VP)
    // ...



      Weight weight(0.0f);
      weight.insert(getFeatureId(createFeatureName("bi_nt", "eps", ntStr)), 1.0f);
      weight.insert(getFeatureId(createFeatureName("uni_nt", ntStr)), 1.0f);
      model->addArc(new Arc(Head(stateId[epsNt]),
                            Tails(stateId[nt]), weight));
    }


    stateId[final0] = model->addState(final0);

    // (FINAL0) <- (NP+VP)





        Weight weight(0.0f);
        weight.insert(getFeatureId(createFeatureName("tri_nt", nt1Str, nt2Str, "FINAL")), 1.0f);
        weight.insert(getFeatureId(createFeatureName("bi_nt", nt2Str, "FINAL")), 1.0f);
        weight.insert(getFeatureId(createFeatureName("uni_nt", "FINAL")), 1.0f);
        model->addArc(new Arc(Head(stateId[final0]),
                              Tails(stateId[nt12]), weight));
      }
    }

    // (FINAL0) <- (eps+NP)



      Weight weight(0.0f);
      weight.insert(getFeatureId(createFeatureName("tri_nt", "eps", nt1Str, "FINAL")), 1.0f);
      weight.insert(getFeatureId(createFeatureName("bi_nt", nt1Str, "FINAL")), 1.0f);
      weight.insert(getFeatureId(createFeatureName("uni_nt", "FINAL")), 1.0f);
      model->addArc(new Arc(Head(stateId[final0]),
                            Tails(stateId[epsNt1]), weight));
    }



    // sortArcs(model); // doesn't work


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

  void addLabelArc(Hypergraph::IMutableHypergraph<Arc>* hg

                   , Hypergraph::StateId from, Hypergraph::StateId to) const {
    Weight weight(0.0f);
    weight.insert(getFeatureId(createFeatureName("word+label", word, label)), 1.0f);
    weight.insert(getFeatureId(createFeatureName("pos+label", pos, label)), 1.0f);
    weight.insert(getFeatureId(createFeatureName("label", label)), 1.0f);

    using namespace Hypergraph;
    hg->addArc(new Arc(Head(to),
                       Tails(from, hg->addState(label)),
                       weight));
  }

  enum HypergraphType {kClamped = 1, kUnclamped = 2};

  Hypergraph::IHypergraph<Arc>*
  createSearchSpace(Sentence const& sent
                    , Hypergraph::IHypergraph<Arc> const& transitionModel
                    , HypergraphType hgType) const {
    using namespace Hypergraph;
    IMutableHypergraph<Arc>* result = new MutableHypergraph<Arc>(kFsmOutProperties);
    result->setVocabulary(pVoc_);
    // Add all structural (non-lexical) states first so they have lower IDs:
    for (std::size_t i = 0; i <= sent.words.size(); ++i) {
      result->addState();
    }


    for (std::size_t i = 0; i < sent.words.size(); ++i) {
      if (hgType == kClamped) { // add the observed (gold) label (clamped, nominator)
        addLabelArc(result, sent.words[i], sent.poss[i], sent.labels[i], i, i + 1);
      }
      else {


        LabelsPerPosMap::const_iterator iter = labelsPerPos_.find(sent.poss[i]);
        if (iter != labelsPerPos_.end()) {
          labels = &(iter->second);
        }

          addLabelArc(result, sent.words[i], sent.poss[i], label, i, i + 1);
        }
      }
    }


    IMutableHypergraph<Arc>* composed =
        new MutableHypergraph<Arc>(kCanonicalLex | kStoreOutArcs | kStoreInArcs);

    if (opts_.fstCompose) {
      ComposeTransformOptions cOpt;
      cOpt.fstCompose = true;
      Hypergraph::fs::compose(
          *result, (IMutableHypergraph<Arc> const&)transitionModel, composed, cOpt);
    }
    else {
      Hypergraph::compose(transitionModel, *result, composed);
    }

    delete result;
    pruneUnreachable(composed);
    sortStates(*composed);

    return composed;
  }

  void processTrainingSentence(Sentence const& sent,
                               Hypergraph::IHypergraph<Arc> const& transitionModel,
                               Pairs& pairs) {
    Hypergraph::IHypergraph<Arc>* clamped =
        createSearchSpace(sent, transitionModel, kClamped);
    Hypergraph::IHypergraph<Arc>* unclamped =
        createSearchSpace(sent, transitionModel, kUnclamped);
    boost::lock_guard<boost::mutex> lock(mutex_);
    pairs.push_back(value_type(IHgPtr(clamped), IHgPtr(unclamped)));
  }

  ///

  /// Threads can call this functor
  struct ProcessTrainingExampleRange : boost::noncopyable {

    typedef CreateSearchSpace<A> Outer;

    ProcessTrainingExampleRange(Outer& o
                                , std::size_t begin
                                , std::size_t end
                                , std::vector<Sentence> const& sents
                                , Hypergraph::IHypergraph<A> const& transitionModel
                                , typename Outer::Pairs& trainingExamples
                                , bool doLogProgress
                                )
        : outer_(o), begin_(begin), end_(end), sents_(sents)
        , transitionModel_(transitionModel), trainingExamples_(trainingExamples)
        , doLogProgress_(doLogProgress)
    {}

    void operator()() {

                "Processing sentences " << begin_ << " to " << end_);
      for (std::size_t i = begin_; i < end_; ++i) {

                  "Processing sentence " << i);
        outer_.processTrainingSentence(sents_[i], transitionModel_, trainingExamples_);

        if (doLogProgress_) {
          std::size_t blockSize = (end_ - begin_) / 10.0f;
          if (blockSize && (i-begin_+1) % blockSize == 0) {

                     (((i-begin_+1.0f) / (end_ - begin_)) * 100.0f) << "% processed");
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

    if (!opts_.writeTrainArchivePath.empty()) {      // Write archive?
      pairs_.reset(
          new Optimization::WriteFeatureHypergraphPairs<Arc>(
              opts_.writeTrainArchivePath));
    }
    else if (!opts_.readTrainArchivePath.empty()) {  // Read archive?
      pairs_.reset(
          new Optimization::ExternalFeatHgPairs<Arc>(opts_.readTrainArchivePath));
      return;
    }
    else {                                            // Construct HGs
      pairs_.reset(new Optimization::InMemoryFeatureHypergraphPairs<Arc>());
    }


    std::vector<Sentence> sents;
    std::string line;
    Sentence sent;
    Util::Input input(opts_.conllPath);
    while (std::getline(*input, line)) {
      if (line.empty()) {
        sents.push_back(sent);
        sent.clear();
      }
      else {
        sent.add(line, &allLabels_, pVoc_, testMode_);
      }
    }
    assert(!allLabels_.empty());



    // For pruning
    forall (Sentence const& sent, sents) {
      for (std::size_t i = 0; i < sent.poss.size(); ++i) {
        labelsPerPos_[sent.poss[i]].insert(sent.labels[i]);
      }
    }

    Hypergraph::IHypergraph<Arc>* transitionModel =
        opts_.transitionModel == kUnigram ? createUnigramModel() :
        opts_.transitionModel == kBigram ? createBigramModel() :
        opts_.transitionModel == kHierarchical ? createHierarchicalModel() :
        createBihierarchicalModel();


    if (opts_.numThreads < 2) {
      std::size_t cnt = 0;
      forall (Sentence const& sent, sents) {
        processTrainingSentence(sent, *transitionModel, *pairs_);
      }
    }
    else { // multi-threaded:
      std::size_t numProducers = opts_.numThreads;
      boost::thread_group producer_threads;

      std::size_t size = sents.size();
      std::size_t blockSize = size / numProducers;
      std::size_t numProducerThreads =
          (numProducers * blockSize == size) ? numProducers : numProducers + 1;

      typedef Util::AutoDeleteAll<ProcessTrainingExampleRange> Producers;
      Producers producers;
      producers.reserve(numProducerThreads);
      ProcessTrainingExampleRange *update;
      for (std::size_t i = 0; i < numProducerThreads; ++i) {
        TrainingDataIndex begin = i * blockSize;
        TrainingDataIndex end = (i == numProducerThreads-1) ? size : (i+1) * blockSize;
        producers.push_back(
            update = new ProcessTrainingExampleRange(
                *this, begin, end,
                sents, *transitionModel, *pairs_, i==0));
        // boost::ref prevents copy:
        producer_threads.create_thread(boost::ref(*update));
      }
      producer_threads.join_all();
    }

    delete transitionModel;


    if (!testMode_) {
      writeLabelsFile(opts_.labelsPath);
      writeLabelsPerPosFile(opts_.labelsPerPosPath);
    }

    pairs_->setNumFeatures(getNumFeatures());
    pairs_->finish();


    if (!opts_.writeTrainArchivePath.empty()
        && !opts_.readTrainArchivePath.empty()) {  // Read after we've just written
      boost::this_thread::sleep(boost::posix_time::milliseconds(1000)); // just in case
      pairs_.reset(
          new Optimization::ExternalFeatHgPairs<Arc>(opts_.readTrainArchivePath));
    }
  }

  std::string getName() const {
    return "CrfDemo";
  }

 private:
  IVocabularyPtr pVoc_;
  shared_ptr< Optimization::IFeatureHypergraphPairs<Arc> > pairs_;
  CrfDemoConfig opts_;


  bool testMode_;
  boost::mutex mutex_;
};



#endif
