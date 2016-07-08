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

    a more efficient std::priority_queue (min item at top()). doesn't keep
    track of positions in heap like d_ary_heap but you can still adjust the
    node at the top

*/

#ifndef PRIORITYQUEUE_JG20121222_HPP
#define PRIORITYQUEUE_JG20121222_HPP
#pragma once

#include <graehl/shared/d_ary_heap.hpp>
#include <graehl/shared/priority_queue.hpp>
#include <graehl/shared/property.hpp>
#include <vector>

namespace sdl {
namespace Util {

using graehl::d_ary_heap_indirect;
/**
   see d_ary_heap.hpp for more details.

   priority_queue public methods push(val), pop(), Val const& top()),
   adjust_top(), add/emplace_unsorted(x) / finish_adding()

   4-ary heaps of ints are the fastest. but this may vary depending on size/cost of Value assignment
   */
using graehl::priority_queue;
using graehl::identity_distance;
using graehl::FirstPmap;
using graehl::SecondPmap;


}}

#endif
