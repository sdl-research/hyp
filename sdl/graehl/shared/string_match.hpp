




#ifndef GRAEHL__SHARED__STRING_MATCH_HPP
#define GRAEHL__SHARED__STRING_MATCH_HPP


#include <graehl/shared/function_macro.hpp>
#include <graehl/shared/null_terminated.hpp>
#include <graehl/shared/string_to.hpp>
#include <graehl/shared/push_backer.hpp>

#include <string>
#include <iterator>
#include <stdexcept>



#include <graehl/shared/test.hpp>
#include <cctype>
#endif

namespace {  // anon
static std::string ascii_whitespace = "\n\r\t ";
}

namespace graehl {



















template <class S>
typename S::const_iterator chomp_end(S const& s) {
  if (s.empty()) return s.end();
  typename S::const_iterator b = s.begin(), i = s.end() - 1;

  if (*i == '\n') --i;
  if (*i == '\r') --i;

}


template <class S>
S chomp(S const& s) {
  if (s.empty()) return s;
  typename S::const_iterator b = s.begin(), i = s.end() - 1;

  if (*i == '\n') --i;
  if (*i == '\r') --i;

}

template <class S>

  typename S::size_type e;
  e = s.find_last_not_of(ws);
  //    if (e==S::npos) return S(); //npos is -1, so -1+1 = 0
  assert(e != S::npos || e + 1 == 0);
  return S(s.begin(), s.begin() + e + 1);
}

template <class S>

  //    std::string ws="\n\t ";
  typename S::size_type b;
  b = s.find_first_not_of(ws);

  return S(s.begin() + b, s.end());
}

template <class S>

  //    std::string ws="\n\t ";
  typename S::size_type b, e;
  b = s.find_first_not_of(ws);

  e = s.find_last_not_of(ws);
  assert(e != S::npos);
  return S(s.begin() + b, s.begin() + e + 1);
}

template <class I>

  std::stringstream s;
  char c;
  while (i.get(c)) {

    s.put(c);
  }
  throw std::runtime_error("EOF on input before string terminated");
}




  s.erase(0, n);
}



  s.erase(s.length() - n, n);
}

template <class Str>

  pos = in.find(oldsub, pos);

  in.replace(pos, oldsub.length(), newsub);
  return pos + newsub.length();
}

// returns true if one was replaced
template <class Str>

  return replace(in, oldsub, newsub, pos) != Str::npos;
}

// returns number of types we replaced
template <class Str>

  unsigned n = 0;

  return n;
}


template <class Str, class Sub>

  return str.find(sub, pos) != Str::npos;
}


// returns true and writes pos, n for substring between left-right brackets.  or false if brackets not found.




  typename Str::size_type rightpos;
  if (Str::npos == (pos = s.find(leftbracket, start_from))) return false;
  pos += leftbracket.length();
  if (Str::npos == (rightpos = s.find(rightbracket, pos))) return false;
  n = rightpos - pos;
  return true;
}

// first is first substring (left->right) between leftbracket and rightbracket in s.
// second is true if found, false if none found



  typedef std::pair<Str, bool> Ret;
  typename Str::size_type pos, n;
  if (substring_inside_pos_n(s, leftbracket, rightbracket, pos, n, start_from))
    return Ret(Str(s, pos, n), true);
  else
    return Ret(Str(), false);
}

// parse both streams as a sequence of ParseAs, comparing for equality


  /* could almost write as istream_iterator<ParseAs>, std::equal - except that
   doesn't check both iterators' end
  */
  ParseAs v1, v2;
  for (;;) {


    if (got1) {
      if (!got2) return false;  // 2 ended first
    } else {
      if (!got2) return true;  // both ended together
      return false;  // 1 ended first
    }
    if (!(v1 == v2)) return false;  // value mismatch
  }
  // unreachable!
  assert(0);
}



  std::basic_stringstream<Ch, Tr> i1(s1), i2(s2);
  return equal_streams_as_seq<ParseAs>(i1, i2);
}

// std::equal can only be called if sequences are same length!


  while (b1 != e1) {
    if (b2 == e2) return false;

  }
  // now b1 == e1
  return b2 == e2;
}



  return equal_safe(b1, e1, b2, e2, equal_typeless());
}





  while (bsub != esub) {


  }
  return true;
}



  return match_begin(bstr, estr, prefix.begin(), prefix.end());
}



  return match_begin(str.begin(), str.end(), prefix.begin(), prefix.end());
}



  while (bsub != esub) {


  }
  return true;
}



  return match_end(bstr, estr, suffix.begin(), suffix.end());
}



  return match_end(str.begin(), str.end(), suffix.begin(), suffix.end());
}



  return match_end(str, std::string(suffix));
}



  for (;;) {
    if (prefix == prefix_end) return true;
    if (str == str_end) return false;
    if (!equals(*prefix, *str)) return false;


  }
  // unreachable
  assert(0);
}



  return starts_with(str, str_end, prefix, prefix_end, equal_typeless());
}

/*
//FIXME: provide skip-first-whitespace or skip-no-whitespace iterators.
template <class Ch, class Tr, class CharIt> inline
bool expect_consuming(std::basic_istream<Ch, Tr> &i, CharIt begin, CharIt end)
{
    typedef std::istream_iterator<Ch> II;
    II ibeg(i), iend;
    return match_begin(ibeg, iend, begin, end);
}
*/




  if (begin == end) return true;
  Ch c;
  if (skip_first_ws)
    i >> c;
  else
    i.get(c);
  if (!i) return false;
  while (begin != end) {


  }
  return true;
  /* //NOTE: whitespace will be ignored!  so don't include space in expectation ...
      typedef std::istream_iterator<Ch> II;
      II ibeg(i), iend;
      return match_begin(ibeg, iend, begin, end);
  */
}



  return expect_consuming(i, str.begin(), str.end(), skip_first_ws);
}


