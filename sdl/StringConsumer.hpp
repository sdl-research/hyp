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
/** \file

    interface: accept a string and do something.
*/

#ifndef STRINGCONSUMER_JG2012719_HPP
#define STRINGCONSUMER_JG2012719_HPP
#pragma once

#include <graehl/shared/warn.hpp>

namespace sdl {

typedef graehl::string_consumer StringConsumer;  // sdl::function(string) without newline
typedef graehl::warn_consumer StringOut;  // StringConsumer c=StringOut(cerr, "PREFIX: ");
typedef graehl::ignore StringIgnore;  // StringConsumer c=StringIgnore();

/// can cast or assign to StringConsumer
template <class StringConsumerImpl>
struct StringConsumerRef {
  StringConsumerImpl* impl;
  StringConsumerRef(StringConsumerImpl& impl) : impl(&impl) {}
  void operator()(std::string const& str) const { (*impl)(str); }
};

/// can cast or assign to StringConsumer
template <class T>
StringConsumerRef<T> stringConsumerRef(T& x) {
  return StringConsumerRef<T>(x);
}


}

#endif
