/**




*/





#include <string>

#include <unicode/chariter.h>
#include <unicode/schriter.h>
#include <unicode/unistr.h>









namespace Util {

typedef FromUnicodes FromString32;

/**

*/























































#else



#endif



















































}





















  */
  bool findNextNonSpace();



























































































/**




*/
template<class GenericString = icu::UnicodeString>

 public:







  void next();

  GenericString const& value() const;
  GenericString const& value(TokenSpan&) const;

 private:



};

////////////////////////////






inline void appendChar(UChar32 ch, String32* pStr) {


}

inline void appendChar(UChar32 ch, String16* pStr) {
  icu::UnicodeString icuStr(ch);


}

inline void appendChar(UChar32 ch, icu::UnicodeString* pStr) {
  pStr->append(ch);
}

template<class GenericString>



  next();
}

template <class GenericString>


}



}

template<class GenericString>
void Tokenizer<GenericString>::next() {



    return;
  }

  if (!findNextNonSpace()) {

    return;
  }







  }

}

template<class GenericString>
GenericString const& Tokenizer<GenericString>::value() const {

}

template<class GenericString>
GenericString const& Tokenizer<GenericString>::value(TokenSpan& tokSpan) const {


}




#endif
