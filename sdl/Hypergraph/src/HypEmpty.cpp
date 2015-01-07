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
#define HG_MAIN
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/Empty.hpp>
#include <sdl/Config/Init.hpp>

namespace sdl {
namespace Hypergraph {

#define USAGE "Print EMPTY if input hypergraph is empty (can't reach final state from start and lexical leaves); otherwise print NONEMPTY"
#define VERSION "v1"

struct HypEmptyOptions {
  bool empty;
  bool fsm;
  bool graph;
  HypEmptyOptions() { Config::inits(this); }
  template <class Config>
  void configure(Config &config) {
    config.is("HypEmpty");
    config("print lines of output indicating structural properties of input hypergraphs");
    config("empty", &empty).init(true)
        ("output a line EMPTY or NONEMPTY");
    config("fsm", &fsm).init(false)
        ("output a line FSM or NONFSM");
    config("graph", &graph).init(false)
        ("output a line GRAPH or NONGRAPH");
  }

};


struct HypEmpty : TransformMain<HypEmpty> { // note base class CRTP (google it)
  typedef TransformMain<HypEmpty> Base;
  HypEmptyOptions empty;
  HypEmpty() : Base("HypEmpty", USAGE, VERSION)
  {
    opt.require_ins();
  }

  void declare_configurable() {
    this->configurable(&empty);
  }

  static LineInputs lineInputs() { return kNoLineInputs; }

  Properties properties(int i) const {
    return default_properties ? (Properties)default_properties : kStoreOutArcs;
    // bottom-up reachability for CFG requires kStoreOutArcs but for graph only
    // requires kStoreFirstTailOutArcs. allows command line -p 0x10 for first
    // tail only
  }
  enum { has_transform1 = false, has_transform2 = false, has_inplace_input_transform = true };
  bool printFinal() const { return false; }
  void outProperty(char const* name, bool value) {
    if (!value)
      out() << "NON";
    out() << name << "\n";
  }

  template <class Arc>
  bool inputTransformInPlace(IHypergraph<Arc> const& hg, int) {
    if (empty.empty)
      outProperty("EMPTY", Hypergraph::empty(hg));
    if (empty.fsm)
      outProperty("FSM", hg.isFsm());
    if (empty.graph)
      outProperty("GRAPH", hg.isGraph());
    return true;
  }
};

}}

INT_MAIN(sdl::Hypergraph::HypEmpty)
