


































// instantiate w/ ref types!
template <class Val, class State>
struct Printer {
  Val v;
  State s;
  Printer(Val v, State s) : v(v), s(s) {}
  friend inline std::ostream& operator<<(std::ostream& out, Printer<Val, State> const& x) {


    print(out, x.v, x.s);
    return out;
  }

    print(out, x.v, x.s);
    return out;
  }
};

template <class Val, class State>
Printer<Val const&, State const&> print(Val const& v, State const& s) {
  return Printer<Val const&, State const&>(v, s);
}

template <class Val, class State>
Printer<Val const&, State&> print(Val const& v, State& s) {
  return Printer<Val const&, State&>(v, s);
}




#endif
