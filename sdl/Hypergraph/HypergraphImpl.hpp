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

    helpers for IHypergraph.
*/


#ifndef HYP__HYPERGRAPHIMPL_HPP
#define HYP__HYPERGRAPHIMPL_HPP
#pragma once

#include <sdl/Hypergraph/Arc.hpp>
#include <sdl/Hypergraph/WeightUtil.hpp>

namespace sdl {
namespace Hypergraph {
namespace impl {

template <class Arc>
struct ArcDeleter {
  void operator()(ArcBase *x) const {
    delete (Arc*)x;
  }
};

template <class Arc>
struct ArcWeightSetOne {
  void operator()(ArcBase *x) const {
    setOne(static_cast<Arc*>(x)->weight_);
  }
};


}}}

#endif
