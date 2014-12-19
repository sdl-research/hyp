/**










 */





#include <utility>
#include <fstream>
#include <vector>
#include <boost/filesystem.hpp>











namespace Optimization {

namespace bfs = boost::filesystem;

namespace detail {

/**


 */
template <class Arc>
void insertFeatureWeights(Hypergraph::IMutableHypergraph<Arc>* pHg,

  if (pWeights) {

    pHg->visitArcs(inserter);
  }
}


/**


 */
template <class ArcT>
class IFeatureHypergraphPairs {
 public:
  typedef Hypergraph::IHypergraph<ArcT> IHg;
  typedef shared_ptr<IHg> IHgPtr;
  typedef ArcT Arc;
  typedef std::pair<IHgPtr, IHgPtr> value_type;
  typedef typename ArcT::Weight Weight;
  typedef typename Weight::FloatT FloatT;

  virtual ~IFeatureHypergraphPairs() {}

  virtual value_type operator[](TrainingDataIndex) = 0;

  virtual void push_back(value_type const&) = 0;

  virtual void finish() {}

  /**

   */


  /**


   */



  /**

   */


  virtual FeatureId getNumFeatures() = 0;
  virtual void setNumFeatures(FeatureId) = 0;
};

/**


 */
template <class ArcT>
class InMemoryFeatureHypergraphPairs : public IFeatureHypergraphPairs<ArcT> {
 public:
  typedef ArcT Arc;
  typedef typename IFeatureHypergraphPairs<Arc>::value_type value_type;
  typedef typename IFeatureHypergraphPairs<Arc>::FloatT FloatT;
  typedef std::vector<value_type> Vector;
  typedef shared_ptr<Vector> VectorPtr;



  /**



   */


  value_type operator[](TrainingDataIndex index)OVERRIDE {


  }

  /**

     *


   */


    setFeatureWeights(0, size(), featWeights, numParams);
  }

  /**



   */








      typedef Hypergraph::IMutableHypergraph<Arc> MHg;


    }
  }








 private:
  VectorPtr pPairs_;
  FeatureId numParams_;
};

/**
 * @brief This just writes each HG pair to disk immediately, nothing
 * else.
 */
template <class ArcT>
class WriteFeatureHypergraphPairs : public IFeatureHypergraphPairs<ArcT> {
 public:
  typedef ArcT Arc;
  typedef typename IFeatureHypergraphPairs<Arc>::value_type value_type;
  typedef typename IFeatureHypergraphPairs<Arc>::FloatT FloatT;

  enum { kSleepForMicroSeconds = 500 };


    bfs::create_directories(fname + "/hg");
    Util::microSleep(kSleepForMicroSeconds);  // to be safe on NFS

  }


    int numThousands = size_ / 1000;

    bfs::create_directories(dir);

    std::ofstream out((dir / bfs::path(name)).string().c_str());
    if (project_) {  // don't need the input labels (could even remove all labels)
      typedef Hypergraph::IMutableHypergraph<Arc> MHg;
      assert(val.first->isMutable());
      assert(val.second->isMutable());
      static_cast<MHg*>(val.first.get())->projectOutput();
      static_cast<MHg*>(val.second.get())->projectOutput();
    }

    ++size_;
  }




    bfs::path p(dir_ / bfs::path("size.txt"));
    std::ofstream out(p.string().c_str());
    out << size_;
  }










    bfs::path p(dir_ / bfs::path("num-feats.txt"));
    std::ofstream out(p.string().c_str());
    out << numParams;
  }

 private:
  boost::filesystem::path dir_;
  std::size_t size_;
  bool project_;
};




#endif
