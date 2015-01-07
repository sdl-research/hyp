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

    The following macros are not intended for direct use,
    they are used by SDL_ENUM defined in Enum.hpp
    to produce the Enum definition.
*/

#ifndef SDL_UTIL_ENUMDETAIL_HPP_
#define SDL_UTIL_ENUMDETAIL_HPP_
#pragma once

#include <stdexcept>
#include <algorithm>

#include <boost/preprocessor/list/at.hpp>
#include <boost/preprocessor/list/enum.hpp>
#include <boost/preprocessor/tuple/to_list.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/list/transform.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/list/for_each_i.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/control/if.hpp>

#include <graehl/shared/warning_compiler.h>
#include <graehl/shared/warning_push.h>
#include <boost/range/iterator_range.hpp>
#include <graehl/shared/warning_pop.h>

#include <sdl/Util/strcasecmp.hpp>
#include <sdl/Util/AsciiCase.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Exception.hpp>

namespace sdl {
namespace Util {
inline void canonicalizeEnumName(std::string& str) {
  std::replace(str.begin(), str.end(), '_', '-');
  std::transform(str.begin(), str.end(), str.begin(), sdl::Util::AsciiDowncaseChar());
}

inline std::string canonicalEnumName(std::string str) {
  canonicalizeEnumName(str);
  return str;
}
}
}


/**
   helpers for preprocessor enumeration of enum type names:
**/
#define SDL_DETAIL_EMIT_TO_STRING_CASE(r, data, i, elem) \
  case elem:                                             \
    str = BOOST_PP_LIST_AT(data, i);                     \
    break;

#define SDL_DETAIL_emitCased(r, data, i, elem) \
  case elem:                                   \
    return BOOST_PP_LIST_AT(data, i);          \
    break;

/**
   generates static std::string EnumTypeInfo::emit(val) method. (result is cached in namesEnumType)
**/
#define SDL_DETAIL_EMIT_TO_STRING(name, elems, nameList) \
  static std::string emit(const name& val) {                                               \
    std::string str;                                                                       \
    switch (val) {                                                                         \
      BOOST_PP_LIST_FOR_EACH_I(SDL_DETAIL_EMIT_TO_STRING_CASE, nameList, elems) \
      default:                                                                             \
        SDL_THROW_LOG(Enum, sdl::IndexException, "Enum value out of range: " << (int)val); \
    };                                                                                     \
    sdl::Util::canonicalizeEnumName(str);                                                  \
    return str;                                                                            \
  }                                                                                        \
  static std::string emitCased(const name& val) {                                          \
    switch (val) {                                                                         \
      BOOST_PP_LIST_FOR_EACH_I(SDL_DETAIL_emitCased, nameList, elems) \
      default:                                                                             \
        SDL_THROW_LOG(Enum, sdl::IndexException, "Enum value out of range: " << (int)val); \
    };                                                                                     \
  }

/**
   generates the if/else if statements in
   parse call.
**/
#define SDL_DETAIL_PARSE_STRING_TO_ENUM_IF(r, data, i, elem) \
  BOOST_PP_IF(i, else if, if)                                \
  (!sdl::Util::strcasecmp(str, elem) || str == BOOST_PP_STRINGIZE(i)) return BOOST_PP_LIST_AT(data, i);

#define SDL_DETAIL_PARSE_EXACT_STRING_TO_ENUM_IF(r, data, i, elem) \
  BOOST_PP_IF(i, else if, if)                                      \
  (str == elem) return BOOST_PP_LIST_AT(data, i);

