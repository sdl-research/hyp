#define TRANSFORM HypGetString
#define USAGE "get the single string found by taking the first edge into each node starting from final (TOP)"
#define VERSION "v1"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/GetString.hpp>

namespace po = boost::program_options;

namespace sdl {
namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // note base class CRTP (google it)
  typedef TransformMain<TRANSFORM> Base;
  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION)
  {}
  bool printFinal() const { return false; }
  Properties properties(int) const { return kStoreInArcs; }
  enum { has_transform1 = false, has_inplace_input_transform = 1 };
  template <class Arc>
  bool inputTransformInPlace(IMutableHypergraph<Arc> &hg, int lineno) const {
    std::string str = sdl::Hypergraph::getString(hg);
    out() << str << '\n';
    return true;
  }
};

}}

INT_MAIN(sdl::Hypergraph::TRANSFORM)
