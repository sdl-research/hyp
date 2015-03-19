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
