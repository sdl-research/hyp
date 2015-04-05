// Copyright 2014-2015 SDL plc
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

    a more efficient std::priority_queue (min item at top()). doesn't keep
    track of positions in heap like d_ary_heap but you can still adjust the
    node at the top
*/

#ifndef PRIORITYQUEUE_JG20121222_HPP
#define PRIORITYQUEUE_JG20121222_HPP
#pragma once

#include <vector>

#include <sdl/graehl/shared/d_ary_heap.hpp>
#include <sdl/graehl/shared/property.hpp>

namespace sdl { namespace Util {

using graehl::d_ary_heap_indirect;
using graehl::NullPropertyMap;
using graehl::IdentityMap;

/**
   priority_queue<Val> but faster. see d_ary_heap for more details.

   4-ary heaps of ints are the fastest. but this may vary depending on size/cost of Value assignment

   // NOTE: Equal is used only if you want to check queue.contains(value); defining
   // D_ARY_TRACK_OUT_OF_HEAP 1 means that it's not used even for that purpose,
   // though you then must init IndexInHeapPropertyMap for values not in queue to
   // -1 (and only use one priority queue for that value type)

   */

template <class Value, std::size_t Arity = 4,
          class DistanceMap = IdentityMap<Value>,
          class IndexInHeapPropertyMap = NullPropertyMap<std::size_t>, // don't track index
          class Better = std::less<typename DistanceMap::value_type>,
          class Container = std::vector<Value>,
          class Size = typename Container::size_type,
          class Equal = std::equal_to<Value> >
struct PriorityQueue
    : d_ary_heap_indirect<Value, Arity, DistanceMap, IndexInHeapPropertyMap, Better, Container, Size, Equal>
{
  typedef d_ary_heap_indirect<Value, Arity, DistanceMap, IndexInHeapPropertyMap, Better, Container, Size, Equal> Base;
  PriorityQueue(DistanceMap const& distance = DistanceMap(),
                IndexInHeapPropertyMap const& indexInHeap = IndexInHeapPropertyMap(),
                Better const& better = Better(),
                Size containerReserve = 10,
                Equal const& equal = Equal())
      : Base(distance, indexInHeap, better, containerReserve, equal)
  {}
  /**
     push(Value const&);

     pop();

     Value const& top();
  */

  /**
     if you modify top() and might need to move element down since it's no longer the least.
  */
  void adjustTop() {
    this->preserve_heap_property_down();
  }

};


}}

#endif
