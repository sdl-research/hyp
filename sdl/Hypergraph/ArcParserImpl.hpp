/**






*/





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


















BOOST_FUSION_ADAPT_STRUCT(ParserUtil::State,

                          (std::string, inputSymbol)
                          (std::string, outputSymbol)
                          (bool, isInputSymbolLexical)





BOOST_FUSION_ADAPT_STRUCT(ParserUtil::Arc,
                          (ParserUtil::State, head)
                          (std::vector<ParserUtil::State>, tails)
                          (std::string, weightStr))


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

  */






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

               )
              >> '(' ]

          | symbol)

           | symbol)
      >> ')'
      ;

  // state without input/output symbols
  state2 %= int_;

  // special case, state as in "START <- 0"
  state3 %=


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




#endif
