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

#include <cmath>
#include <sdl/Hypergraph/FeatureIdRange.hpp>
#include <sdl/Util/LogHelper.hpp>


namespace sdl {
namespace Hypergraph {

void FeatureIdRange::requireSize(double needed, char const* prefix) {
  double n = (double)size();
  double epsilon = 0.1; // prevent integer rounding difficulty
  if (n < needed-epsilon)
    SDL_THROW_LOG(Context, ConfigException,
                  prefix << " " << needed << " ids; you allowed only " << n << " in " << *this);
  end = begin + (FeatureId)(needed + epsilon);
}

std::ostream& operator<<(std::ostream &out,
                         FeatureIdRange const& x) {
  if (!x.enabled())
    out << "none";
  else {
    out << x.begin;
    if (x.end != (FeatureId) - 1)
      out << '-' << (x.end - 1);
  }
  return out;
}

double FeatureIdRange::tupleRequiresDouble(unsigned order) {
  double n = (double)size();
  double sum = 0;
  for (unsigned len = 2; len<=order; ++len)
    sum += std::pow(n, (double)len);
  return sum;
}

FeatureId FeatureIdRange::tupleRequires(unsigned order) {
  double maxFid = (double)(FeatureId)-1;
  double r = tupleRequiresDouble(order);
  if (r >= maxFid)
    SDL_THROW_LOG(Context, ConfigException,
                  "tuple order " << order << " for context "<<*this << " would result in too many features (" << r<<")");
  return (FeatureId)std::ceil(r); // if r fits, it's integral. still, ceil is safer
}


}}
