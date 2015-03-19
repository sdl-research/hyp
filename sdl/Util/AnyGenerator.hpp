/** \file

    type-erased Generator - see Generator.hpp for concept details

    if you want to type-erase the implementation, an AnyGenerator is superior in
    implementation effort and run-time expense to a pair of dynamic-type
    iterators,
    e.g. http://www.boost.org/doc/libs/1_49_0/libs/range/doc/html/range/reference/ranges/any_range.html

    AnyIterator<V> is a lightweight copyable handle to an underlying generator
    implementation. copying it won't make a copy of the underlying data. We
    haven't defined a clone() yet.

    AnyGenerator<Value, Tag>
    with Tag=NonPeekableT is a type erased Generator
    with Tag=PeekableT is a type erased PeekableGenerator, and can be used as an input iterator -

    a peekable AnyGenerator is also an input iterator that compares not equal to everything unless it and
    note: iterators will only compare equal when they're done. i.e. for PeekableT g you could use the boost::range (g, g)

    note: peekable anygenerators still return by value. you could use an AnyGenerator<Value const&> or Value * instead if you want that efficiency.
*/

#ifndef SDL_UTIL__ANY_GENERATOR_HPP
#define SDL_UTIL__ANY_GENERATOR_HPP
#pragma once

#include <sdl/Util/Generator.hpp>
#include <sdl/Util/RefCount.hpp>

namespace sdl {
namespace Util {

template <class V>
struct IGeneratorBase : intrusive_refcount<IGeneratorBase<V>, RefCount>
{
  //TODO: for hard-to debug reasons, AnyVisitor<V const&> (takes arg by const ref) causes segfault - couldn't get informative line numbers in gdb or valgrind. probably because AnyVisitor::peek returns by value not ref?

  virtual std::size_t visit(AnyVisitor<V> const& v) = 0; // these are more efficient by virtue of a single dynamic dispatch per item rather than more() followed by next()
  virtual std::size_t visit(AnyVisitor<V> const& v, std::size_t max) = 0;
  virtual std::size_t append(std::vector<V> &v) = 0;
  virtual std::size_t append(std::vector<V> &v, std::size_t max) = 0;

  typedef IGeneratorBase<V> self_type;
  friend void intrusive_ptr_add_ref(self_type *p) { p->add_ref();  }
  friend void intrusive_ptr_release(self_type *p) { p->release(p); }

