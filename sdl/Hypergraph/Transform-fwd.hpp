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

/** \file

    Transform types.

    TransformHolder: for Transform, hold arc-type-specific models (or other
    init. we don't want to happen for every single hg transformed).
*/


#ifndef TRANSFORMFWD_LW2012426_HPP
#define TRANSFORMFWD_LW2012426_HPP
#pragma once

#include <sdl/SharedPtr.hpp>

namespace sdl { namespace Hypergraph {

namespace Transform {
const bool Inplace=true;
const bool Inout=false;
const bool NoProperties=0;
}

/// template typedef trait TransformForHolder. you can also specialize this class
template <class TransformOptions, class Arc>
struct TransformFor {
  typedef typename TransformOptions::template TransformFor<Arc>::type type;
};

typedef shared_ptr<void> TransformHolder; // for state specific to a weight

/*
TransformHolder example: (include full Transform.hpp)

 ReweightOptions reweight;
 TransformHolder t = makeTransform<Arc>(reweight);

 Reweight<Arc>
 MutableHypergraph<Arc> hg1, hg2;
 inplace(hg1, useTransform(t, opt, reweight));
 */

}}

#endif
