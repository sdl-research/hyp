// Copyright 2014-2015 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#define USAGE_HypProject \
  "turn FST into FSA by setting input/output labels of FST both to output(default) or input"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/Project.hpp>

namespace sdl {
namespace Hypergraph {

struct HypProject : TransformMain<HypProject> {

  typedef TransformMain<HypProject> Base;

  Project project;

  HypProject() : Base("Project", USAGE_HypProject) {}

  void declare_configurable() { this->configurable(&project); }

  enum { has_inplace_transform1 = true };

  template <class Arc>
  bool transform1Inplace(IMutableHypergraph<Arc>& hg) {
    project.inplace(hg);
    return true;
  }
};


}}

HYPERGRAPH_NAMED_MAIN(Project)
