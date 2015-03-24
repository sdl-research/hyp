#define USAGE_HypEmpty "Print EMPTY if input hypergraph is empty (can't reach final state from start and lexical leaves); otherwise print NONEMPTY"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/Empty.hpp>
#include <sdl/Config/Init.hpp>

namespace sdl {
namespace Hypergraph {

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
  HypEmpty() : TransformMain<HypEmpty>("Empty", USAGE_HypEmpty)
  {
    opt.require_ins();
  }
  HypEmptyOptions empty;

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

HYPERGRAPH_NAMED_MAIN(Empty)
