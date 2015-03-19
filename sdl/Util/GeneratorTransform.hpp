/** \file

    transformed generator. given generator g and transform f, yields f(x) for all x in g

    (like boost range transform)

*/

#ifndef LWUTIL__GENERATOR_TRANSFORM_HPP
#define LWUTIL__GENERATOR_TRANSFORM_HPP
#pragma once

#include <sdl/Util/Generator.hpp>

namespace sdl {
namespace Util {

template <class Gen, class Trans, class Result = typename Trans::result_type, class Tag = typename Gen::GeneratorTag>
struct TransformedGenerator
    : protected Gen, protected Trans
    , public GeneratorTraitsBase<Result, Tag>
{
  typedef Result result_type;
  typedef Tag GeneratorTag;
  typedef void NonPeekable;
  TransformedGenerator() {}
  TransformedGenerator(Gen const& gen, Trans const& t = Trans()) : Gen(gen), Trans(t) {}
  operator bool() const
  {
    return (bool)(Gen const&)*this;
  }
  bool operator !() const {
    return !(bool)(Gen const&)*this;
  }

  Result operator()() {
    return Trans::operator()(Gen::operator()());
  }
  Result get() {
    return Trans::operator()(Gen::operator()());
  }
  void got() {}
};

template <class Gen, class Trans, class Result>
struct TransformedGenerator<Gen, Trans, Result, PeekableT>
    : protected Gen, protected Trans
    , public GeneratorTraitsBase<Result, PeekableT>
{
  typedef Result result_type;
  typedef PeekableT GeneratorTag;
  typedef void Peekable;
  TransformedGenerator() {}
  TransformedGenerator(Gen const& gen, Trans const& t = Trans()) : Gen(gen), Trans(t) {}
  Result operator()() {
    return Trans::operator()(Gen::operator()());
  }
  operator bool() const
  {
    return (bool)(Gen const&)*this;
  }
  bool operator !() const {
    return !(bool)(Gen const&)*this;
  }
  void pop()
  {
    Gen::pop();
  }
  void got()
  {
    Gen::pop();
  }
  /// peek may be const or non-const depending on Gen
  Result peek() const
  {
    return Trans::operator()(Gen::peek());
  }
  Result peek()
  {
    return Trans::operator()(Gen::peek());
  }
  Result get() const
  {
    return Trans::operator()(Gen::peek());
  }
  Result get()
  {
    return Trans::operator()(Gen::peek());
  }

};


}}

#endif
