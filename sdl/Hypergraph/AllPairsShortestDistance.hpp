










#include <cmath>









namespace Hypergraph {


/**










*/














    }
  }
}










template<class Weight>




  for (StateId k = 0; k < n; ++k) {

    for (StateId i = 0; i < n; ++i) {




      for (StateId j = 0; j < n; ++j) {




      }
    }
  }
}

















  floydWarshallOverMatrix(distances);
}

template<class Arc>
void floydWarshall(IHypergraph<Arc> const& hg,
                   Util::Matrix<typename Arc::Weight>* distances) {

}





























































  enum { kZeroDropped = ZeroDroppedPaths };

  InputHypergraph const& hg;























    //    if (!hg.isFsm() || !hg.storesOutArcs())








 private:




















        if (viaWt==zeroWeight)
          continue;



          if (kZeroDropped)
            viaWt = zeroWeight;
          continue; // we already don't care for from->via. definitely don't continue it.











        }























#endif
