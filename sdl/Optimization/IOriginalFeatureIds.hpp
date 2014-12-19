






namespace Optimization {

struct IOriginalFeatureIds {
  virtual ~IOriginalFeatureIds() {};
  /**



   */

};

struct IdentityOriginalFeatureIds : IOriginalFeatureIds {
  FeatureId getOriginalFeatureId(FeatureId id) {
    return id;
  }
};




#endif
