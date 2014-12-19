











#include <boost/spirit/include/karma.hpp>

#ifndef HG_PRINT_ESCAPE_SINGLE_QUOTE
#define HG_PRINT_ESCAPE_SINGLE_QUOTE 0
// FIXME: 1 is similar to HG_PRINT_ESCAPE_NON_ASCII 0 - in that roundtrip doesn't work
#endif




namespace karma = boost::spirit::karma;



template <typename OutputIterator>


      // xEscapeNonAscii true = prints all ascii chars - utf8 becomes \x214 etc








    e.toKarma(esc_char);


  }
  karma::rule<OutputIterator, std::string()> esc_str;
  karma::symbols<char, char const*> esc_char;
};

template <typename OutputIterator>



    e.toKarma(esc_char);


  }
  karma::rule<OutputIterator, std::string()> esc_str;
  karma::symbols<char, char const*> esc_char;
};











#if HG_PRINT_ESCAPE_SINGLE_QUOTE

#else

#endif








}



    std::string escaped;


    out << escaped;


}





}






}





}



