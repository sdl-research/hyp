




#ifndef QUOTE_LW20111219_HPP
#define QUOTE_LW20111219_HPP







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