  //TODO: specialized free fns append() and visit() using virtual - less virtual dispatch
  inline operator bool() const {return more(); }
  inline V operator()() const {return next(); }
  virtual bool peekable() const = 0;
  virtual bool more() const = 0;
  virtual V next() = 0;
  virtual ~IGeneratorBase() {}
  typedef boost::intrusive_ptr<IGeneratorBase<V> > Pointer;
};
//TODO@JG: non-virtual version of AnyGenerator (lightweight handle to underlying shared generator, with same generator interface. for now, use smart ptr to concrete generator and deref it).

template <class V, class T>
struct IGenerator : IGeneratorBase<V>, GeneratorBase<IGenerator<V, T>, V, NonPeekableT>  {
  virtual bool peekable() const OVERRIDE { return false; }
};

template <class V>
struct IGenerator<V, PeekableT>
    : IGeneratorBase<V>, GeneratorBase<IGenerator<V, PeekableT>, V, PeekableT> {
  virtual bool peekable() const OVERRIDE { return true; } // means you can cast parent to this type
  virtual void pop() = 0;
  virtual V peek() const = 0;
};

// G is the concrete Generator implementation type, or, if T=PeekableT, G may be an iterator (adapted to
template <class GenImpl, class V, class T>
struct AnyGeneratorImplBase
    : IGenerator<V, T>
    , GeneratorBase<AnyGeneratorImplBase<GenImpl, V, T>, V, T>
{
  virtual std::size_t visit(AnyVisitor<V> const& v) OVERRIDE {
    UTIL_DBG_MSG(12, "Any:visit");
    return Util::visitGenerate(g, v);
  }
  virtual std::size_t visit(AnyVisitor<V> const& v, std::size_t max) OVERRIDE {
    UTIL_DBG_MSG(12, "Any:visit(max)");
    return Util::visitGenerate(g, v, max);
  }
  virtual std::size_t append(std::vector<V> &v) OVERRIDE {
    UTIL_DBG_MSG(12, "Any:append");
    return Util::appendGenerate(g, v);
  }
  virtual std::size_t append(std::vector<V> &v, std::size_t max) OVERRIDE {
    UTIL_DBG_MSG(12, "Any:append(max)");
    return Util::appendGenerate(g, v, max);
  }
  virtual bool more() const OVERRIDE { return bool(g); }
  virtual V next() OVERRIDE { return g(); }
  AnyGeneratorImplBase(GenImpl const& g)
      : g(g) {}
  AnyGeneratorImplBase()
      : g() {}
  template <class C1, class C2>
  AnyGeneratorImplBase(C1 const& c1, C2 const& c2) : g(c1, c2) {}
  GenImpl &impl() { return g; }
  GenImpl const &impl() const { return g; }
  GenImpl g;
};

template <class V>
std::size_t appendGenerate(IGeneratorBase<V> &gen, std::vector<V> &vec, std::size_t max)
{ return gen.append(vec, max); }

template <class V>
std::size_t appendGenerate(IGeneratorBase<V> &gen, std::vector<V> &vec)
{ return gen.append(vec); }

template <class V>
std::size_t visitGenerate(IGeneratorBase<V> &gen, AnyVisitor<V> const& visitor, std::size_t max)
{ return gen.visit(visitor, max); }

template <class V>
std::size_t visitGenerate(IGeneratorBase<V> &gen, AnyVisitor<V> const& visitor)
{ return gen.visit(visitor); }

template <class G, class V = typename G::result_type, class T = typename G::GeneratorTag>
struct AnyGeneratorImpl : AnyGeneratorImplBase<G, V, T> {
  AnyGeneratorImpl() : AnyGeneratorImplBase<G, V, T>() {}
  AnyGeneratorImpl(G const& g) : AnyGeneratorImplBase<G, V, T>(g) {}
  template <class C1, class C2>
  AnyGeneratorImpl(C1 const& c1, C2 const& c2) : AnyGeneratorImplBase<G, V, T>(c1, c2) {}
};

template <class G, class V>
struct AnyGeneratorImpl<G, V, PeekableT> : AnyGeneratorImplBase<G, V, PeekableT> {
  AnyGeneratorImpl() : AnyGeneratorImplBase<G, V, PeekableT>() {}
  AnyGeneratorImpl(G const& g) : AnyGeneratorImplBase<G, V, PeekableT>(g) {}
  template <class C1, class C2>
  AnyGeneratorImpl(C1 const& c1, C2 const& c2) : AnyGeneratorImplBase<G, V, PeekableT>(c1, c2) {}
  virtual void pop() { AnyGeneratorImplBase<G, V, PeekableT>::g.pop(); }
  virtual V peek() const { return AnyGeneratorImplBase<G, V, PeekableT>::g.peek(); }
};

/*
  AnyGenerator:
  type erasure of Generator or Peekable (see docs at top of file)
*/

template <class Value>
struct AnyGeneratorBase
{
 protected:
  typedef IGeneratorBase<Value> IGen;
  typedef boost::intrusive_ptr<IGen> IGenPtr;
  IGenPtr pimpl;
 public:

  AnyGeneratorBase() : pimpl() {}

  AnyGeneratorBase(AnyGeneratorBase const& o) : pimpl(o.pimpl) {}

  AnyGeneratorBase(IGen *p) // must be new()
      : pimpl(p) {}

  AnyGeneratorBase& operator = (AnyGeneratorBase const& o)
  {
    pimpl = o.pimpl;
    return *this;
  }

