









#include <ctime>
#include <cstdlib>
#include <queue>











namespace Hypergraph {

/**


*/
























    if (numArcs == 0) {
      return NULL;
    }
    double r = rand() / (RAND_MAX + 1.0);
    ArcId aid = static_cast<ArcId>(r * numArcs);

  }

};












    if (numArcs == 0) {
      return NULL;
    }

































// TODO: Add other sample functors: (1) Local: Sample in proportion to
// the weights of the incoming arcs, (2) Global: Sample in proportion
// to the forward (i.e., inside) weights

/**

*/

void samplePath(IHypergraph<Arc> const& hg,

                IMutableHypergraph<Arc>* result) {


  }
  result->setVocabulary(hg.getVocabulary());

  std::queue<StateId> queue;
  std::set<StateId> onQueue;

  queue.push(finalStateId);
  onQueue.insert(finalStateId);




  while (!queue.empty()) {
    StateId hgStateId = queue.front();
    queue.pop();


    }

    Arc* arc = sampler.sampleArc(hg, hgStateId);
    if (arc != NULL) {









        queue.push(sid);
        onQueue.insert(sid);
      }

    }
  }


}




#endif
