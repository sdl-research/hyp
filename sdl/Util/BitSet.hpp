
























#include <iterator>
#include <memory>





#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/size.hpp>












#define bitset_grow(n) (2 * (n))














template <class I, class A = std::allocator<I> >
struct SetBitsIter : std::iterator<std::forward_iterator_tag, bool> {

  B const* b;


  //  SetBitsIter(SetBitsIter const& o) : b(o.b), i(o.i) {}
  SetBitsIter() : b(), i(B::npos) {}
  explicit SetBitsIter(B const& b) : b(&b), i(b.find_first()) {}
  typedef SetBitsIter Self;
  inline void advance() {
    UTIL_DBG_MSG(10, "advance " << i << " = ");
    i = b->find_next(i);
    CUTIL_DBG_MSG(10, b->find_next(i) << "\n");
  }
  Self& operator++() {
    advance();
    return *this;
  }
  Self operator++(int) {
    SetBitsIter r = *this;
    advance();
    return r;
  }



  bool operator==(SetBitsIter const& o) const {
    assert(o.b == 0 || b == 0 || o.b == b);
    return i == o.i;
  }
  bool operator!=(SetBitsIter const& o) const {
    assert(o.b == 0 || b == 0 || o.b == b);
    return i != o.i;
  }
};

template <class I, class A>


}

template <class I, class A>


}








































// ADL for bitset means we need to be in boost ns

template <class I, class A>












}

template <class I, class A>


}

template <class I, class A>


}

template <class I, class A>


}

template <class I, class A>


  return c.count();
}

template <class I, class A>


};

template <class I, class A>


};










template <class I, class A, class K>


}







// if you use vector<bool> etc as bitset, watch for conflict with Contains.hpp
template <class I, class A>



}





  if (i >= n) {
    v.resize(std::max(bitset_grow(n), i + 1));
    return true;
  }
  return false;
}



  std::size_t n = v.size();



    v.resize(i + 1);
    return true;
  }
}

template <class B>
void addSetFromMap(B& v, std::size_t i) {
  v[i] = 1;
}

// see also setGrow - requires preallocation
// NOTE: for vector, add = push_back



}




  b.reset(i);
}




}



  reinit(b, end - i);
  setBits(b, i, end);
}



  setBits(b, c.begin(), c.end());
}





}



  copyBitsOut(b, adder(c));
}



  c.resize(b.count());

  copyBitsOut(b, c.begin());
}



  return v[i];
}


template <class Vec>

  v[i] = true;
}

template <class Vec>

  v[i] = bit;
}




  v.set(i, bit);
}




  v.set(i);
}



  v[i] = 0;
}

static const std::size_t npos = (std::size_t) - 1;




  return p == v.end() ? npos : p - v.begin();
}




  return p == v.end() ? npos : p - v.begin();
}




  return v.test(i);
}




  v.reset(i);
}




  return v.find_first();
}





  return v.find_next(i);
}

template <class Vec>
void setGrow(Vec& v, std::size_t i) {
  resizeFor(v, i);
  v.set(i);
}

// for vector or bitset
template <class Vec>
bool latchGrow(Vec& v, std::size_t i) {

  v.set(i);
  return true;
}

template <class Vec>





void resetSparse(Vec& v, std::size_t i) {

}

template <class Vec>





void setSparse(Vec& v, std::size_t i, bool bit) {
  if (bit)
    setGrow(v, i);
  else
    resetSparse(v, i);
}











#endif
