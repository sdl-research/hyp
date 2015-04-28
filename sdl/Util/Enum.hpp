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
/** file

   The SDL_ENUM macro is a mechanism to define an
   enum, In addition to the enum definition this macro
   also generates a EnumTypeInfo<> class that has
   emit and parse methods to convert an enum instance
   to/from a string.
   The macro also generates methods required to use the
   enum with the configure library.
   e.g.
   SDL_ENUM(MyEnum, 2, (Test1, Test2))

   yields:

   enum MyEnum {
   kTest1 ,
   kTest2 ,
   knMyEnum
   };

  template <>
  struct EnumTypeInfo<MyEnum> {
   public:
    EnumTypeInfo() {}
    static inline MyEnum begin() { return (MyEnum)0; }
    static inline MyEnum end() { return knMyEnum; }
    static inline unsigned size() { return (unsigned)knMyEnum; }
    static inline bool contains(std::string const& str) { return parse(str) != knMyEnum; }
  };
  inline void string_to_impl(const std::string& str, MyEnum& out) {
    out = EnumTypeInfo<MyEnum>::parse(str);
    if (out == knMyEnum)
      SDL_THROW_LOG(Enum, ::sdl::ConfigException,
                    "Invalid " #MyEnum " enum: " << str << " - should be one of "
                                               << EnumTypeInfo<MyEnum>::getInfo());
  }
  inline MyEnum getValue(MyEnum, const std::string& val) { return EnumTypeInfo<MyEnum>::parse(val); }
  inline MyEnum getValueExact(MyEnum, const std::string& val) { return EnumTypeInfo<MyEnum>::parseExact(val);
 }
  inline std::string type_string(MyEnum const& in);
  inline std::string allowed_values(MyEnum const& in);
  inline std::string example_value(MyEnum const& in);
  inline bool contains(MyEnum, std::string const& str) { return EnumTypeInfo<MyEnum>::contains(str); }
  struct MyEnumMyEnums {
    std::string canonical[knMyEnum];
    std::string cased[knMyEnum];
    MyEnumMyEnums() {
      for (unsigned i = 0; i < knMyEnum; ++i) {
        canonical[i] = EnumTypeInfo<MyEnum>::emit((MyEnum)i);
        cased[i] = EnumTypeInfo<MyEnum>::emitCased((MyEnum)i);
      }
    }
    std::string const& operator[](unsigned i) const { return cased[i]; }
  };
  MyEnumspace {
  MyEnumMyEnums kMyEnumsMyEnum;
  }
  inline std::string const& getString(MyEnum val) { return kMyEnumsMyEnum.canonical[val]; }
  inline bool valid(MyEnum val) { return (int)val >= 0 && val < knMyEnum; }
  inline void assertValid(MyEnum val) {
    if (!valid(val)) SDL_THROW_LOG(Enum, ::sdl::IndexException, #MyEnum " value out of range: " << (int)val);
  }
  inline std::string const& to_string_impl(MyEnum val) {
    assertValid(val);
    return kMyEnumsMyEnum.canonical[val];
  }
  inline std::string const& casedString(MyEnum val) {
    assertValid(val);
    return kMyEnumsMyEnum.cased[val];
  }
  inline std::istream& operator>>(std::istream& in, MyEnum& val) {
    std::string str;
    in >> str;
    string_to_impl(str, val);
    return in;
  }
  inline std::ostream& operator<<(std::ostream& out, MyEnum val) { return out << to_string_impl(val); }
  struct MyEnumIterator : public boost::iterator_facade<MyEnumIterator, MyEnum, boost::forward_traversal_tag>
 {
    MyEnumIterator(bool) : val_((MyEnum)0) {}
    MyEnumIterator(MyEnum p) : val_(p) {}
    MyEnumIterator() : val_(knMyEnum) {}

   private:
    friend class boost::iterator_core_access;
    void increment() { ++val_; }
    bool equal(MyEnumIterator const& other) const { return val_ == other.val_; }
    MyEnum& dereference() const { return (MyEnum&)val_; }
    unsigned val_;
  };
  typedef boost::iterator_range<MyEnumIterator> MyEnumRange;
  inline MyEnumRange allMyEnum() { return MyEnumRange(MyEnumIterator(false), MyEnumIterator()); }

 **/

#ifndef SDL_UTIL_ENUM_HPP_
#define SDL_UTIL_ENUM_HPP_
#pragma once

#include <sdl/Util/EnumDetail.hpp>

/**
   e.g. c++11 doesn't need size: SDL_ENUM(name, BOOST_PP_TUPLE_TO_LIST(elems)
**/
#define SDL_ENUM_DEF(name, elems)                                                               \
  SDL_DETAIL_ENUM_TYPE_INFO(name, BOOST_PP_LIST_TRANSFORM(SDL_DETAIL_ENUM_PREPEND_k, k, elems), \
                            BOOST_PP_LIST_TRANSFORM(SDL_DETAIL_TOSTRING, _, elems))

#define SDL_ENUM(name, size, elems) SDL_ENUM_DEF(name, BOOST_PP_TUPLE_TO_LIST(size, elems))


#endif
