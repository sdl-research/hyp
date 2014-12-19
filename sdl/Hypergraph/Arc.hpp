













  (see also ArcWithData which has additional data attached)









#include <vector>
#include <iostream>
#include <cassert>














namespace Hypergraph {

struct WeightCount {
  std::size_t n, n1;
  WeightCount() : n(0), n1(0) {}




    ++n;

  }


    o << n << " hyperarcs, " << n1;

    o << " unweighted";
  }

  friend inline std::ostream& operator<<(std::ostream& o, WeightCount const& w) {
    w.print(o);
    return o;
  }
};

struct ArcPrinter {
  std::ostream& o;
  ArcPrinter(std::ostream& o = std::cout) : o(o) {}
  ArcPrinter(ArcPrinter const& o) : o(o.o) {}
  template <class Arc>
  void operator()(Arc* a) const {
    o << *a << '\n';
  }
  template <class Arc>
  void operator()(Arc const& a) const {
    o << a << '\n';
  }
};

struct Head {



};

struct Tails : public StateIdContainer {

  explicit Tails(StateId t1, StateId t2) {
    this->push_back(t1);
    this->push_back(t2);
  }
  explicit Tails(StateId t1, StateId t2, StateId t3) {
    this->push_back(t1);
    this->push_back(t2);
    this->push_back(t3);
  }
  explicit Tails(StateId t1, StateId t2, StateId t3, StateId t4) {
    this->push_back(t1);
    this->push_back(t2);
    this->push_back(t3);
    this->push_back(t4);
  }
  explicit Tails(StateId t1, StateId t2, StateId t3, StateId t4, StateId t5) {
    this->push_back(t1);
    this->push_back(t2);
    this->push_back(t3);
    this->push_back(t4);
    this->push_back(t5);
  }
  template <class Iter>
  explicit Tails(Iter begin, Iter end) {
    for (; begin != end; ++begin) {
      this->push_back(*begin);
    }
  }
};















































/**








template <class W>

 public:








  typedef W Weight;




  // TODO: consider using a fixed-size boost::array, e.g. for 2 tails
  // (binarized hypergraph)
  // better name: StateIds






























  /**
























  */





  /**



  */



  }




  virtual ~ArcTpl() {}










    assert(isFsmArc());

  }






    assert(isFsmArc());

  }





  TailIdRange tailIds() const {

  }




















    // heads equal: continue with weight

















  }

 private:



};

/**


*/
template <class W>
std::ostream& operator<<(std::ostream& out, const ArcTpl<W>& arc) {



  return out;
}








template <class Arc>

  assert(hg.isFsmArc(arc));

}























template <class Arc>

  assert(hg.isFsmArc(arc));

}

template <class Arc>

  assert(pHg->isFsmArc(arc));
  pHg->setInputLabel(arc.getTail(1), symid);
}

template <class Arc>

  assert(pHg->isFsmArc(arc));
  return pHg->setOutputLabel(arc.getTail(1), symid);
}


struct ArcWithDataDeleter {

  virtual ~ArcWithDataDeleter() {}
};

template <class T>
struct ArcWithDataDeleterTpl : public ArcWithDataDeleter {

};









/**

























*/
template <class W>
class ArcWithDataTpl : public ArcTpl<W> {

 public:
  typedef W Weight;
  typedef ArcTpl<W> Base;
  typedef ArcWithDataTpl<W> Arc;
  typedef boost::function<bool(Arc*)> ArcFilter;
  typedef boost::function<void(Arc*)> ArcVisitor;
























  virtual ~ArcWithDataTpl() {

  }


  void clear() {


  }










  }

  template <class T>



  }





  }












  }



 private:


};


/**


*/
template <class Arc, class StateIdTranslation>

  Arc* pResult = new Arc();



    const StateId mappedTail = tr.stateFor(otherArc.getTail(i));
    assert(mappedTail != kNoState);
    pResult->addTail(mappedTail);
  }
  return pResult;
}















#endif