template <class Str>

  return starts_with(str.begin(), str.end(), prefix.begin(), prefix.end());
}

template <class Str>

  //        return starts_with(str.rbegin(), str.rend(), suffix.rbegin(), suffix.rend());
  return match_end(str.begin(), str.end(), suffix.begin(), suffix.end());
}










template <class Str>

  return starts_with(str.begin(), str.end(), cstr_const_iterator(prefix), cstr_const_iterator());
  //    return starts_with(str, std::string(prefix));
}



  return starts_with(bstr, estr, prefix.begin(), prefix.end());
}

template <class Str>

  //  return match_end(str.begin(), str.end(), null_terminated_rbegin(suffix), null_terminated_rend(suffix));
  return ends_with(str, std::string(suffix));
}



  return match_end(bstr, estr, suffix.begin(), suffix.end());
}

// func(const Func::argument_type &val) - assumes val can be parsed from string tokenization (no whitespace)


  std::string s;
  bool last = false;
  while (!last && (in >> s)) {
    if (!term.empty() && ends_with(s, term)) {
      last = true;
      erase_end(s, term.length());
    }

    typename Func::argument_type val;

    func(val);
  }
}



  parse_until(term, in, make_push_backer(cont));
}



  typedef typename F::key_type Key;
  typedef typename F::data_type Data;
  using namespace std;
  typedef pair<Key, Data> Component;
  typedef string::const_iterator It;
  Component to_add;
  for (It i = s.begin(), e = s.end();;) {
    for (It key_beg = i;; ++i) {
      if (i == e) return;
      if (*i == key_val_sep) {  // [last, i) is key

        break;  // done key, expect val
      }
    }
    for (It val_beg = ++i;; ++i) {
      if (i == e || *i == pair_sep) {

        f(to_add);
        if (i == e) return;
        ++i;
        break;  // next key/val
      }
    }
  }
}















































































// NOTE: could use substring but that's more bug-prone ;D


  using namespace std;
  string s1("str1"), emptystr;
  BOOST_CHECK(starts_with(s1, emptystr));
  BOOST_CHECK(starts_with(emptystr, emptystr));
  BOOST_CHECK(ends_with(s1, emptystr));
  BOOST_CHECK(ends_with(emptystr, emptystr));
  BOOST_CHECK(!starts_with(s1, "str11"));
  BOOST_CHECK(!ends_with(s1, string("sstr1")));
  BOOST_CHECK(!ends_with(s1, "sstr1"));
  BOOST_CHECK(!starts_with(s1, string("str*")));
  BOOST_CHECK(!ends_with(s1, string("*tr1")));
  BOOST_CHECK(!ends_with(s1, string("str*")));
  BOOST_CHECK(!starts_with(s1, string("*tr1")));
  for (unsigned i = 0; i < 4; ++i) {
    string starts(TEST_starts_with[i]), ends(TEST_ends_with[i]);
    BOOST_CHECK(starts_with(s1, starts));
    BOOST_CHECK(starts_with(s1, starts.c_str()));
    BOOST_CHECK(ends_with(s1, ends));
    BOOST_CHECK(ends_with(s1, ends.c_str()));
    BOOST_CHECK(match_begin(s1.begin(), s1.end(), starts.begin(), starts.end()));
    BOOST_CHECK(match_end(s1.begin(), s1.end(), ends.begin(), ends.end()));
    if (i != 3) {
      BOOST_CHECK(!starts_with(s1, ends));
      BOOST_CHECK(!ends_with(s1, starts));
      BOOST_CHECK(!match_end(s1.begin(), s1.end(), starts.begin(), starts.end()));
      BOOST_CHECK(!match_begin(s1.begin(), s1.end(), ends.begin(), ends.end()));
    }
  }
  string s2(" s t  r1");
  BOOST_CHECK(equal_strings_as_seq<char>(s1, s2));
  BOOST_CHECK(!equal_strings_as_seq<string>(s1, s2));
  string s3(" s \nt  \tr1 ");
  BOOST_CHECK(equal_strings_as_seq<char>(s2, s3));
  BOOST_CHECK(equal_strings_as_seq<string>(s2, s3));
  string s4("str1a");
  BOOST_CHECK(!equal_strings_as_seq<string>(s1, s4));
  BOOST_CHECK(!equal_strings_as_seq<char>(s1, s4));
  BOOST_CHECK(!equal_strings_as_seq<char>(s4, s1));
  string s5("ab \t\n ");
  string s5t("ab");
  string s6("\t a \r\n");
  string s6c("\t a ");
  string s6t("a");
  string s6rt("\t a");
  string s6lt("a \r\n");
  EXPECT_EQ(s5t, trim(s5));
  EXPECT_EQ(s5t, trim(s5t));
  EXPECT_EQ(s5t, rtrim(s5));
  EXPECT_EQ(s5t, rtrim(s5t));
  EXPECT_EQ(s6t, trim(s6));
  EXPECT_EQ(s6t, trim(s6t));
  EXPECT_EQ(s6rt, rtrim(s6));
  EXPECT_EQ(s6rt, rtrim(s6rt));
  EXPECT_EQ(s6lt, ltrim(s6));
  EXPECT_EQ(s6lt, ltrim(s6lt));
  EXPECT_EQ(s6c, chomp(s6));
  EXPECT_EQ(s6c, chomp(s6c));
  EXPECT_EQ(s6rt, chomp(s6rt));
  EXPECT_EQ("", chomp(string("\n")));
}

#endif

}  // graehl

#endif
