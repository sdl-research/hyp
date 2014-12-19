/** \file allow passing function objects by value without copying them (by storing a
   reference and passing arguments to it)
   */

#ifndef SDL_FN_REFERENCE_HPP
#define SDL_FN_REFERENCE_HPP
#pragma once


namespace sdl {
namespace Util {

template <class Fn, class FnReturnType = void>
struct FnReference {
  typedef FnReturnType return_type;
  Fn& x;
  FnReference(Fn& x) : x(x) {}
  FnReference(FnReference const& o) : x(o.x) {}
  template <class Y1>
  return_type operator()(Y1 const& y1) const {
    return x(y1);
  }
  template <class Y1, class Y2>
  return_type operator()(Y1 const& y1, Y2 const& y2) const {
    return x(y1, y2);
  }
};

template <class Fn>
FnReference<Fn, void> visitorReference(Fn& x) {
  return FnReference<Fn, void>(x);
}

template <class Fn>
FnReference<Fn, bool> predicateReference(Fn& x) {
  return FnReference<Fn, bool>(x);
}


}}

#endif
