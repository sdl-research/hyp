





#ifndef GRAEHL_SHARED__SPLIT_HPP
#define GRAEHL_SHARED__SPLIT_HPP


#include <string>
#include <vector>
#include <graehl/shared/string_to.hpp>
#include <graehl/shared/input_error.hpp>


#include <graehl/shared/test.hpp>
#include <cstring>
#endif


namespace graehl {


































template <class Cont>

  typedef typename Cont::value_type value_type;
  Cont* c;
  split_push_back(Cont& cr) : c(&cr) {}
  template <class Str>

    c->push_back(string_to<value_type>(s));
    return true;
  }
};

template <class Cont>

  Cont* c;
  split_string_push_back(Cont& cr) : c(&cr) {}
  template <class Str>

    c->push_back(s);
    return true;
  }
};


template <class Func>

    const std::string& csv,


    const std::string& delim = ",",

    bool leave_tail = true,  // if N reached and there's more string left, include it in final call to f
    bool must_complete = false  // throw if whole string isn't consumed (meaningless unless leave_tail==false)

  using namespace std;
  string::size_type delim_len = delim.length();
  if (delim_len == 0) {  // split by characters as special case

    for (; i < n; ++i)

    return i;
  }
  string::size_type pos = 0, nextpos;

  while (n < N && (!leave_tail || n + 1 < N) && (nextpos = csv.find(delim, pos)) != string::npos) {

    ++n;
    pos = nextpos + delim_len;
  }
  if (csv.length() != 0) {
    if (must_complete && n + 1 != N)
      throw_parse_error(csv, "Expected exactly " + to_string(N) + " " + delim + " separated fields", pos);

    ++n;
  }
  return n;
}

template <class Cont>

  return split_noquote(str, split_push_back<Cont>(c), delim);
}

template <class Cont>

  Cont c;
  split_noquote(str, split_string_push_back<Cont>(c), delim);
  return c;
}


  return split_string<std::vector<std::string> >(str, delim);
}



char const* split_strs[] = {"", ", a", "", 0};
char const* seps[] = {";", ";;", ",,", " ", "=,", ",=", 0};
char const* split_chrs[] = {";", ", ", "a", ";"};


  using namespace std;
  {
    std::string str = ";,a;";
    split_noquote(str, make_expect_visitor(split_chrs), "");
    split_noquote(str, make_expect_visitor(split_strs), ";");
    for (char const** p = seps; *p; ++p) {
      string s;
      char const* sep = *p;
      for (char const** q = split_strs; *q; ++q) {
        s.append(*q);

      }
      // split_noquote(s, make_expect_visitor(split_strs), seps);

    }
  }
}
#endif


}

#endif
