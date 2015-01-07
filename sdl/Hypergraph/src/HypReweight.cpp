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
#define TRANSFORM HgReweight
#define USAGE ReweightOptions::caption()
#define VERSION "v1"
#define HG_TRANSFORM_MAIN

#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/Reweight.hpp>

namespace sdl {
namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // CRTP
  typedef TransformMain<TRANSFORM> Base;
  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION)
  {
  }
  void declare_configurable() {
    this->configurable(&rw);
  }

  Reweight rw;
  static RandomSeed randomSeed() { return kRandomSeed; }
  enum { has_inplace_transform1 = true };
  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc> &h) {
    rw.inplace(h);
    return true;
  }
};

}}

INT_MAIN(sdl::Hypergraph::TRANSFORM)