  template <class Generator>
  AnyGeneratorBase& operator = (Generator const& g)
  {
    AnyGeneratorBase(g).swap(*this);
    return *this;
  }

  operator bool() const { return bool(pimpl) && pimpl->more(); }
  Value operator()() { return pimpl->next(); }
  Value get() { return pimpl->next(); }
  void got() {}
  bool peekable() const { return pimpl->peekable(); }
  bool empty() const { return !pimpl; }
  IGen * getBase() const { return pimpl.get(); }
  IGen &operator *() const { return *pimpl; }
  IGen *operator ->() const { return pimpl.get; }
  void swap(AnyGeneratorBase& o)
  {
    assert(pimpl->peekable()==o.pimpl->peekable());
    pimpl.swap(o.pimpl);
  }
};


template <class Value, class Tag = PeekableT>
struct AnyGenerator : AnyGeneratorBase<Value>
    , GeneratorBase<AnyGenerator<Value, Tag>, Value, Tag>
{
  typedef AnyGeneratorBase<Value> Base;
  typedef IGenerator<Value, Tag> Interface;
  typedef Tag GeneratorTag;
  AnyGenerator() {}
  AnyGenerator(AnyGenerator const& o) : Base((Base const&)o) {}
  template <class Generator>
  AnyGenerator(Generator const& g) : Base(new AnyGeneratorImpl<Generator, Value, Tag>(g)) {}
  Interface * getBase() const { return (Interface *)Base::pimpl.get(); }
};

template <class Value>
struct AnyGenerator<Value, PeekableT>
    : AnyGeneratorBase<Value>
    , GeneratorBase<AnyGenerator<Value, PeekableT>, Value, PeekableT>
{
  typedef PeekableT GeneratorTag;
  typedef AnyGeneratorBase<Value> Base;
  typedef IGenerator<Value, PeekableT> Interface;
  template <class Generator>
  struct Impl
  {
    typedef AnyGeneratorImpl<Generator, Value, PeekableT> type;
  };

  AnyGenerator() {}
  AnyGenerator(AnyGenerator const& o) : Base((Base const&)o) {}
  AnyGenerator(typename Base::IGenPtr const& p) : Base(p) {}
  template <class Imp> // Imp = Impl<GeneratorImpl>::type
  AnyGenerator(Imp *newPimpl) : Base(static_cast<IGeneratorBase<Value>*>(newPimpl)) {}
  template <class Generator>
  AnyGenerator(Generator const& g)
      : Base(static_cast<IGeneratorBase<Value>*>(new AnyGeneratorImpl<Generator, Value, PeekableT>(g))) {}
  Value peek() const { return ((Interface *)Base::pimpl.get())->peek(); }
  Value get() const { return ((Interface *)Base::pimpl.get())->peek(); }
  void pop() { return ((Interface *)Base::pimpl.get())->pop(); }
  void got() { return ((Interface *)Base::pimpl.get())->pop(); }
  Interface * getBase() const {
    assert(Base::pimpl->peekable());
    return (Interface *)Base::pimpl.get();
  }
  inline bool peekable() const {
    assert(Base::pimpl->peekable());
    return true;
  }
  Value operator()() { return Base::pimpl->next(); }
};

template <class ConcreteGen>
AnyGenerator<typename ConcreteGen::result_type, typename ConcreteGen::GeneratorTag>
anyGenerator(ConcreteGen const& gen)
{
  return gen;
}


template <class V, class T>
void swap(AnyGenerator<V, T>& g1, AnyGenerator<V, T>& g2)
{
  g1.swap(g2);
}

template <class V>
AnyGenerator<V, PeekableT> makeNullGenerator() {
  return NullGenerator<V>();
}

template <class V>
AnyGenerator<V, PeekableT> makeAnyUnitGenerator(V const& v = V()) {
  return UnitGenerator<V>(v);
}


}}

#endif
