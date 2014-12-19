










#include <set>
#include <queue>











namespace Hypergraph {

struct IStatesVisitor {
  virtual void visit(StateId sid) = 0;

};








struct PrintStatesVisitor : public IStatesVisitor {
  PrintStatesVisitor(std::ostream& out = std::cout,
                     std::string separator = "\n")


  void visit(StateId sid) {

  }



};


/**




*/
template<class Arc>









 public:







      // MD: TODO: If incoming arcs are not stored we can still do it








                    "TopsortStatesTraversal needs incoming arcs");
    }

  }









  }








      }

    }
  }















































};


/**









   */
template<class Arc>




 public:



  }








    }






  }
};







































#endif
