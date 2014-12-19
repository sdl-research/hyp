/** \file

   The SDL_ENUM macro is a mechanism to define an
   enum, In addition to the enum definition this macro
   also generates a EnumTypeInfo<> class that has
   emit and parse methods to convert an enum instance
   to/from a string.
   The macro also generates methods required to use the
   enum with the configure library.
   e.g.
   SDL_ENUM(TestEnum, 2, (Test1, Test2))

   yields:

   enum TestEnum {
   kTest1 ,
   kTest2
   };

   template <typename T> struct EnumTypeInfo;

   template<> struct EnumTypeInfo<TestEnum> {
   public:
   EnumTypeInfo() {
   }

   static std::string emit(const TestEnum& val) {
     std::string val;
     switch(val) {
       case kTest1:
         val = "Test1";
         break;
       case kTest2:
         val = "Test2";
         break;
       default:
         throw std::runtime_error("Enum value out of range");
     };
     return val;
   }

   static TestEnum parse(const std::string& val) {
     if (val == "Test1" || val == "0")
       return kTest1;
     else if (val == "Test2" || val == "1")
       return kTest2;
     else
       throw std::runtime_error("Invalid enum string:"+val);
   }

   static std::string getInfo() {
     return "" "Test1" " | " "Test2" ;
   }
   };

   inline std::string to_string_impl(const TestEnum& val) {
   return EnumTypeInfo<TestEnum>::emit(val);
   }

   inline void string_to_impl(const std::string& val, TestEnum& out) {
   out = EnumTypeInfo<TestEnum>::parse(val);
   }

   inline std::string type_string(TestEnum& in) {
   return std::string("TestEnum") + std::string(": [ ") +
       EnumTypeInfo<TestEnum>::getInfo() +
       std::string(" ]");
   return info;
   }

 **/

#ifndef SDL_UTIL_ENUM_HPP_
#define SDL_UTIL_ENUM_HPP_
#pragma once

#include <sdl/Util/EnumDetail.hpp>

#define SDL_ENUM(name, size, elems) SDL_DETAIL_ENUM_DEF(name, BOOST_PP_TUPLE_TO_LIST(size, elems))

#endif
