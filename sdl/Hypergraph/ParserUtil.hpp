








#include <string>
#include <vector>




namespace Hypergraph {
namespace ParserUtil {

// State and Arc hold information about the actual states and arcs
// parsed. We are using these wrappers instead of the real Arcs
// because otherwise we'd have to template the parser on the arc.




struct State {

  State()



  {}





  {}







  std::string outputSymbol;

  bool isInputSymbolLexical;
  bool isOutputSymbolLexical;
































};

struct Arc {
  State head;




  std::string weightStr;
};




#endif
