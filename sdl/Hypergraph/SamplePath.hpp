









#include <ctime>
#include <cstdlib>
#include <queue>











namespace Hypergraph {

/**


*/

template<typename Arc>
class Sampler {
 public:

  Sampler(unsigned long seed = time(0)) {srand(seed); }

  virtual ~Sampler() {}

  virtual void initialize(IHypergraph<Arc> const& hg) {}

  virtual Arc* sampleArc(IHypergraph<Arc> const& hg, StateId sid) const = 0;


};

template<typename Arc>
class UniformInArcSampler: public Sampler<Arc> {
 public:
  typedef typename Arc::Weight Weight;
  UniformInArcSampler(unsigned long seed = time(0)):Sampler<Arc>(seed) {}

  virtual Arc* sampleArc(IHypergraph<Arc> const& hg, StateId sid) const {

    if (numArcs == 0) {
      return NULL;
    }
    double r = rand() / (RAND_MAX + 1.0);
    ArcId aid = static_cast<ArcId>(r * numArcs);

  }

};

template<typename Arc>
class ProbabilityArcSampler: public Sampler<Arc> {
 public:
  typedef typename Arc::Weight Weight;
  ProbabilityArcSampler(unsigned long seed = time(0)):Sampler<Arc>(seed) {}

  virtual void initialize(IHypergraph<Arc> const& hg) {

  }
  virtual Arc* sampleArc(IHypergraph<Arc> const& hg, StateId sid) const {

    if (numArcs == 0) {
      return NULL;
    }
    if (numArcs == 1) {

    }


    std::vector< Weight> weights;
    std::vector< Arc* > arcs;




        if ( !hg.hasLexicalLabel(tailId)) {

        }
      }

      arcs.push_back(arc);
      weights.push_back(w);
    }

    double random = rand()%RAND_MAX/static_cast<double>(RAND_MAX) * exp(-totalWeight.getValue());
    int i = 0;
    Weight weightSum = weights[0];
    while (exp(- weightSum.getValue() ) < random) {

    }
    return arcs[i];
  }
 private:

};


// TODO: Add other sample functors: (1) Local: Sample in proportion to
// the weights of the incoming arcs, (2) Global: Sample in proportion
// to the forward (i.e., inside) weights

/**

*/
template<class Arc>
void samplePath(IHypergraph<Arc> const& hg,
                Sampler<Arc>& sampler,
                IMutableHypergraph<Arc>* result) {


  }
  result->setVocabulary(hg.getVocabulary());
  sampler.initialize(hg);
  std::queue<StateId> queue;
  std::set<StateId> onQueue;

  queue.push(finalStateId);
  onQueue.insert(finalStateId);
  result->addStateId(finalStateId,



  while (!queue.empty()) {
    StateId hgStateId = queue.front();
    queue.pop();


    }

    Arc* arc = sampler.sampleArc(hg, hgStateId);
    if (arc != NULL) {


        if (onQueue.find(sid) != onQueue.end()) {
          continue;
        }

        result->addStateId(sid,


        queue.push(sid);
        onQueue.insert(sid);
      }
      result->addArc(new Arc(*arc));
    }
  }


}




#endif
