




















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























































template <typename T> int sgn(T val) {
  return (T(0) < val) - (val < T(0));
}



#endif
