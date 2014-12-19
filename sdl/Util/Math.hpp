




















namespace Util {

/**



 */
template<class U>
U unsignedSubtract(U a, U b) {
  return b < a ? a - b : 0;
}

template<class U>
U unsignedDiff(U a, U b) {
  return b < a ? a - b : b - a;
}



                       FloatT epsilon = FloatConstants<FloatT>::epsilon) {
  return d1 <= d2 + epsilon && d2 <= d1 + epsilon;
}







template<class FloatT, class FloatT2>

                              FloatT epsilon = FloatConstants<FloatT>::epsilon) {













}

/**

 */
template<class FloatT>
struct ApproxEqualFct {
  ApproxEqualFct(FloatT eps) : epsilon(eps) {}
  bool operator()(FloatT f1, FloatT f2) const {
    return floatEqual(f1, f2, epsilon);
  }
  FloatT epsilon;
};

/**


 */
template<class MapT>
struct ApproxEqualMapFct {
  typedef typename MapT::value_type ValueT;   // i.e., key/value pair
  typedef typename MapT::mapped_type MappedT; // i.e., value
  ApproxEqualMapFct(MappedT eps) : epsilon(eps) {}
  bool operator()(ValueT const& pair1,
                  ValueT const& pair2) const {
    return pair1.first == pair2.first
        && floatEqual(pair1.second, pair2.second, epsilon);
  }
  MappedT epsilon;
};


























template <typename T> int sgn(T val) {
  return (T(0) < val) - (val < T(0));
}



#endif
