// Copyright 2014 SDL plc
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
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
