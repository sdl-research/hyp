














#include <utility>  // make_pair
#include <boost/unordered_map.hpp>


namespace Hypergraph {

/**

*/
template <class FromA, class ToA, class MapFct>


 public:
  typedef FromA FromArc;
  typedef ToA ToArc;
  typedef MapFct Mapper;
  typedef boost::unordered_map<FromArc*, ToArc*> ArcsMap;





  ~MapHypergraph() {
    // TODO: memory leak if memoization is turned off

      ToArc* newArc = pairOldNewArcs.second;
      delete newArc;
      // We're not deleting old arc (pairOldNewArcs.first) because it
      // was created outside of this class.
    }
  }








  }
















  }




  }









 private:

    ToArc* mappedArc = (ToArc*)NULL;





    return mappedArc;
  }




};

// Mappers

/**

*/
template <class FromArc, class ToArc>
struct ConvertArcTypeMapper {
  ToArc* operator()(FromArc const* arc) const {
    typedef typename FromArc::Weight FromWeight;
    typedef typename ToArc::Weight ToWeight;
    ToWeight toWeight;


  }
};

/**


*/
template <class FromArc, class ToArc>
struct SetWeightMapper {

  typedef typename ToArc::Weight ToWeight;



  ToArc* operator()(FromArc* arc) const {

    return result;
  }


};

/**



*/
template <class FromArc, class ToArc>
struct LengthMapper {

  typedef typename ToArc::Weight ToWeight;



  ToArc* operator()(FromArc* arc) const {

    const bool isTerminal = symid.isTerminal();
    const ToWeight weight = isTerminal ? ToWeight(1.0) : ToWeight(0.0);

    return result;
  }



};




#endif
