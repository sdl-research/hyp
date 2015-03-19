/** \file

    characters we escape inside single or double quotes for hypergraph format.
*/

#ifndef QUOTE_LW20111219_HPP
#define QUOTE_LW20111219_HPP
#pragma once

#include <sdl/Util/EscapeChars.hpp>

namespace sdl {
namespace Util {

struct DoubleQuoteEsc : public EscapeChars
{
  DoubleQuoteEsc()
  {
    add()
        ('\a', "\\a")('\b', "\\b")('\f', "\\f")('\n', "\\n")
        ('\r', "\\r")('\t', "\\t")('\v', "\\v")('\\', "\\\\")
        ('"', "\\\"");
  }
};

struct SingleQuoteEsc : public EscapeChars
{
  SingleQuoteEsc()
  {
    add()
        ('\'',"\\'")('\\',"\\\\");
  }
};


}}

#endif
