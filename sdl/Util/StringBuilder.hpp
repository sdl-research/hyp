/** \file

    probably more efficient than stringstream; also gives contiguous
    vector<char> result without necessarily building a std::string object first
    (so can use e.g. xmt/Types.hpp Slice)

    StringBuilder s("1");
    s.append(" plus ")('2').
    s.str(); // "1 plus 2"

*/

#ifndef STRINGBUILDER_JG2012711_HPP
#define STRINGBUILDER_JG2012711_HPP
#pragma once

#include <graehl/shared/string_to.hpp>

namespace sdl {
namespace Util {

using graehl::string_to;
using graehl::to_string_impl;

template <class V>
inline std::string toString(V const& val) {
  return graehl::to_string_impl(val);
}

typedef graehl::string_buffer StringBuffer;
/**
   vector of char with methods for converting to string and appending
   (conversion to string may be faster than ostream <<).
*/
typedef graehl::string_builder StringBuilder;

/**
   WordSpacer sp('.');
   StringBuilder().range(vector, sp);
*/
typedef graehl::word_spacer WordSpacer;

/**
   value object (ref to StringBuilder).
*/
typedef graehl::append_string_builder AppendStringBuilder;

/**
   value object (ref to StringBuilder) appends a newline after every write operation.
*/
typedef graehl::append_string_builder_newline AppendStringBuilderNewline;


}}

#endif
