#include <string>
#include <sstream>
#include <fstream>

#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>









namespace Optimization {

namespace bfs = boost::filesystem;

template<class Arc>
ExternalFeatHgPairs<Arc>::ExternalFeatHgPairs(std::string const& location)
    : location_(location)
    , weights_(NULL)
{

           << location << "'");

  bfs::path pth1(bfs::path(location_) / bfs::path("size.txt"));
  std::ifstream in1(pth1.string().c_str());
  in1 >> size_;


  bfs::path pth2(bfs::path(location_) / bfs::path("num-feats.txt"));
  std::ifstream in2(pth2.c_str());
  in2 >> numParams_;

}

template<class Arc>
typename ExternalFeatHgPairs<Arc>::value_type
ExternalFeatHgPairs<Arc>::operator[](TrainingDataIndex i) {
  int numThousands = i / 1000;
  bfs::path pth(bfs::path(location_)
                / bfs::path("hg")



  // We don't actually make use of the symbols
  // TODO: remove or pass in
  shared_ptr<PerProcessVocabulary> voc(
      new PerProcessVocabulary(Vocabulary::createDefaultVocab()));

  std::ifstream in(pth.string().c_str());

  Hypergraph::IHypergraphsIteratorTpl<Arc>* iter =
      Hypergraph::IHypergraphsIteratorTpl<Arc>::create(
          in, Hypergraph::kDashesSeparatedHg, voc);
  iter->setHgProperties(Hypergraph::kStoreInArcs | Hypergraph::kStoreOutArcs);
  IHg* hg1 = iter->value();
  iter->next();
  assert(!iter->done());
  IHg* hg2 = iter->value();
  delete iter;

  typedef Hypergraph::IMutableHypergraph<Arc> MHg;
  detail::insertFeatureWeights(static_cast<MHg*>(hg1), weights_, numParams_);
  detail::insertFeatureWeights(static_cast<MHg*>(hg2), weights_, numParams_);

  return value_type(IHgPtr(hg1), IHgPtr(hg2));
}

template<class Arc>
void ExternalFeatHgPairs<Arc>::push_back(value_type const&) {

                "Cannot push back to ExternalFeatHgPairs -- it's read-only");
}

template<class Arc>
void ExternalFeatHgPairs<Arc>::setFeatureWeights(FloatT const* featWeights,
                                                 FeatureId numParams) {
  boost::lock_guard<boost::mutex> lock(mutex_);
  weights_ = featWeights;
  numParams_ = numParams;

}

template<class Arc>
void ExternalFeatHgPairs<Arc>::setFeatureWeights(TrainingDataIndex begin,
                                                 TrainingDataIndex end,
                                                 FloatT const* featWeights,
                                                 FeatureId numParams) {
  setFeatureWeights(featWeights, numParams);
}