/**
   generates EnumInfo<enumtype>::parse(str). this converts (case insensitive, and
   - -> _) string to enum value of same name (but without the leading k for
   Constant). it also converts integral or bool values to the corresponding enum
   value. (the bool part is for config backward compatability and could be
   dropped if we want to update the appropriate regression tests and old CT
   config files; I don't recommend it).

   could also allow yaml boolean alternatives: yes on, no off but just in case
   you defined your own kOn I wouldn't presume.
*/
#define SDL_DETAIL_PARSE_STRING_TO_ENUM(name, elems, nameList) \
  static name parse(std::string str) {                                            \
    sdl::Util::inplaceAsciiAllDowncase(str);                                      \
    std::replace(str.begin(), str.end(), '-', '_');                               \
    if (str == "false" || str == "n") return (name)0;                             \
    if (kn##name > 1) {                                                           \
      if (str == "true" || str == "y") return (name)1;                            \
    }                                                                             \
    BOOST_PP_LIST_FOR_EACH_I(SDL_DETAIL_PARSE_STRING_TO_ENUM_IF, elems, nameList) \
    else return kn##name;                                                         \
  }

#define SDL_DETAIL_PARSE_EXACT_STRING_TO_ENUM(name, elems, nameList) \
  static name parseExact(std::string const& str) {                                      \
    BOOST_PP_LIST_FOR_EACH_I(SDL_DETAIL_PARSE_EXACT_STRING_TO_ENUM_IF, elems, nameList) \
    else return kn##name;                                                               \
  }

/**
   generates help string
**/
#define SDL_DETAIL_ENUM_GET_INFO_HELPER(r, data, i, elem) \
  BOOST_PP_IF(i, +" | " +, ) sdl::Util::canonicalEnumName(elem)

/**
   generates the getInfo() method.
**/
#define SDL_DETAIL_ENUM_GET_INFO(name, nameList) \
  static std::string getInfo() {                                                   \
    return BOOST_PP_LIST_FOR_EACH_I(SDL_DETAIL_ENUM_GET_INFO_HELPER, _, nameList); \
  }

