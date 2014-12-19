/**






 */





#include <cmath>





namespace Optimization {



/**

 */
struct ILearningRate {
  virtual ~ILearningRate() {}



};



class ConstantLearningRateFct : public ILearningRate {
 public:

  : eta(eta_) {}

  template <class Config>
  void configure(Config& config) {
    config("Options for a constant learning rate");
    config.is("ConstantLearningRateFct");


  }

  // Required, but no-op here
  void setNumUpdates(std::size_t) {}




};

class ExponentialLearningRateFct : public ILearningRate {
 public:


      : initialEta(initialEta_)
      , alpha(alpha_)
      , numUpdates(100) // make compiler happy by initializing
  {}

  template <class Config>
  void configure(Config& config) {
    config("Options for an exponential learning rate");
    config.is("ExponentialLearningRateFct");
    config("eta", &initialEta)("Rate on first call").init(initialEta);
    config("alpha", &alpha)("Decay").init(alpha);
  }

  // Required
  void setNumUpdates(std::size_t num) {

  }



  }




};



/**

 */
class NocedalLearningRateFct : public ILearningRate {
 public:

  /**



   */

      : stepSize(stepSize_)


  {}

  template <class Config>
  void configure(Config& config) {
    config("Options for the Nocedal learning rate");
    config.is("NocedalLearningRateFct");
    config("step-size", &stepSize)("Step size").init(stepSize);
  }

  // Required fct
  void setNumUpdates(std::size_t numUpdates) {
    // Nocedal recommends length to be 10% of the expected calls:

  }


    // Nick Andrews: "alpha must be between 0.5 and 1; alpha=0.602 is
    // special, and you shouldn't need to change this"

  }




};





struct LearningRateOptions {






  template <class Config>
  void configure(Config& config) {
    config("Options for the learning rate");
    config.is("LearningRate");
    config("method", &method)("Name of the method").init(kExponential);
    config("constant-rate", &constantRate)("Options if you pick method: 'constant'");
    config("exponential-rate", &exponentialRate)("Options if you pick method: 'exponential'");
    config("nocedal-rate", &nocedalRate)("Options if you pick method: 'nocedal'");




  }

  LearningRateType method;
  ConstantLearningRateFct constantRate;
  ExponentialLearningRateFct exponentialRate;
  NocedalLearningRateFct nocedalRate;
  float adagradRate, adagradL1Strength;
};

/**

 */

makeLearningRate(std::size_t numUpdates,
                 LearningRateOptions& opts);




