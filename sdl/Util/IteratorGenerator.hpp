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
#ifndef ITERATORGENERATOR_LW2012412_HPP
#define ITERATORGENERATOR_LW2012412_HPP
#pragma once

/* see Generator.hpp for Generator concept

   iterator pair (bounded range, generator) and single iterator (infinite generator)

   requires default constructed iterator pair are initialized so to compare == (empty range)

   also transformed iterator pair (bounded range, generator), since using
   GeneratorTransform would only give you a generator.
*/

#include <sdl/Util/Generator.hpp>
#include <iterator>
#include <utility>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/iterator/transform_iterator.hpp>

namespace sdl {
namespace Util {

template <class Iterator, class Result = typename std::iterator_traits<Iterator>::value_type>
struct IteratorGenerator
    : GeneratorBase<IteratorGenerator<Iterator, Result>, Result, PeekableT>
{
  //for boost::range
  typedef Iterator iterator;
  typedef iterator const_iterator;
  iterator begin() const { return iBegin; }
  iterator end() const { return iEnd; }

  iterator iBegin, iEnd;
  void clear() {
    iBegin = iEnd;
  }
  IteratorGenerator() : iBegin(), iEnd() {}
  template <class Range>
  IteratorGenerator(Range const& range) : iBegin(boost::begin(range)), iEnd(boost::end(range)) {}
  IteratorGenerator(IteratorGenerator const& o) : iBegin(o.iBegin), iEnd(o.iEnd) {}
  IteratorGenerator(iterator iBegin, iterator iEnd) : iBegin(iBegin), iEnd(iEnd) {}
  IteratorGenerator(std::pair<iterator, iterator> const& pairBeginEnd) : iBegin(pairBeginEnd.first), iEnd(pairBeginEnd.second) {}
  //TODO: test
  Result peek() const { return *iBegin; }
  //TODO: test
  void pop() { ++iBegin; }
  //TODO: test
  operator bool() const { return iBegin!=iEnd; }
};

template <class Trans, class Iterator, class Result = typename Trans::result_type>
struct TransformedIteratorGenerator
    : GeneratorBase<TransformedIteratorGenerator<Trans, Iterator, Result>, Result, PeekableT>
    , Trans
{
  typedef Iterator PreIterator;
  //for boost::range
  typedef boost::transform_iterator<Trans, PreIterator> iterator;
  typedef iterator const_iterator;
  iterator begin() const { return iterator(iBegin); }
  iterator end() const { return iterator(iEnd); }

  PreIterator iBegin, iEnd;

  template <class Range>
  TransformedIteratorGenerator(Range const& range, Trans const& t)
      : Trans(t), iBegin(boost::begin(range)), iEnd(boost::end(range)) {}
  TransformedIteratorGenerator() : iBegin(), iEnd() {}
  TransformedIteratorGenerator(iterator iBegin, iterator iEnd, Trans const& t = Trans())
      : Trans(t), iBegin(iBegin), iEnd(iEnd) {}
  explicit TransformedIteratorGenerator(std::pair<iterator, iterator> const& pairBeginEnd, Trans const& t = Trans())
      : Trans(t), iBegin(pairBeginEnd.first), iEnd(pairBeginEnd.second) {}
  Result operator()() { return Trans::operator()(*iBegin++); }
  Result peek() const { return Trans::operator()(*iBegin); }
  void pop() { ++iBegin; }
  operator bool() const { return iBegin!=iEnd; }
};

template <class Iterator, class Result = typename std::iterator_traits<Iterator>::value_type>
struct InfiniteIteratorGenerator
    : GeneratorBase<InfiniteIteratorGenerator<Iterator, Result>, Result, PeekableT> {
  typedef Iterator iterator;
  typedef iterator const_iterator;
  iterator iter;
  InfiniteIteratorGenerator() {}
  InfiniteIteratorGenerator(InfiniteIteratorGenerator const& o) : iter(o.iter) {}
  InfiniteIteratorGenerator(iterator iter) : iter(iter) {}
  Result peek() const { return *iter; }
  void pop() { ++iter; }
  operator bool() const { return true; }
};

template <class V>
InfiniteIteratorGenerator<V> makeInfiniteIteratorGenerator(V const& v) {
  return InfiniteIteratorGenerator<V>(v);
}


}}

#endif
