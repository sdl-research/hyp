





    struct T {
    typedef T self_type;
    template <class charT, class Traits>
    void read(std::basic_istream<charT, Traits>& in)
    {
    }
    template <class charT, class Traits>
    void print(std::basic_ostream<charT, Traits>& o) const
    {
    }

   /// or, even shorter:

    template <class I>
    void read(I& in)
    {}

    template <class O>
    void print(O& o) const
    {}
};
*/

#ifndef GRAEHL__SHARED__PRINT_READ_HPP
#define GRAEHL__SHARED__PRINT_READ_HPP





#define TO_OSTREAM_PRINT                                                                                  \






  typedef self_type has_print;

#define FROM_ISTREAM_READ                                                                                 \







#define TO_OSTREAM_PRINT_FREE(self_type)                                                           \







#define FROM_ISTREAM_READ_FREE(self_type)                                                          \







namespace graehl {
template <class Val, class State>
struct printer {
  Val v;
  State s;
  printer(Val v, State s) : v(v), s(s) {}
  friend inline std::ostream& operator<<(std::ostream& o, printer<Val, State> const& x) {
    // must be found by ADL - note: typedefs won't help
    print(o, x.v, x.s);
    return o;
  }
};

template <class Val, class State>
printer<Val const&, State const&> print(Val const& v, State const& s) {
  return printer<Val const&, State const&>(v, s);
}

template <class Val, class State>
printer<Val const&, State&> print(Val const& v, State& s) {
  return printer<Val const&, State&>(v, s);
}




#endif
