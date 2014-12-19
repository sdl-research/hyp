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

#define USAGE "turn FST into FSA by setting input/output labels of FST both to output(default) or input"
#define VERSION "v1"

#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/Project.hpp>

namespace sdl {
namespace Hypergraph {

struct HypProject : TransformMain<HypProject> {

  typedef TransformMain<HypProject> Base;

  Project project;

  HypProject()
      : Base("HypProject", USAGE, VERSION)
  {}

  void declare_configurable() {
    this->configurable(&project);
  }

  enum { has_inplace_transform1 = true };

  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc>& hg) {
    project.inplace(hg);
    return true;
  }

};

}}

INT_MAIN(sdl::Hypergraph::HypProject)

#undef HgProject
#undef USAGE
#undef VERSION
