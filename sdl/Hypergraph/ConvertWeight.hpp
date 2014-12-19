




namespace Hypergraph {

/**


 */
template<class FromWeight, class ToWeight>
ToWeight convertWeight(FromWeight const& fromWeight) {
  return ToWeight(fromWeight.getValue());
}

/**


 */
template<class FromW, class ToW>
struct WeightConverter {

  WeightConverter(FromW const& from, ToW& to) {
    to = ToW(from.getValue());
  }
};



#endif
