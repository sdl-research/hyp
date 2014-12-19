










#include <boost/functional/hash.hpp>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/type_traits/is_pointer.hpp>









namespace Util {
























struct IndexVisitor {























};

/**








*/

         class Id = std::size_t,



         >
class Index {

 private:


 public:

  // types

  typedef Id IdType;
  typedef Hash Hasher;



  Index()





  }



      delete k;

    }



































    }
















    return found->second;
  }



  }

















      visitor.visit(it->first);
  }



























































  }

































 private:

























};

///

/**





   */



         >
class Registry {


 public:

  typedef Hash Hasher;


  Registry() {}

















  }





    delete k;
  }



  }



      visitor.visit(*it);
    }
  }















};




#endif
