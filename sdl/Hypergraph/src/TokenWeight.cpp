#include <sdl/IVocabulary.hpp>
#include <sdl/Hypergraph/TokenWeight.hpp>
#include <sdl/Util/PrintRange.hpp>

namespace sdl {
namespace Hypergraph {

void Token::append(Token const& other) {
  syms_.append(other.syms());
  if (other.properties() != Token::kUnspecified) {
    setExtendableRight(other.isExtendableRight());
    setMustExtendRight(other.mustExtendRight());
  }
  if (other.getEndState() != kNoState)
    setEndState(other.getEndState());
}

void Token::print(std::ostream& out, IVocabulary const* voc) const {
  if (start() == kNoState)
    out << "?";
  else
    out << start();
  out << "-";
  if (getEndState() == kNoState)
    out << "?";
  else
    out << getEndState();
  out << ",";
  if (!syms_.empty()) {
    sdl::print(out, syms_, voc);
    out << ",";
  }
  out << "props:" << this->properties();
}


}}
