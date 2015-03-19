/** \file

    a visitor v has member bool operator()()Arg) const;

    i.e.

    struct Visitor { bool operator()(Arc *) const {return true;} };
    Visitor v;
    AnyVisitor<Arc *> anyv(v); // v must be an lvalue and live as long as anyv; we save its address
    bool result=anyv(0);

    AnyVisitor models adaptable unary function (result_type, argument_type)

    implementation takes address of a template function, so compares semantically equal only in same compilation unit

    this is faster than a virtual call. roughly the same as non-virtual member function pointer call

    motivation: to make using AnyGenerator sequences more efficient (calling the AnyVisitor happens pre-erasure)
*/

#ifndef SDL_UTIL__ANY_VISITOR_HPP
#define SDL_UTIL__ANY_VISITOR_HPP
#pragma once


namespace sdl {
namespace Util {

/*
  TODO: assuming no performance penalty, use mechanism like boost::function<bool (int)> to make creation of AnyFnPtr from static template fns easier (right now, requires redundantly specifying function type, which could be deduced from the free or member fn ptr name at compile time)
*/
template <class Arg>
struct AnyVisitor
{
  typedef bool result_type;
  typedef Arg argument_type;
  typedef result_type (*FreeFnPtr)(argument_type);
  typedef result_type (*AnyFnPtr)(void *, argument_type);
  template <class Obj>
  struct MemberFn
  {
    template <result_type (Obj::*ObjMemFn)(argument_type)>
    static inline result_type call(void *objv, argument_type a) {
      Obj *obj = static_cast<Obj *>(objv);
      return (obj->*ObjMemFn)(a);
    }
    template <result_type (Obj::*ObjMemFn)(argument_type) const>
    static inline result_type call(void *objv, argument_type a) {
      Obj const*obj = static_cast<Obj *>(objv);
      return (obj->*ObjMemFn)(a);
    }
  };
  template <class A>
  inline result_type operator()(A const& a) const
  {
    return pany(pobj, a);
  }
  template <class A>
  inline result_type operator()(A & a) const
  {
    return pany(pobj, a);
  }
  AnyVisitor() : pany(), pobj() {}
  AnyVisitor(AnyVisitor const& o) : pany(o.pany), pobj(o.pobj) {}
  AnyVisitor &operator = (AnyVisitor const& o) const { pany = o.pany; pobj = o.pobj; }
  // usage: &free<fnname>
  template <result_type (*Func)(argument_type)>
  static inline result_type free(void *, argument_type a) { return (*Func)(a); }
  // usage: AnyVisitor(obj, &MemberFn<&Obj::memberfn>::call)
  template <class Obj>
  AnyVisitor(Obj const& obj, AnyFnPtr anyfnp) : pany(anyfnp), pobj((void *)&obj) {}
  template <class Obj>
  AnyVisitor(Obj const* obj, AnyFnPtr anyfnp) : pany(anyfnp), pobj((void *)obj) {}
  AnyVisitor(AnyFnPtr anyfnp) : pany(anyfnp), pobj() {}
  // WARNING: same fn wrapped in different compilation units -> unequal address of MemberFn or free<fn>
  bool operator==(AnyVisitor const& o) const { return pany==o.pany && pobj==o.pobj; }
  bool operator!=(AnyVisitor const& o) const { return !(pany==o.pany && pobj==o.pobj); }
  //  friend std::size_t hash_value(AnyVisitor const& v) { return (intptr_t)pany>>5+((intptr_t)pobj>>3); }
 private:
  typedef void * AnyVisitor::*SafeBool;
 public:
  operator SafeBool() const { return pany != 0 ? &AnyVisitor::pobj : 0; }
  bool operator!() const { return pany == 0; }
  AnyFnPtr pany; // automatically generated wrapper
  void *pobj; // 0 for free fn
};

}}

#define ANY_VISITOR_FREE(arg, freefn) (sdl::Util::AnyVisitor<arg>(&sdl::Util::AnyVisitor<arg>::free<freefn>))
//#define ANY_VISITOR_FREE(arg, freefn) (sdl::Util::AnyVisitor<arg>(freefn))
#define ANY_VISITOR_MEMBER(Objtype, obj, member, arg)                   \
  (sdl::Util::AnyVisitor<arg>(obj, &sdl::Util::AnyVisitor<arg>::MemberFn<Objtype>::call<&Objtype::member>))

#endif
