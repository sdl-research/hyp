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

    fix TinyXMLCPP warnings by including this first.
*/

#ifndef TINYXML_JG_2014_01_17_HPP
#define TINYXML_JG_2014_01_17_HPP
#pragma once

#include <graehl/shared/warning_compiler.h>
CLANG_DIAG_OFF(overloaded-virtual)
#include <tinyxmlcpp/ticpp.h>
CLANG_DIAG_ON(overloaded-virtual)

#include <graehl/shared/string_to.hpp>
#include <tinyxmlcpp/ticpprc.h>
#include <tinyxmlcpp/tinystr.h>
#include <tinyxmlcpp/tinyxml.h>

namespace sdl {
namespace Util {

inline bool xmlAttribute(ticpp::Element& xml, std::string const& name) {
  return xml.HasAttribute(name);
}

inline bool xmlAttribute(ticpp::Element* xml, std::string const& name) {
  return xml && xml->HasAttribute(name);
}

template <class Val>
bool maybeXmlAttribute(ticpp::Element& xml, std::string const& name, Val* val) {
  if (xml.HasAttribute(name)) {
    if (val) graehl::string_to(xml.GetAttribute(name), *val);
    return true;
  } else
    return false;
}

template <class Val>
bool maybeXmlAttribute(ticpp::Element* xml, std::string const& name, Val* val) {
  return xml && maybeXmlAttribute(*xml, name, val);
}

/// \return whether attribute existed. if so string_to(attr, *val) if val
/// if not, maybe set to orElse and return false
template <class Val, class OrElseVal>
bool xmlAttribute(ticpp::Element* xml, std::string const& name, Val* val, OrElseVal const& orElse) {
  if (maybeXmlAttribute(xml, name, val))
    return true;
  else {
    *val = orElse;
    return false;
  }
}

/// \return whether attribute existed. if so string_to(attr, *val) if val
/// if not, maybe set to orElse and return false
template <class Val, class OrElseVal>
bool xmlAttribute(ticpp::Element& xml, std::string const& name, Val* val, OrElseVal const& orElse) {
  return xmlAttribute(&xml, name, orElse);
}

/// \return whether attribute existed. if so string_to(attr, *val) if val
/// if not, maybe set to Val() and return false
template <class Val>
bool xmlAttribute(ticpp::Element& xml, std::string const& name, Val* val) {
  return xmlAttribute(xml, name, val, Val());
}

template <class Val>
bool xmlAttribute(ticpp::Element* xml, std::string const& name, Val* val) {
  return xmlAttribute(xml, name, val, Val());
}


}}

#endif
