/** \file

    interface: accept a string and do something.
*/

#ifndef STRINGCONSUMER_JG2012719_HPP
#define STRINGCONSUMER_JG2012719_HPP
#pragma once

#include <sdl/graehl/shared/warn.hpp>

namespace sdl {

typedef graehl::string_consumer StringConsumer; // boost::function(string) without newline
typedef graehl::warn_consumer StringOut; // StringConsumer c=StringOut(cerr, "PREFIX: ");
typedef graehl::ignore StringIgnore; // StringConsumer c=StringIgnore();

/// can cast or assign to StringConsumer
template <class StringConsumerImpl>
struct StringConsumerRef {
  StringConsumerImpl *impl;
  StringConsumerRef(StringConsumerImpl &impl)
      : impl(&impl)
  {}
  void operator()(std::string const& str) const {
    (*impl)(str);
  }
};

/// can cast or assign to StringConsumer
template <class T>
StringConsumerRef<T> stringConsumerRef(T &x) {
  return StringConsumerRef<T>(x);
}

}

#endif
