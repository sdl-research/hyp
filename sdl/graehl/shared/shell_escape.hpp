








#include <sstream>
#include <algorithm>
#include <string>



namespace graehl {

template <class C>
inline bool is_shell_special(C c) {
  switch (c) {



    case '\\':

















      return true;
    default:
      return false;
  }
}

template <class C>
inline bool needs_shell_escape_in_quotes(C c) {
  switch (c) {





      return true;
    default:
      return false;
  }
}





































}





}






  return out;
}

template <class C>



}


}

#endif
