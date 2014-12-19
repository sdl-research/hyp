#include <iostream>
#include <vector>
#include <istream>

















namespace Hypergraph {

template <class Arc>
class FlatStringHypergraphsIterator : public IHypergraphsIteratorTpl<Arc> {







    opts.inputFeatures = feats;
  }

  ~FlatStringHypergraphsIterator() {}




  virtual void next() {

















    }
  }

  virtual IHypergraph<Arc>* value() {


  }





 private:





};






template <class Arc>
class FormattedHypergraphsIterator : public IHypergraphsIteratorTpl<Arc> {

 public:



  ~FormattedHypergraphsIterator() {}

  virtual void next() {





      return;
    }








  }

  virtual IHypergraph<Arc>* value() {


  }





 private:





};










template <class Arc>




    return new FlatStringHypergraphsIterator<Arc>(in, pVoc, feats);

    return new FormattedHypergraphsIterator<Arc>(in, pVoc);  // features are already determined


};



