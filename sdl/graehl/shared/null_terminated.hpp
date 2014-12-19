






   char const* s="ab";

   cstr_const_iterator(s), cstr_const_iterator()

   or

   null_terminated_begin(s), null_terminated_end(s)
*/





#include <boost/iterator/iterator_facade.hpp>
#include <graehl/shared/is_null.hpp>
#include <cstring>
#include <iterator>

namespace graehl {


template <class C>


 public:
  // end is default constructed
  null_terminated_iterator(C* p = 0) : p(p) {}



 private:
  C* p;

  friend class boost::iterator_core_access;



  }
  /* //note: you can't decrement from end==default constructed because it's 0, not address of null
       void decrement()
      {
        --p;
      }
  */
  C& dereference() const { return *p; }


};

typedef null_terminated_iterator<char> cstr_iterator;
typedef null_terminated_iterator<char const> const_cstr_iterator;
typedef null_terminated_iterator<char const> cstr_const_iterator;


  return s + std::strlen(s);
}

template <class C>



  return s;
}


/*
inline std::reverse_iterator<char const*> null_terminated_rbegin(char const* s)
{
  return std::reverse_iterator<char const*>(null_terminated_end(s));
}

inline std::reverse_iterator<char const*> null_terminated_rend(char const* s)
{
  return std::reverse_iterator<char const*>(s);
}

template <class C>
inline std::reverse_iterator<C const*> null_terminated_rbegin(C const* s)
{
  return null_terminated_end(s);
}

template <class C>
inline std::reverse_iterator<C const*> null_terminated_rend(C const* s)
{
  return s;
}
*/

// nonconst (copies of above)

  return s + std::strlen(s);
}

template <class C>



  return s;
}




#endif
