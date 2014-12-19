




*/





#include <boost/unordered_map.hpp>
#include <boost/noncopyable.hpp>







namespace Hypergraph {

/**

*/
template<class Visitor1, class Visitor2>
struct CompositeVisitor : boost::noncopyable {

  CompositeVisitor(Visitor1* visitor1,
                   Visitor2* visitor2)



  ~CompositeVisitor() {


  }

  template<class Arc>
  void operator()(Arc* pArc) const {


  }



};



template<class Arc>
struct SetWeightValueVisitor : boost::noncopyable {

  typedef typename Arc::Weight::FloatT FloatT;



  void operator()(Arc* pArc) const {

  }


};



//
// You can exclude a range of original feature IDs from this process
// -- they won't appear on the mapped arcs.
template<class Arc>
struct MapFeatureIdsVisitor : boost::noncopyable {
  typedef typename Arc::Weight Weight;

  typedef typename Weight::Map Map;
  typedef typename Weight::FloatT FloatT;

  typedef typename Map::value_type value_type;
  typedef typename Map::key_type key_type;
  typedef typename Map::mapped_type mapped_type;

  MapFeatureIdsVisitor(IdsMap* idsMap, IdsMap* idsMap_new2old)
      : idsMap_(idsMap)
      , idsMap_new2old_(idsMap_new2old)
  {}

  void setExcludeRange(FeatureIdRange excludeRange) {
    excludeRange_ = excludeRange;
  }

  void operator()(Arc* pArc) const {

    // Get original features map of this arc (uses orig, non-contiguous IDs):


    // New features map for the arc (uses new, contiguous IDs):



      if (excludeRange_ && (*excludeRange_).contains(aPair.first)) {
        continue;
      }
      key_type oldId = aPair.first;
      assert(idsMap_);
      assert(idsMap_new2old_);
      typename IdsMap::const_iterator iter = idsMap_->find(oldId);
      key_type newId;
      if (iter == idsMap_->end()) {
        newId = (key_type)idsMap_->size();


                        "Too many feature IDs: Maximum ID of " << newId << " reached");
        idsMap_->insert(std::make_pair(oldId, newId));
        idsMap_new2old_->insert(std::make_pair(newId, oldId));
      }
      else {
        newId = iter->second;
      }
      pNewMap->insert(value_type(newId, aPair.second));
    }


  }

  IdsMap* idsMap_;         /// Maps old feature IDs to new, contiguous IDs.
  IdsMap* idsMap_new2old_; /// Maps new, contiguous IDs back to old IDs.
  boost::optional<FeatureIdRange> excludeRange_;
};

/**

*/
template<class Arc>
struct InsertWeightsVisitor {

  typedef typename Arc::Weight Weight;
  typedef typename Weight::Map FeatMap;
  typedef typename Weight::FloatT FloatT;







  ~InsertWeightsVisitor() {}

  void operator()(Arc* arc) const {
    FloatT weightVal = 0.0;


    for (typename FeatMap::const_iterator it = map.begin(); it != map.end();
         ++it) {


    }

  }



};

/**



*/
template<class Arc>
struct InsertSparseWeightsVisitor {

  typedef typename Arc::Weight::Map Map;
  typedef typename Arc::Weight::FloatT FloatT;

  InsertSparseWeightsVisitor(Map const& featureWeights)

  {}

  void operator()(Arc* arc) const {
    FloatT weightVal(static_cast<FloatT>(0.0));

    for (typename Map::const_iterator it = map.begin(); it != map.end();
         ++it) {


        weightVal += weightsIter->second * it->second;
      }
    }
    arc->weight().setValue(weightVal);
  }


};



#endif
