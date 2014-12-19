






#ifndef GRAEHL_SHARED__RANDOM_HPP
#define GRAEHL_SHARED__RANDOM_HPP




#endif






#include <cmath>  // also needed for boost/random :( (pow)
#include <algorithm>  // min for boost/random



















#include <cstdlib>
#endif

#include <boost/scoped_array.hpp>


#include <ctime>
#include <graehl/shared/os.hpp>


#include <graehl/shared/test.hpp>
#include <cctype>
#endif

namespace graehl {







  return (n + 1) * 2654435769U;
}


// long pid=get_process_id();

  return boost::random_device().operator()();
#else


#endif
}



























#else

#endif
#endif



  srand(value);
#else

#endif
}




inline double random01()  // returns uniform random number on [0..1)
{

  return ((double)std::rand()) * (1. / ((double)RAND_MAX + 1.));
#else
  return g_random01();
#endif
}






struct set_random_pos_fraction {
  template <class C>
  void operator()(C& c) {
    c = random_pos_fraction();
  }
};












  }





  using namespace std;


    unsigned ran_lt_i = random_less_than(i);
    BOOST_CHECK(0 <= ran_lt_i && ran_lt_i < i);

    char r_alphanum = random_alphanum();

  }
}
#endif


}

#endif
