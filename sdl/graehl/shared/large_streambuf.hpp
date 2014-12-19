





#ifndef GRAEHL_SHARED__LARGE_STREAMBUF_HPP
#define GRAEHL_SHARED__LARGE_STREAMBUF_HPP


#include <boost/config.hpp>
#include <boost/noncopyable.hpp>
#include <fstream>
#include <new>

namespace graehl {



template <std::size_t bufsize = 256 * 1024>

  BOOST_STATIC_CONSTANT(std::size_t, size = bufsize);
  char buf[bufsize];
  large_streambuf() {}
  template <class S>

    attach_to_stream(s);
  }
  template <class S>


  }




};













  std::size_t size;
  void* buf;




  template <class S>



  }
  template <class S>



      s.rdbuf()->pubsetbuf((char*)buf, size);
    }









};


}

#endif
