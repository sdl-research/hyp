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

#include <sdl/Hypergraph/Label.hpp>
#include <sdl/Hypergraph/SymbolPrint.hpp>
#include <sdl/Hypergraph/Types.hpp>

#include <sdl/IVocabulary.hpp>
#include <sdl/Sym.hpp>

#include <sdl/Util/Print.hpp>
#include <sdl/Util/QuoteEsc.hpp>

#include <sdl/Vocabulary/SpecialSymbols.hpp>

#include <boost/spirit/include/karma.hpp>

#ifndef HG_PRINT_ESCAPE_SINGLE_QUOTE
#define HG_PRINT_ESCAPE_SINGLE_QUOTE 0
// FIXME: 1 is similar to HG_PRINT_ESCAPE_NON_ASCII 0 - in that roundtrip doesn't work
#endif

namespace sdl {
namespace Hypergraph {

namespace karma = boost::spirit::karma;

char const* const kDoubleQuote = "\"";

template <typename OutputIterator>
struct QuotedEscapedStringGrammar : karma::grammar<OutputIterator, std::string()> {
  QuotedEscapedStringGrammar()
      // xEscapeNonAscii true = prints all ascii chars - utf8 becomes \x214 etc
      // false = prints utf8 unmolested, except for the below C-style ascii-special char and quote\backslash
      // chars

      // NOTE: in either case, parsing will succeed; e.g. \\\x248 will parse as '\\' '\x248' and not '\' '\\'
      // 'x248'

      : QuotedEscapedStringGrammar::base_type(esc_str) {
    Util::DoubleQuoteEsc e;
    e.toKarma(esc_char);

    esc_str = kDoubleQuote << *(esc_char | karma::char_) << kDoubleQuote;
  }
  karma::rule<OutputIterator, std::string()> esc_str;
  karma::symbols<char, char const*> esc_char;
};

template <typename OutputIterator>
struct SingleQuotedStringGrammar : karma::grammar<OutputIterator, std::string()> {
  SingleQuotedStringGrammar() : SingleQuotedStringGrammar::base_type(esc_str) {
    Util::SingleQuoteEsc e;
    e.toKarma(esc_char);

    esc_str = "'" << *(esc_char | karma::char_) << "'";
  }
  karma::rule<OutputIterator, std::string()> esc_str;
  karma::symbols<char, char const*> esc_char;
};

namespace {

typedef std::back_insert_iterator<std::string> StringSink;
#if HG_PRINT_ESCAPE_SINGLE_QUOTE
SingleQuotedStringGrammar<StringSink> const kStringQuoteGrammar;
#else
QuotedEscapedStringGrammar<StringSink> const kStringQuoteGrammar;
#endif

typedef std::back_insert_iterator<Util::StringBuffer> StringBufferSink;
#if HG_PRINT_ESCAPE_SINGLE_QUOTE
SingleQuotedStringGrammar<StringBufferSink> const kStringBufferQuoteGrammar;
#else
QuotedEscapedStringGrammar<StringBufferSink> const kStringBufferQuoteGrammar;
#endif
}

inline void quoteAndEscapeString(std::string& out, std::string const& in) {
  karma::generate(StringSink(out), kStringQuoteGrammar, in);
}

inline void quoteAndEscapeString(Util::StringBuilder& out, std::string const& in) {
  karma::generate(StringBufferSink(out), kStringBufferQuoteGrammar, in);
}

void writeLabel(std::ostream& out, std::string const& label, SymbolQuotation quote) {
  if (quote == kQuoted) {
    std::string escaped;
    escaped.reserve(label.length() + 2);
    quoteAndEscapeString(escaped, label);
    out << escaped;
  } else
    out << label;
}
void writeLabel(Util::StringBuilder& out, std::string const& label, SymbolQuotation quote) {
  if (quote == kQuoted)
    quoteAndEscapeString(out, label);
  else
    out << label;
}

void writeLabel(std::ostream& out, Sym sym) {
  if (sym.isSpecial())
    out << Vocabulary::specialSymbols().str(sym);
  else
    out << sym;
}
void writeLabel(Util::StringBuilder& out, Sym sym) {
  if (sym.isSpecial())
    out << Vocabulary::specialSymbols().str(sym);
  else
    out << sym;
}


}}
