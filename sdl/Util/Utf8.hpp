


















/*




  else: byte sequence iter
*/




#include <graehl/shared/os.hpp>


#include <vector>
#include <string>
#include <iterator>


#include <utf8/checked.h>




#define UTF8_CHECKED_NS utf8
#else
#define UTF8_CHECKED_NS utf8::unchecked
#endif



















































































































}























































































































































































































































#ifdef _MSC_VER

#pragma execution_character_set("utf-8")
#endif




inline std::size_t bytesForUtf(std::size_t nchar) {
  return MAX_UTF8_CHAR_BYTES * nchar;
}





































  typename Utf8Chars::iterator out = vec.begin();





































































































































}





}







}


// note: unchecked output iter





}



  return toUtf8(boost::begin(c), boost::end(c), o);
}

template <class I, class V>



}

template <class I, class V>
void toUtf8v(I i, I const& iend, V& v) {
  v.resize(bytesForUtf(std::distance(i, iend)));  // could be more efficient: back_inserter?

}

template <class I, class V>




}











  toUtf8v(boost::begin(c), boost::end(c), v);
}



  std::string r;
  toUtf8v(c, r);
  return r;
}


  std::string o(MAX_UTF8_CHAR_BYTES, char());
  o.resize(UTF8_CHECKED_NS::append(c, o.begin()) - o.begin());
  return o;
}


// note: unchecked output iter




  return o;
}

template <class I, class V>
void toUtf8sv(I i, I const& iend, V& v) {
  v.resize(bytesForUtf(std::distance(i, iend)));  // could be more efficient: back_inserter?


}

// v is a vector of strings


  v.resize(bytesForUtf(boost::distance(c)));
  toUtf8sv(boost::begin(c), boost::end(c), v);
}







}




























































  while (i != e) {

    *o = c;
    ++o;
  }
  return o;
}








  std::string::const_iterator i = s.begin();

}







}





















































































  }












}

// toUtf8Chs - vector<char> (utf8 enc) -> vector<string> (unicode chars in utf8)



}




  while (first != last) {


  }

}



  return lengthUtf8Chs(boost::begin(b), boost::end(b));
}

/**






 **/




  const char* s;
  std::size_t count = 0;
  std::size_t u;
  unsigned char b;

  /* Handle any initial misaligned bytes. */

    b = *s;

    /* Exit if we hit a zero byte. */


    /* Is this byte NOT the first byte of a character? */
    count += (b >> 7) & ((~b) >> 6);
  }

  /* Handle complete blocks. */
  for (;; s += sizeof(std::size_t)) {

    /* Prefetch 256 bytes ahead. */
    __builtin_prefetch(&s[256], 0, 0);


    /* Grab 4 or 8 bytes of UTF-8 data. */
    u = *(std::size_t*)(s);

    /* Exit the loop if there are any zero bytes. */


    /* Count bytes which are NOT the first byte of a character. */


  }

  /* Take care of any left-over bytes. */
  for (;; s++) {
    b = *s;

    /* Exit if we hit a zero byte. */


    /* Is this byte NOT the first byte of a character? */
    count += (b >> 7) & ((~b) >> 6);
  }

done:
  return ((s - _s) - count);
}

















































































}
















#endif
