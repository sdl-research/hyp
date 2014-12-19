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

#define TRANSFORM HgGetString
#define USAGE "get the single string found by taking the first edge into each node starting from final (TOP)"
#define VERSION "v1"
#define HG_TRANSFORM_MAIN

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <sdl/Hypergraph/TransformMain.hpp>
#include <sdl/Hypergraph/Arc.hpp>
#include <sdl/Hypergraph/ArcParserFct.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/GetString.hpp>

#include <sdl/IVocabulary.hpp>
#include <sdl/Vocabulary/HelperFunctions.hpp>

#include <sdl/SharedPtr.hpp>
#include <sdl/Util/Forall.hpp>

#include <sdl/Util/ProgramOptions.hpp>

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
