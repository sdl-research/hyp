// Copyright 2014 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/**
   \file
   Parses an arc string into an arc wrapper ParserUtil::Arc,
   using boost::spirit. Should be included only from .cpp files, if
   possible, to reduce compile times.

   \author Markus Dreyer
*/

#ifndef HYP__HYPERGRAPH_ARCPARSER_IMPL_HPP
#define HYP__HYPERGRAPH_ARCPARSER_IMPL_HPP
#pragma once

#include <cstring>
#include <iostream>
#include <string>

#include <boost/spirit/include/qi.hpp>

#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

#include <sdl/Hypergraph/ParserUtil.hpp>

#include <sdl/Hypergraph/SymbolPrint.hpp>
#include <sdl/Util/QuoteEsc.hpp>
#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Util/Delete.hpp>

namespace ParserUtil = sdl::Hypergraph::ParserUtil;

/**
   present our structs as mpl vectors so that the list of ( ) captures in the
   grmamar below assign to id -> inputSymbol -> outputSymbol.

   (that requires we place this is in the top-level namespace)

   http://www.boost.org/doc/libs/1_56_0/libs/spirit/doc/html/spirit/qi/tutorials/employee___parsing_into_structs.html
*/
BOOST_FUSION_ADAPT_STRUCT(ParserUtil::State,
                          (sdl::Hypergraph::StateId, id)
                          (std::string, inputSymbol)
                          (std::string, outputSymbol)
                          (bool, isInputSymbolLexical)
                          (bool, isOutputSymbolLexical)
                          )

enum { kAtId = 0, kAtInputSymbol = 1, kAtOutputSymbol = 2, kAtInputLexical = 3, kAtOutputLexical = 4};

BOOST_FUSION_ADAPT_STRUCT(ParserUtil::Arc,
                          (ParserUtil::State, head)
                          (std::vector<ParserUtil::State>, tails)
                          (std::string, weightStr))

namespace sdl {
namespace Hypergraph {


namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

// adapted from:
// http://boost-spirit.com/home/articles/qi-example/parsing-escaped-string-input-using-spirit-qi
template <typename InputIterator>
struct DoubleQuotedEscapedString
    : qi::grammar<InputIterator, std::string()>
{
  DoubleQuotedEscapedString()
      : DoubleQuotedEscapedString::base_type(startRule),
        quote("\"")
  {
    Util::DoubleQuoteEsc e;
    e.toQi(escChar);

    startRule = quote
        >> *(escChar | "\\x" >> qi::hex | (qi::char_ - quote))
        >> quote
        ;
  }

  qi::rule<InputIterator, std::string()> startRule;
  qi::symbols<char const, char const> escChar;
  char const* quote;
};

template <typename InputIterator>
struct SingleQuotedString
    : qi::grammar<InputIterator, std::string()>
{
  SingleQuotedString()
      : SingleQuotedString::base_type(startRule),
        quote("'")
  {
    Util::SingleQuoteEsc e;
    e.toQi(escQuote);
    startRule = quote
        >> *(escQuote | (qi::char_ - quote))
        >> quote
        ;
  }

  qi::rule<InputIterator, std::string()> startRule;
  qi::symbols<char const, char const> escQuote;
  char const* quote;
};

template <typename Iterator>
struct ArcParserImpl : qi::grammar<Iterator, ParserUtil::Arc(), ascii::space_type> {

  ArcParserImpl();

  qi::rule<Iterator, std::string()> ntSymbol, lexSymbol, symbol, weight;
  qi::rule<Iterator, ParserUtil::State(), ascii::space_type> state1, state2, state3, state;
  qi::rule<Iterator, std::vector<ParserUtil::State>(), ascii::space_type> states;
  qi::rule<Iterator, ParserUtil::Arc(), ascii::space_type> start;
  DoubleQuotedEscapedString<Iterator> doubleQuotedEscapedStr;
  SingleQuotedString<Iterator> singleQuotedStr;

  /**
     Parses an arc string using the boost::spirit ArcParserImpl.
  */
  ParserUtil::Arc* parse(const std::string& str) const {
    Iterator iter = str.begin(), end = str.end();
    Util::AutoDelete<ParserUtil::Arc> arc(new ParserUtil::Arc());
    return (qi::phrase_parse(iter, end, *this, ascii::space, *arc) && iter == end) ?
        arc.release() :
        (ParserUtil::Arc*)0;
  }
};

template<class Iterator>
ArcParserImpl<Iterator>::ArcParserImpl()
    : ArcParserImpl<Iterator>::base_type(start) {

  using qi::char_;
  using qi::int_;
  using qi::_val;
  using qi::lexeme;

  // nonterminal symbol, e.g., S, NP, VP
  ntSymbol %=
      +(qi::print - char_("\\()\"' "))
      ;

  // lexical symbol, e.g., "foo"
  lexSymbol %=
      doubleQuotedEscapedStr
      | singleQuotedStr
      ;

  // the weight (as a string, to be parsed later)
  weight %= +char_;

  // input or output symbol for a state
  symbol %=
      lexSymbol | ntSymbol
      ;

  // state with optional ID, input symbol and optional output symbol
  state1 %=
      lexeme[ (int_
               | qi::eps [phoenix::at_c<kAtId>(_val) = -1]
               )
              >> '(' ]
      >> (lexSymbol [phoenix::at_c<kAtInputLexical>(_val) = true]
          | symbol)
      >> -(lexSymbol [phoenix::at_c<kAtOutputLexical>(_val) = true]
           | symbol)
      >> ')'
      ;

  // state without input/output symbols
  state2 %= int_;

  // special case, state as in "START <- 0"
  state3 %=
      qi::lit("START") [phoenix::at_c<kAtId>(_val) = -2]
      | qi::lit("FINAL") [phoenix::at_c<kAtId>(_val) = -3]
      ;

  state %=
      state1 | state2 | state3
      ;

  states %= +state;

  // complete line describing an arc
  start %=
      state
      >> "<-"
      >> states
      >> -("/" >> weight)
      ;
}


}}

#endif
