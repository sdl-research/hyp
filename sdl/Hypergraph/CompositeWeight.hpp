
















#include <ostream>



namespace Hypergraph {

/**

*/
class NoWeight {
 public:
  NoWeight() {}
  template <class W>
  NoWeight(W const& otherWeight) {}



  }



  }














};


  return out;
}


  return true;
}

  return false;
}

template <typename W>

  return false;
}


  return true;
}




}




}




}




















}


/**








*/

class CompositeWeight {
 public:


































  typedef W1 Weight1;
  typedef W2 Weight2;
  typedef W3 Weight3;
  typedef W4 Weight4;
  typedef W5 Weight5;



                           Weight5 const& w5 = Weight5())


  static CompositeWeight<Weight1, Weight2, Weight3, Weight4, Weight5> zero() {

        Weight1::zero(), Weight2::zero(), Weight3::zero(), Weight4::zero(), Weight5::zero());
  }

  static CompositeWeight<Weight1, Weight2, Weight3, Weight4, Weight5> one() {

        Weight1::one(), Weight2::one(), Weight3::one(), Weight4::one(), Weight5::one());
  }

















 private:





};


/**

*/
template <class W1>
class CompositeWeight<W1, NoWeight, NoWeight, NoWeight, NoWeight> {
 public:





  typedef W1 Weight1;
  typedef NoWeight Weight2;
  typedef NoWeight Weight3;
  typedef NoWeight Weight4;
  typedef NoWeight Weight5;

































  static CompositeWeight<Weight1, NoWeight, NoWeight, NoWeight, NoWeight> zero() {

  }

  static CompositeWeight<Weight1, NoWeight, NoWeight, NoWeight, NoWeight> one() {

  }



 private:

};

/**

*/
template <class W1, class W2>
class CompositeWeight<W1, W2, NoWeight, NoWeight, NoWeight> {
 public:




















  typedef W1 Weight1;
  typedef W2 Weight2;
  typedef NoWeight Weight3;
  typedef NoWeight Weight4;
  typedef NoWeight Weight5;





  static CompositeWeight<Weight1, Weight2, NoWeight, NoWeight, NoWeight> zero() {

  }

  static CompositeWeight<Weight1, Weight2, NoWeight, NoWeight, NoWeight> one() {

  }





















 private:


};

/**

*/
template <class W1, class W2, class W3>
class CompositeWeight<W1, W2, W3, NoWeight, NoWeight> {
 public:
























  typedef W1 Weight1;
  typedef W2 Weight2;
  typedef W3 Weight3;
  typedef NoWeight Weight4;
  typedef NoWeight Weight5;
























  static CompositeWeight<Weight1, Weight2, Weight3, NoWeight, NoWeight> zero() {


  }

  static CompositeWeight<Weight1, Weight2, Weight3, NoWeight, NoWeight> one() {


  }



 private:



};

/**

*/
template <class W1, class W2, class W3, class W4>
class CompositeWeight<W1, W2, W3, W4, NoWeight> {
 public:






























  typedef W1 Weight1;
  typedef W2 Weight2;
  typedef W3 Weight3;
  typedef W4 Weight4;
  typedef NoWeight Weight5;






















  static CompositeWeight<Weight1, Weight2, Weight3, Weight4, NoWeight> zero() {


  }

  static CompositeWeight<Weight1, Weight2, Weight3, Weight4, NoWeight> one() {


  }



 private:




};


template <class W1, class W2, class W3, class W4, class W5>



}

template <class W1, class W2, class W3, class W4, class W5>

  return !(x == y);
}












template <class W1, class W2, class W3, class W4, class W5>

                                          CompositeWeight<W1, W2, W3, W4, W5> const& y) {



}

template <class W1, class W2, class W3, class W4, class W5>

                                           CompositeWeight<W1, W2, W3, W4, W5> const& y) {





  return CompositeWeight<W1, W2, W3, W4, W5>(w1, w2, w3, w4, w5);
}

template <class W1, class W2, class W3, class W4, class W5>

                                          CompositeWeight<W1, W2, W3, W4, W5> const& y) {





  return CompositeWeight<W1, W2, W3, W4, W5>(w1, w2, w3, w4, w5);
}
















// Output operators: (Partial) template specializations.

//  (We could also specialize the times() etc. operations that way
// just for efficiency, but the compiler will prob. already optimize
// appropriately.)

template <class W1, class W2, class W3, class W4, class W5>
std::ostream& operator<<(std::ostream& out, CompositeWeight<W1, W2, W3, W4, W5> const& w) {


}

template <class W1, class W2, class W3, class W4>
std::ostream& operator<<(std::ostream& out, CompositeWeight<W1, W2, W3, W4, NoWeight> const& w) {

}

template <class W1, class W2, class W3>
std::ostream& operator<<(std::ostream& out, CompositeWeight<W1, W2, W3, NoWeight, NoWeight> const& w) {

}

template <class W1, class W2>
std::ostream& operator<<(std::ostream& out, CompositeWeight<W1, W2, NoWeight, NoWeight, NoWeight> const& w) {

}

template <class W1>



}





}




#endif
