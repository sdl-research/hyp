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

    Generator concept.

   // here's a peekable generator that doesn't have any header dependency.

   template <class Arc>
   struct PeekableArcGeneratorBase {
   typedef void Peekable; // or else typedef void NonPeekable, in which case you don't define peek and pop.
   typedef Arc *value_type;
   typedef Arc *reference;
   typedef Arc *result_type;


   Arc ** i;Arc ** end;

   Arc *peek() const { return *i; }
   Arc *get() const { return *i; }
   operator bool() const { return i != end; }
   void pop() { ++i; }
   void got() { ++i; }
   Arc *operator()() {
    Arc *r = get();
    got();
    return r;
   }

   };


   EXAMPLE (using a non-peekable generator):

   template <class LMimpl>
   void printVocab(LMimpl &lm)
   {
   typename LMimpl::StringIdPairGenerator stringIds=lm.getStringIdPairGenerator();
   while(stringIds)
   o << stringIds().first<<'\n';
   }

   EXAMPLE (using a peekable generator):

   template <class LMimpl>
   void printVocab(LMimpl &lm)
   {
   typename LMimpl::StringIdPairGenerator stringIds=lm.getStringIdPairGenerator();
   while(stringIds) {
   o << stringIds.peek().first<<'\n';
   stringIds.pop();
   }
   }


   Generator concept (similar to pair of input iterators, but less implementation burden and simpler interface):

   Generator g(...); // generators are modified by using them (are lvalues, not iterators/handles)
   typedef Generator::result_type V; // this should be a reference type if the underlying implementation can return it. use Generator::value_type if you want to hold the result
   while(g) {
   V r=g(); // gets (copy of) current value and advances
   visit(r);
   visit(r);
   }

   PeekableGenerator
   while(g) {
   visit(g.peek()); // get current value without advancing
   visit(g.peek()); // same value as before
   g.pop(); // advance: previous reference returned by g.peek() may not be valid
   }

   either generator, maximum possible efficiency (if your compiler optimizes ok):

   while(g) {
    visit(g.get());
    g.got(); // necessary for peekable generator - to avoid having to make a copy (postincrement problem)
   }
   defining a concrete generator:

   struct MyGenerator : GeneratorBase<MyGenerator, int, PeekableT> // inheriting adds typedefs, and gives default Generator operator() from peek and pop
   {
   int state;
   MyGenerator(int state=1) : state(state) {}
   int peek() const { return state; }
   void pop() { --state; }
   operator bool() const {return state;}
   };

   note that a PeekableGenerator is also a Generator.

   // for functions selecting between peekable and not:

   template <class Gen>
   bool peekable(typename boost::enable_if<is_void<typename Gen::Peekable>,Gen>::type &gen) { return true; }

   also:

   std::vector<V> vec;
   int n=appendGenerate(g, vec,10); // like v.push_back(g()) up to 10 times, returning number of items appended.
   int n=appendGenerate(g, vec); // unlimited
   boost::function<bool (V const&)> f;
   int n = visitGenerate(g, f,10); // same as: vec.clear();append(vec, g,10);forall (V const& x, vec) if (!f(x)) break;
   //NOTE: if false is returned by visitor, maybe the rest of the items were generated, maybe not.

   default implementaitons of append and visit are provided.

*/
/*
  also: AnyGenerator<Value, Tag>
  with Tag=NonPeekableT is a type erased Generator
  with Tag=PeekableT is a type erased PeekableGenerator, and can be used as an input iterator -


  a peekable AnyGenerator is also an input iterator that compares not equal to everything unless it and
  note: iterators will only compare equal when they're done. i.e. for PeekableT g you could use the boost::range (g, g)


*/

#ifndef SDL_UTIL__GENERATOR_HPP
#define SDL_UTIL__GENERATOR_HPP
#pragma once


#include <sdl/SharedPtr.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/add_reference.hpp>
#include <boost/mpl/bool.hpp>
#include <sdl/Util/RefCount.hpp>
#include <sdl/Util/Override.hpp>
#include <sdl/Util/AnyVisitor.hpp>
#include <sdl/Util/Debug.hpp>


