/** \file

    generalization of boost/type_traits/remove_pointer.hpp to cover shared_ptr and intrusive_ptr also.
*/

#ifndef REMOVEPOINTER_JG20121228_HPP
#define REMOVEPOINTER_JG20121228_HPP
#pragma once

#include <boost/type_traits/remove_pointer.hpp>
#include <boost/type_traits/remove_const.hpp>

namespace sdl { namespace Util {

template <class Ptr, class Enable = void>
struct RemovePointer : boost::remove_pointer<Ptr> {};

template <class T>
struct RemovePointer<boost::intrusive_ptr<T>, void> { typedef T type; };

template <class T>
struct RemovePointer<shared_ptr<T>, void> { typedef T type; };

template <class T>
struct RemoveConstPointer {
  typedef typename boost::remove_const<typename RemovePointer<T>::type>::type type;
};

}}

#endif
