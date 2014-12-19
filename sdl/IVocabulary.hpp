




*/






#include <string>
#include <ostream>
#include <stdexcept>
#include <boost/scoped_ptr.hpp>






















/**



*/

  virtual ~IVocabularyVisitor() {}

};

struct IVocabulary : Resource {








  bool evictThread(Occupancy const&) OVERRIDE;





  virtual ~IVocabulary() {}












































































































  // variables are supposed to start from 0 index
  // this is a mapping from indices to variable id in vocabulary
  // TODO@SK: Should we check the presence of this variable in vocabulary?






































  }






















  void print(std::ostream& out) const;












 protected:



























  virtual unsigned doGetNumSymbols(SymbolType) const = 0;








};










































#endif
