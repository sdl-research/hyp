












namespace Hypergraph {



/**


*/
struct IFeaturesPerInputPosition {
  virtual ~IFeaturesPerInputPosition() {}

  /**



  */

};

/**

*/
struct NoFeatures : public IFeaturesPerInputPosition {


  /**

  */


  }



};

/**


*/
struct TakeFeaturesFromVector : public IFeaturesPerInputPosition {








  }



};

/**


*/
struct MultipleFeaturesPerInputPosition : public IFeaturesPerInputPosition {





  }



};




#endif