/**
   Macro generates the EnumTypeInfo

   warning: this goes in the same namespace as the enum. so there are
   ns::EnumTypeInfo<ns::YourEnum> littered all over. convert to free fns only?
**/
#define SDL_DETAIL_ENUM_TYPE_INFO(name, elems, nameList) \
  enum name { BOOST_PP_LIST_ENUM(elems), kn##name };                                                          \
  template <typename T>                                                                                       \
  struct EnumTypeInfo;                                                                                        \
  template <>                                                                                                 \
  struct EnumTypeInfo<name> {                                                                                 \
   public:                                                                                                    \
    EnumTypeInfo() {}                                                                                         \
    SDL_DETAIL_EMIT_TO_STRING(name, elems, nameList) \
    SDL_DETAIL_PARSE_STRING_TO_ENUM(name, elems, nameList) \
    SDL_DETAIL_PARSE_EXACT_STRING_TO_ENUM(name, elems, nameList) \
    SDL_DETAIL_ENUM_GET_INFO(name, nameList) \
    static inline name begin() { return (name)0; }                                                            \
    static inline name end() { return kn##name; }                                                             \
    static inline unsigned size() { return (unsigned)kn##name; }                                              \
    static inline bool contains(std::string const& str) { return parse(str) != kn##name; }                    \
  };                                                                                                          \
  inline void string_to_impl(const std::string& str, name& out) {                                             \
    out = EnumTypeInfo<name>::parse(str);                                                                     \
    if (out == kn##name)                                                                                      \
      SDL_THROW_LOG(Enum, ::sdl::ConfigException, \
                    "Invalid " #name " enum: " << str << " - should be one of "                               \
                                               << EnumTypeInfo<name>::getInfo());                             \
  }                                                                                                           \
  inline name getValue(name, const std::string& val) { return EnumTypeInfo<name>::parse(val); }               \
  inline name getValueExact(name, const std::string& val) { return EnumTypeInfo<name>::parseExact(val); }     \
  inline std::string type_string(name const& in) {                                                            \
    return std::string(BOOST_PP_STRINGIZE(name)) + std::string(": ") + EnumTypeInfo<name>::getInfo();         \
  }                                                                                                           \
  inline std::string allowed_values(name const& in) { return EnumTypeInfo<name>::getInfo(); }                 \
  inline std::string example_value(name const& in) { return BOOST_PP_LIST_AT(nameList, 0); }                  \
  inline bool contains(name, std::string const& str) { return EnumTypeInfo<name>::contains(str); }            \
  struct name##Names {                                                                                        \
    std::string canonical[kn##name];                                                                          \
    std::string cased[kn##name];                                                                              \
    name##Names() {                                                                                           \
      for (unsigned i = 0; i < kn##name; ++i) {                                                               \
        canonical[i] = EnumTypeInfo<name>::emit((name)i);                                                     \
        cased[i] = EnumTypeInfo<name>::emitCased((name)i);                                                    \
      }                                                                                                       \
    }                                                                                                         \
    std::string const& operator[](unsigned i) const { return cased[i]; }                                      \
  };                                                                                                          \
  namespace {                                                                                                 \
  name##Names kNames##name;                                                                                   \
  }                                                                                                           \
  inline std::string const& getString(name val) { return kNames##name.canonical[val]; }                       \
  inline bool valid(name val) { return (int)val >= 0 && val < kn##name; }                                     \
  inline void assertValid(name val) {                                                                         \
    if (!valid(val)) SDL_THROW_LOG(Enum, ::sdl::IndexException, #name " value out of range: " << (int)val); \
  }                                                                                                           \
  inline std::string const& to_string_impl(name val) {                                                        \
    assertValid(val);                                                                                         \
    return kNames##name.canonical[val];                                                                       \
  }                                                                                                           \
  inline std::string const& casedString(name val) {                                                           \
    assertValid(val);                                                                                         \
    return kNames##name.cased[val];                                                                           \
  }                                                                                                           \
  inline std::istream& operator>>(std::istream& in, name& val) {                                              \
    std::string str;                                                                                          \
    in >> str;                                                                                                \
    string_to_impl(str, val);                                                                                 \
    return in;                                                                                                \
  }                                                                                                           \
  inline std::ostream& operator<<(std::ostream& out, name val) { return out << to_string_impl(val); }         \
  struct name##Iterator : public boost::iterator_facade<name##Iterator, name, boost::forward_traversal_tag> { \
    name##Iterator(bool) : val_((name)0) {}                                                                   \
    name##Iterator(name p) : val_(p) {}                                                                       \
    name##Iterator() : val_(kn##name) {}                                                                      \
                                                                                                              \
   private:                                                                                                   \
    friend class boost::iterator_core_access;                                                                 \
    void increment() { ++val_; }                                                                              \
    bool equal(name##Iterator const& other) const { return val_ == other.val_; }                              \
    name& dereference() const { return (name&)val_; }                                                         \
    unsigned val_;                                                                                            \
  };                                                                                                          \
  typedef boost::iterator_range<name##Iterator> name##Range;                                                  \
  inline name##Range all##name() { return name##Range(name##Iterator(false), name##Iterator()); }

// end SDL_DETAIL_ENUM_TYPE_INFO

#define SDL_DETAIL_TOSTRING(d, data, elem) BOOST_PP_STRINGIZE(elem)

/**
   prepends k to each Enum name per coding standards.
**/
#define SDL_DETAIL_ENUM_PREPEND_k(d, data, elem) BOOST_PP_CAT(data, elem)

/**
   generates enum declaraion, definition and EnumTypeInfo<> definition
**/
#define SDL_DETAIL_ENUM_DEF(name, elems) \
  SDL_DETAIL_ENUM_TYPE_INFO(name, BOOST_PP_LIST_TRANSFORM(SDL_DETAIL_ENUM_PREPEND_k, k, elems), \
                            BOOST_PP_LIST_TRANSFORM(SDL_DETAIL_TOSTRING, _, elems))

#endif
