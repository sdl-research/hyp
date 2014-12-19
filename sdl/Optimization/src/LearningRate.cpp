#include <boost/make_shared.hpp>
#include <sdl/Optimization/LearningRate.hpp>

namespace sdl {
namespace Optimization {

shared_ptr<ILearningRate>
makeLearningRate(std::size_t numUpdates,
                 LearningRateOptions& opts) {
  if (opts.method == kConstant) {
    opts.constantRate.setNumUpdates(numUpdates);
    return make_shared<ConstantLearningRateFct>(opts.constantRate);
  }
  if (opts.method == kExponential) {
    opts.exponentialRate.setNumUpdates(numUpdates);
    return make_shared<ExponentialLearningRateFct>(opts.exponentialRate);
  }
  if (opts.method == kNocedal) {
    opts.nocedalRate.setNumUpdates(numUpdates);
    return make_shared<NocedalLearningRateFct>(opts.nocedalRate);
  }
  return shared_ptr<ILearningRate>(); // make compiler happy
}

}}
