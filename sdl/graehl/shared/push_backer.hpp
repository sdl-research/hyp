




#ifndef GRAEHL__SHARED__PUSH_BACKER_HPP
#define GRAEHL__SHARED__PUSH_BACKER_HPP


namespace graehl {

template <class Cont>

  Cont* cont;
  typedef void result_type;
  typedef typename Cont::value_type argument_type;
  typedef push_backer<Cont> self_type;
  push_backer(self_type const& o) : cont(o.cont) {}
  push_backer(Cont& container) : cont(&container) {}
  template <class V>

    cont->push_back(v);
  }

};



  return push_backer<Cont>(container);
}

template <class Output_It>

  Output_It o;
  typedef void result_type;
  //    typedef typename std::iterator_traits<Output_It>::value_type  argument_type;
  typedef outputter<Output_It> self_type;
  outputter(self_type const& o) : o(o.o) {}
  outputter(Output_It const& o) : o(o) {}
  template <class V>

    *o++ = v;
  }
};



  return outputter<typename Cont::iterator>(container.begin());
}


}

#endif