namespace sdl {
namespace Util {

template <class T>
struct is_void
{
  enum {value = 0};
};

template <>
struct is_void<void>
{
  enum {value = 1};
};

template <class T, class Enable = void>
struct is_peekable
{
  enum {value = 0};
};

template <class T>
struct is_peekable<T, typename T::Peekable>
{
  enum {value = 1};
};

template <class T, class Enable = void>
struct is_non_peekable
{
  enum {value = 0};
};

template <class T>
struct is_non_peekable<T, typename T::NonPeekable>
{
  enum {value = 1};
};

template <class T>
struct is_generator
{
  enum {value = is_peekable<T>::value || is_non_peekable<T>::value};
};

struct NonPeekableT {};
struct PeekableT {};

template <class Gen, class Vec>
typename boost::enable_if<is_non_peekable<Gen>, std::size_t>::type
appendGenerate(Gen &gen, Vec &vec, std::size_t max)
{
  UTIL_DBG_MSG(12, "non-peekable generate(max)");
  std::size_t n = 0;
  vec.reserve(vec.size()+max);
  for (; n<max && gen; ++n)
    vec.push_back(gen());
  return n;
}

template <class Gen, class Vec>
typename boost::enable_if<is_non_peekable<Gen>, std::size_t>::type
appendGenerate(Gen &gen, Vec &vec)
{
  UTIL_DBG_MSG(12, "non-peekable generate");
  std::size_t n = 0;
  for (; gen; ++n) {
    vec.push_back(gen());
  }
  return n;
}

template <class Gen, class Visit>
typename boost::enable_if<is_non_peekable<Gen>, std::size_t>::type
visitGenerate(Gen &gen, Visit const& visit, std::size_t max)
{
  UTIL_DBG_MSG(12, "non-peekable visit(max)");
  std::size_t n = 0;
  for (; n<max && gen; ++n)
    if (!visit(gen()))
      break;
  return n;
}

template <class Gen, class Visit>
typename boost::enable_if<is_non_peekable<Gen>, std::size_t>::type
visitGenerate(Gen &gen, Visit const& visit)
{
  UTIL_DBG_MSG(12, "non-peekable visit");
  std::size_t n = 0;
  for (; gen; ++n)
    if (!visit(gen()))
      break;
  return n;
}


template <class Gen, class Vec>
typename boost::enable_if<is_peekable<Gen>, std::size_t>::type
appendGenerate(Gen &gen, Vec &vec, std::size_t max)
{
  UTIL_DBG_MSG(12, "peekable append(max)");
  std::size_t n = 0;
  vec.reserve(vec.size()+max);
  for (; n<max && gen; ++n) {
    vec.push_back(gen.peek());
    gen.pop();
  }
  return n;
}

template <class Gen, class Vec>
typename boost::enable_if<is_peekable<Gen>, std::size_t>::type
appendGenerate(Gen &gen, Vec &vec)
{
  UTIL_DBG_MSG(12, "peekable append");
  std::size_t n = 0;
  for (; gen; ++n) {
    vec.push_back(gen.peek());
    gen.pop();
  }
  return n;
}

template <class Gen, class Visit>
typename boost::enable_if<is_peekable<Gen>, std::size_t>::type
visitGenerate(Gen &gen, Visit const& visit, std::size_t max)
{
  UTIL_DBG_MSG(12, "peekable visit(max)");
  std::size_t n = 0;
  for (; n<max && gen; ++n) {
    if (!visit(gen.peek())) {
      gen.pop();
      break;
    }
    gen.pop();
  }
  return n;
}

template <class Gen, class Visit>
typename boost::enable_if<is_peekable<Gen>, std::size_t>::type
visitGenerate(Gen &gen, Visit const& visit)
{
  UTIL_DBG_MSG(12, "peekable visit");
  std::size_t n = 0;
  for (; gen; ++n) {
    if (!visit(gen.peek())) {
      gen.pop();
      break;
    }
    gen.pop();
  }
  return n;
}

/**
   we implicitly convert to this to supply a default printer. the conversion means it loses out to overrides
*/
struct GeneratorDesc {
  friend inline std::ostream& operator<<(std::ostream &out,
                                         GeneratorDesc const& self) {
    out << "(generator)";
    return out;
  }
};

/**
   helps define required GeneratorTag, result_type, value_type.
*/
template <class Value, class Tag>
struct GeneratorTraitsBase
{
  typedef Value result_type; // this is what the generator returns
  // result_type could be value_type or reference and things should still work.
  typedef Tag GeneratorTag;
  operator GeneratorDesc() const { return GeneratorDesc(); }
 private:
  typedef typename boost::remove_reference<result_type>::type maybe_const_value_type;
 public:
  typedef typename boost::remove_const<maybe_const_value_type>::type value_type; // you can always use this to store a copy (e.g. making a nonpeekable generator peekable)
  typedef value_type const& reference; // and you can return this from a stored value_type, of course.
};

// void for enable_if<Gen::NonPeekable>
template <class Derived, class Value, class Tag>
struct GeneratorTraits : GeneratorTraitsBase<Value, Tag>
{
  typedef void NonPeekable;
  static inline bool peekable() { return false; }
  bool operator !() const { return !(bool)(*(Derived const*)this); }
};

template <class Derived, class Value>
struct GeneratorTraits<Derived, Value, PeekableT> : GeneratorTraitsBase<Value, PeekableT>
{
  typedef void Peekable;
  static inline bool peekable() { return true; }
  bool operator !() const { return !(bool)(*(Derived const*)this); }
};

// usage: struct MyGenerator: public GeneratorBase<MyGenerator, int, PeekableT> {};
template <class Derived, class Value, class Tag>
struct GeneratorBase : GeneratorTraits<Derived, Value, Tag>
{
  typedef Value value_type; //typename Derived::value_type
  value_type get() { return (*(Derived*)this)(); }
  void got() {}
};

// this will have the wrong semantics entirely if iterators are ever copied by value:

// you can a peekable generator as an input iterator range only if you can create a finished "end" of same type that compares false. then you're equal to end exactly when you're done too
template <class Derived, class Value>
class GeneratorAsIterator
    : public boost::iterator_facade<
  Derived
  , Value
  , std::input_iterator_tag
  , Value const&>
{
  friend class boost::iterator_core_access;

  void increment()
  {
    static_cast<Derived*>(this)->pop();
  }

  Value const& dereference() const
  {
    return static_cast<Derived const*>(this)->peek();
  }

  // generator can only be compared to end()
  bool equal(GeneratorAsIterator const& other) const
  {
    return (!static_cast<Derived const&>(*this)) &&
        (!static_cast<Derived const&>(other));
  }
};

template <class Derived, class Value>
struct GeneratorBase<Derived, Value, PeekableT>
    : GeneratorTraits<Derived, Value, PeekableT>
//, GeneratorAsIterator<Derived, Value>
{
  typedef typename GeneratorBase::value_type value_type;
  value_type operator()() { // values returned by reference may not remain valid for a peekable gen, after you pop.
    value_type v = ((Derived*)this)->peek();
    ((Derived*)this)->pop();
    return v;
  }
  Value get() { return (*(Derived*)this).peek(); }
  void got() { ((Derived*)this)->pop(); }
};

template <class Value> struct NullGenerator
: GeneratorBase<NullGenerator<Value>, Value, PeekableT> {
  // cppcheck-suppress nullPointer
  Value const& peek() const { assert(0); return *(Value const*)0; } // never (correctly) called
  void pop() const {}
  operator bool() const { return false; }
};

template <class Value>
struct UnitGenerator
    : GeneratorTraits<UnitGenerator<Value>, Value, PeekableT>
{
  typedef Value result_type;
  typedef PeekableT Tag;
  typedef void Peekable;
  template <class X>
  UnitGenerator(X const& result)
      : result(result)
      , called(false) {}
  UnitGenerator() : called(true) {} // for using as input iterator range
  Value const& peek() const { return result; }
  Value const& get() const { return result; }
  Value const& operator()() {
    called = true;
    return result;
  }
  void got() { called = true; }
  void pop() { called = true; }
  operator bool() const { return !called; }
 private:
  Value result;
  bool called;
};

template <class V>
UnitGenerator<V> makeUnitGenerator(V const& v = V()) {
  return UnitGenerator<V>(v);
}

template <class Func, class Result = typename Func::result_type>
struct FuncGenerator
    : GeneratorBase<FuncGenerator<Func, Result>, Result, NonPeekableT> {
  typedef Result result_type;
  typedef Func Function;
  Function func;
  FuncGenerator(FuncGenerator const& o) : func(o.func) {}
  FuncGenerator(Func func) : func(func) {}
  result_type operator()() const { return func(); }
  operator bool() const { return true; }
};

template <class Func>
FuncGenerator<Func> makeFuncGenerator(Func const& func) {
  return FuncGenerator<Func>(func);
}


}}

#endif
