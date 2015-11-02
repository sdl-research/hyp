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

    Transform types.

    TransformHolder: for Transform, hold arc-type-specific models (or other
    init. we don't want to happen for every single hg transformed).

    If Options == Transform class (no per-thread mutable state), you may
    optionally 'typedef void IsSimpleTransform;' (easiest to inherit from
    SimpleTransform<...> in this case.
*/


#ifndef TRANSFORMFWD_LW2012426_HPP
#define TRANSFORMFWD_LW2012426_HPP
#pragma once

#include <sdl/SharedPtr.hpp>

namespace sdl {
namespace Hypergraph {

namespace Transform {
static constexpr const bool Inplace = true;
static constexpr const bool Inout = false;
static constexpr const bool NoProperties = 0;
}


/**
   TransformHolder is used to cache process-wide state for options in order to
   transform a particular *arc* type (i.e. it holds a TransformFor<..., Arc>):

example:

 ReweightOptions reweight;
 TransformHolder t = transformFor<Arc>(reweight);

 Reweight<Arc>
 MutableHypergraph<Arc> hg1, hg2;
 inplace(hg1, useTransform<Arc, ReweightOptions>(t, opt, reweight));

  would be marked const so you don't accidentally put mutable state in what
  could be a thread-shared object, except that shared_ptr<void const> doesn't
  work.
 */
typedef void AnyTransform;
typedef shared_ptr<AnyTransform> TransformHolder;

/// template typedef trait TransformForHolder. you can also specialize this class

template <class TransformOptions, class Arc, class VoidIfSimpleTransform = void>
struct TransformFor {
  typedef typename TransformOptions::template TransformFor<Arc>::type type;
  enum { Simple = false };
  static type const* getSimple(TransformOptions const&) { return (type const*)0; }
  static TransformHolder getComplex(TransformOptions const& opt) { return TransformHolder(new type(opt)); }
};

template <class TransformOptions, class Arc>
struct TransformFor<TransformOptions, Arc, typename TransformOptions::IsSimpleTransform> {
  typedef TransformOptions type;
  enum { Simple = true };
  static type const* getSimple(TransformOptions const& opt) { return &opt; }
  static TransformHolder getComplex(TransformOptions const& opt) { return TransformHolder(); }
};


}}

#endif
