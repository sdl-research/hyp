






 */





#include <vector>


#include <boost/range/detail/safe_bool.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/functional/hash.hpp>
#include <boost/serialization/is_bitwise_serializable.hpp>
#include <boost/serialization/level.hpp>








VERBOSE_EXCEPTION_DECLARE(InvalidSymType)




























 */








enum SymbolType {

  kSpecialTerminal = 0,















































};






















        return "Special Terminal";

        return "Special Non-terminal";

        return "Variable";

        return "Persistent Non-terminal";



        return "Persistent Terminal";






        return "Invalid symbol type!";
    }
  }






















    }
  }



  }


  }

  /*

   *



   *

   *


   */



    return id;
  }





  }








    return id;
  }



  }

  static inline bool isTerminalType(SymbolType type) {


  }












  */


  }

  static inline bool isVariableType(SymbolType type) {





  }









   */
  inline bool isTerminal() const {



  }







   */
  inline bool isLexical() const {




  }

  /*

   *

   */



  }

  /*

   *


   */
  inline bool isVariable() const {

  }

  inline bool isPersistent() const {












  }

  inline bool isSpecial() const {




















  }























    return id_ == rhs.id_;
  }


    return id_ == rhs;
  }







  }


    return id_ < rhs.id_;
  }


    return id_ < rhs;
  }


    return id_ > rhs.id_;
  }


    return id_ > rhs;
  }


    return id_ <= rhs.id_;
  }


    return id_ <= rhs;
  }


    return id_ >= rhs.id_;
  }


    return id_ >= rhs;
  }









  {

    ++id_;
    return *this;
  }





  {




  }


  typedef safe_bool_t::unspecified_bool_type unspecified_bool_type;

  operator unspecified_bool_type() const
  {

  }

  /*

   */
  template <class Archive>

    ar & id_;

  }

  inline SymbolType type() const {




  }


    return id_;
  }


















};






















}






}






}















#else



#endif
}




}




}






{



  }
};
}





#endif
