




#ifndef GRAEHL__SHARED__STREAM_UTIL_HPP
#define GRAEHL__SHARED__STREAM_UTIL_HPP






















#include <iomanip>
#include <iostream>
#include <cmath>

namespace graehl {











  std::ios_base::sync_with_stdio(false);
}



  unsync_stdio();
  std::cin.tie(0);
}



























template <class I, class O>

  o << i.rdbuf();
}

template <class I>

  i.seekg(0, std::ios::beg);
}


}  // graehl

#endif
