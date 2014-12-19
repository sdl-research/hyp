





#ifndef GRAEHL__SHARED__COMMAND_LINE_HPP
#define GRAEHL__SHARED__COMMAND_LINE_HPP


#include <graehl/shared/word_spacer.hpp>
#include <graehl/shared/shell_escape.hpp>
#include <sstream>
#include <vector>


#include <graehl/shared/test.hpp>
#include <cstring>
#endif

namespace graehl {

template <class O, class Argv>
O& print_command_line(O& out, int argc, Argv const& argv, const char* header = "### COMMAND LINE:\n") {

  graehl::word_spacer_c<' '> sep;
  for (int i = 0; i < argc; ++i) {
    out << sep;
    out_shell_quote(out, argv[i]);
  }

  return out;
}


template <class Argv>
inline std::string get_command_line(int argc, Argv const& argv, const char* header = "COMMAND LINE:\n") {
  std::ostringstream os;
  print_command_line(os, argc, argv, header);
  return os.str();
}


  typedef char const* arg_t;
  typedef arg_t* argv_t;

  std::vector<char const*> argvptrs;



    //    return std::isspace(c);
    return c == ' ' || c == '\n' || c == '\t';
  }




  }

  // note: str is from stringbuf.

    argvptrs.clear();
    argvptrs.push_back(progname);
    str(cmdline + " ");  // we'll need space for terminating final arg.
    char* i = gptr(), * end = egptr();

    char* o = i;
    char terminator;
  next_arg:
    while (i != end && isspace(*i)) ++i;  // [ ]*
    if (i == end) return;

    if (*i == '"' || *i == '\'') {
      terminator = *i;
      ++i;
      argvptrs.push_back(o);
      while (i != end) {
        if (*i == '\\') {
          ++i;
          if (i == end) throw_escape_eof();
        } else if (*i == terminator) {
          *o++ = 0;
          ++i;
          goto next_arg;
        }
        *o++ = *i++;
      }
    } else {
      argvptrs.push_back(o);
      while (i != end) {
        if (*i == '\\') {
          ++i;
          if (i == end) throw_escape_eof();
        } else if (isspace(*i)) {
          *o++ = 0;
          ++i;
          goto next_arg;
        }
        *o++ = *i++;
      }
    }
    *o++ = 0;
  }
  argc_argv() : argvptrs() {}

};









char const* test_strs[] = {"ARGV", "ba", "a", "b c", "d", " e f ", "123", 0};


  using namespace std;
  {
    string opts = "ba a \"b c\" 'd' ' e f ' 123";
    argc_argv args(opts);
    BOOST_CHECK_EQUAL(args.argc(), 7);

      CHECK_EQUAL_STRING(test_strs[i], args.argv()[i]);
    }
  }
  {
    string opts = " ba a \"\\b c\" 'd' ' e f '123 ";
    argc_argv args(opts);
    BOOST_CHECK_EQUAL(args.argc(), 7);

      CHECK_EQUAL_STRING(test_strs[i], args.argv()[i]);
    }
  }
  {
    argc_argv args("");
    BOOST_CHECK_EQUAL(args.argc(), 1);
  }
}
#endif

}  // graehl

#endif
