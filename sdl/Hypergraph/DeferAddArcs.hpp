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
/** \file

    add arcs later all at once (stop iterator invalidation while in-place adding to a mutable hg).
*/

#ifndef DEFERREDADDARCS_JG_2013_12_15_HPP
#define DEFERREDADDARCS_JG_2013_12_15_HPP
#pragma once

#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Util/Delete.hpp>

namespace sdl { namespace Hypergraph {

template <class Arc>
struct DeferAddArcs : Util::AutoDeleteAll<Arc> {
  typedef Util::AutoDeleteAll<Arc> Base;
  IMutableHypergraph<Arc> &hg;
  DeferAddArcs(IMutableHypergraph<Arc> &hg)
      : hg(hg)
  {
    this->reserve(hg.size() * 2);
  }
  void operator()(Arc *arc) const {
    const_cast<DeferAddArcs*>(this)->push_back(new Arc(*arc));
  }
  void finish() {
    typename Base::iterator i = this->begin(), e = this->end();
    try {
      for ( ; i!=e; ++i)
        hg.addArc((Arc*)*i);
    } catch (...) {
      for (; i!=e; ++i)
        delete (Arc*)*i;
      this->releaseAll();
      throw;
    }
    this->releaseAll();
  }
  ~DeferAddArcs() {
    finish();
  }
};


}}

#endif
