





#ifndef GRAEHL_SHARED__POOL_TRAITS
#define GRAEHL_SHARED__POOL_TRAITS


#include <boost/shared_ptr.hpp>
#include <graehl/shared/intrusive_refcount.hpp>

namespace graehl {

// e.g. T=boost::object_pool<O, A> >

template <class T>

  typedef T impl;  // only while this lives (noncopyable), its pointers are valid
  typedef typename T::element_type element_type;
  typedef typename T::user_allocator user_allocator;
  typedef typename user_allocator::size_type size_type;
  typedef typename user_allocator::difference_type difference_type;



  enum { has_is_from = false };  // is_from(p)
  typedef element_type* pointer_type;  // this may be overriden e.g. refcounted ptr
};

template <class T>

  enum { has_is_from = true };  // is_from(p)
};

template <class T>
struct pool_destroyer;

template <class T>

  typedef T pool_type;
  T& t;
  explicit pool_destroyer(T& t) : t(t) {}
  typedef typename pool_traits<T>::pointer_type pointer_type;

    t.destroy(p);

  }
};




// U has only static malloc, free, size_type


  typedef T element_type;
  typedef U user_allocator;
  typedef element_type* pointer_type;
  static inline T* malloc() { return (T*)U::malloc(sizeof(T)); }
  static inline void free(T const* p) { U::free((char*)p); }
  static inline bool is_from(T const* p) { return true; }  // warning: this is not like a normal pool
  enum { can_leak = true };
  enum { has_is_from = false };
  static inline void destroy(T const* p) {
    p->~T();
    free(p);
  }
#include <graehl/shared/pool_construct.ipp>
};

template <class T, class U>

  typedef untracked_pool<T, U> pool_type;
  typedef typename T::pointer_type pointer_type;

};

template <class T, class U>

  enum { has_is_from = false };
};




// e.g. struct my : graehl::intrusive_refcount
template <class T>
// R=unsigned if single-threaded!

  typedef typename intrusive_traits<T>::user_allocator user_allocator;
  typedef T element_type;
  typedef boost::intrusive_ptr<T> pointer_type;
  static inline pointer_type malloc() { return (T*)user_allocator::malloc(sizeof(T)); }
  static inline void free(pointer_type const& p) {}  // noop!



  enum { can_leak = true };
  enum { has_is_from = false };
  static inline void destroy(pointer_type p) {}  // noop!


#include <graehl/shared/pool_construct.ipp>
};

// default traits are good

template <class T>

  typedef untracked_pool<T> pool_type;
  typedef typename T::pointer_type pointer_type;

};




#endif
