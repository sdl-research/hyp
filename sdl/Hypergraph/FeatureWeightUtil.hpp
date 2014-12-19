










namespace Hypergraph {

/**


*/




  static inline Result dotProduct(Weight const& w, WeightsMap const& weights) {
    return 0;
  }














}


template<class Weight, class Enable = void>
struct FeaturesVisitor {
  template <class Visitor>
  static inline void visit(Weight const&, Visitor&) {
    // no op in no-features weight type (e.g., ViterbiWeight)
  }
};
template<class Weight>
struct FeaturesVisitor<Weight, typename Weight::IsFeatureWeight> {
  template <class Visitor>
  static inline void visit(Weight const& weight, Visitor& visitor) {
    typedef typename Weight::value_type value_type;

      visitor(aPair.first, aPair.second);
    }
  }
};

/**



*/
template <class Weight, class Visitor>
void visitFeatures(Weight const& weight, Visitor& visitor) {
  return FeaturesVisitor<Weight>::visit(weight, visitor);
}


template<class Weight, class Enable = void>
struct GetNumFeatures {
  std::size_t operator()(Weight const&) const {
    return 0; // because not a feature weight
  }
};

// specialization for feature weight
template<class Weight>
struct GetNumFeatures<Weight, typename Weight::IsFeatureWeight> {
  std::size_t operator()(Weight const& weight) const {
    return weight.size();
  }
};

template <class Weight>
std::size_t getNumFeatures(Weight const& weight) {
  GetNumFeatures<Weight> getNum;
  return getNum(weight);
}


template<class Weight, class Enable = void>
struct RemoveFeatures {
  void operator()(Weight&) const {
    // not a feature weight: no-op
  }
};

// specialization for feature weight
template<class Weight>
struct RemoveFeatures<Weight, typename Weight::IsFeatureWeight> {
  void operator()(Weight& weight) const {
    weight.removeFeatures();
  }
};

template <class Weight>
void removeFeatures(Weight& weight) {
  RemoveFeatures<Weight> removeFeats;
  return removeFeats(weight);
}

////////////////////////////////////////////////////////////////////////////////













/**


*/










               "Hypergraph weight type does not support features.");

  }









  }
};
























































#endif
