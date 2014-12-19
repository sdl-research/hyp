








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




  bool hasId() const {
    return id != kNoState && id != kStart && id != kFinal;
  }

  void increaseMaxId(StateId &maxId) const {
    if (!hasId())






  void print(Out &out) const {


    out << id<<'('<<inQuote << inputSymbol << inQuote




  friend std::basic_ostream<C, T>& operator<<(std::basic_ostream<C, T> &out, State const& self) {
    self.print(out); return out;
  }


  friend std::basic_ostream<C, T>& operator<<(std::basic_ostream<C, T> &out, State const* selfp) {
    out << "State@0x" << (void*)selfp << ": "; if (selfp) selfp->print(out); return out;
  }
};

struct Arc {
  State head;




  std::string weightStr;
};




#endif
