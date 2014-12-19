



#include <string>






namespace Hypergraph {

template<class Iter>



struct ArcParser {

  typedef ArcParserImpl<std::string::const_iterator> Impl;

  ArcParser();





};



#endif
