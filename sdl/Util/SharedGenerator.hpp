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

    SharedGenerator<shared_ptr<Generator> > uses a pointer to generator to
    presents the usual generator interface. pointer may be bare or smart. null
    pointer means empty generator. copies are shallow; use clone() to get a deep
    copy.

    (TODO:C++11 - make copy call clone, else move ctor? or copy on write (check
    refcount))
*/

#ifndef PTRGENERATOR_JG20121228_HPP
#define PTRGENERATOR_JG20121228_HPP
#pragma once

#include <sdl/Util/RemovePointer.hpp>
#include <sdl/Util/Generator.hpp>

namespace sdl { namespace Util {

template <class PtrGen, class Tag = typename RemovePointer<PtrGen>::type::GeneratorTag>
struct SharedGenerator;

template <class PtrGen>
struct SharedGenerator<PtrGen, NonPeekableT>
    : GeneratorBase<SharedGenerator<PtrGen>
                    , typename RemovePointer<PtrGen>::type::result_type
                    , NonPeekableT>
{
  typedef typename RemovePointer<PtrGen>::type Generator;
  typedef PtrGen Ptr;
  Ptr p;
  Generator &generator() const { return *p; }
  SharedGenerator(Ptr const& p = PtrGen())
      : p(p) {}
  SharedGenerator(SharedGenerator const& other)
      : p(other.p) {}
  //  SharedGenerator(Generator *newGen) : p(newGen) {}
  typename Generator::result_type operator()() {
    return (*p)();
  }
  operator bool() const {
    return p&&(bool)*p;
  }
  SharedGenerator clone() const {
    return p ? SharedGenerator(new Generator(*p)) : SharedGenerator();
  }
  friend inline std::ostream& operator<<(std::ostream &out,
                                         SharedGenerator<PtrGen, NonPeekableT> const& self) {
    if (self.p)
      out << self.generator();
    else
      out << "(NULL)";
    return out;
  }
};

template <class PtrGen>
struct SharedGenerator<PtrGen, PeekableT>
    : GeneratorBase<SharedGenerator<PtrGen>
                    , typename RemovePointer<PtrGen>::type::result_type
                    , PeekableT>
{
  typedef typename RemovePointer<PtrGen>::type Generator;
  typedef PtrGen Ptr;
  Ptr p;
  Generator &generator() const { return *p; }
  SharedGenerator(Ptr const& p = Ptr())
      : p(p) {}
  SharedGenerator(SharedGenerator const& other)
      : p(other.p) {}
  //  SharedGenerator(Generator *newGen) : p(newGen) {}
  typename Generator::result_type peek() const {
    return p->peek();
  }
  typename Generator::value_type operator()() {
    typename Generator::value_type r((p->peek()));
    pop();
    return r;
  }
  void pop() {
    p->pop();
  }
  operator bool() const {
    return p&&(bool)*p;
  }
  SharedGenerator clone() const {
    return p ? SharedGenerator(new Generator(*p)) : SharedGenerator();
  }
  friend inline std::ostream& operator<<(std::ostream &out,
                                         SharedGenerator<PtrGen, PeekableT> const& self) {
    if (self.p)
      out << self.generator();
    else
      out << "(NULL)";
    return out;
  }
};


}}

#endif
