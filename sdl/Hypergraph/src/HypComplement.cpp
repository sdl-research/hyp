#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <sdl/Hypergraph/Arc.hpp>
#include <sdl/Hypergraph/ArcParserFct.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/HypergraphDrawer.hpp>
#include <sdl/Hypergraph/HypergraphWriter.hpp>
#include <sdl/Hypergraph/fs/Complement.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Vocabulary/HelperFunctions.hpp>

#include <sdl/SharedPtr.hpp>
#include <sdl/Util/Forall.hpp>

#include <sdl/Util/ProgramOptions.hpp>

#define TRANSFORM HgComplement
#define USAGE "Create the complement (determinize first if necessary) of an unweighted fsa hypergraph -- input symbols only"
#define VERSION "v1"
#define HG_TRANSFORM_MAIN
#include <sdl/Hypergraph/TransformMain.hpp>

namespace sdl {
namespace Hypergraph {

struct TRANSFORM : TransformMain<TRANSFORM> { // note base class CRTP (google it)
  typedef TransformMain<TRANSFORM> Base;
  TRANSFORM() : Base(TRANSFORM_NAME(TRANSFORM), USAGE, VERSION) {}
  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc> *o) {
    fs::complement(i, o);
    return true;
  }
};

}}

INT_MAIN(sdl::Hypergraph::TRANSFORM)
