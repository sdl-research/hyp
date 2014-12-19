#include <boost/make_shared.hpp>



namespace Optimization {


makeLearningRate(std::size_t numUpdates,
                 LearningRateOptions& opts) {
  if (opts.method == kConstant) {
    opts.constantRate.setNumUpdates(numUpdates);

  }
  if (opts.method == kExponential) {
    opts.exponentialRate.setNumUpdates(numUpdates);

  }
  if (opts.method == kNocedal) {
    opts.nocedalRate.setNumUpdates(numUpdates);

  }

}


