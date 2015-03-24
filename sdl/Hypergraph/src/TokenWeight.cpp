#include <sdl/IVocabulary.hpp>
#include <sdl/Hypergraph/TokenWeight.hpp>
#include <sdl/Util/PrintRange.hpp>

namespace sdl {
namespace Hypergraph {

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
  if (props_ == kUnspecified)
    out << '*';
  else {
    //    if (props_ & kBlockLeft) out << '{';
    if (props_ & kMustExtendLeft) out << '<';
    if (props_ & kExtendableLeft) out << '[';
    if (props_ & kExtendableRight) out << ']';
    if (props_ & kMustExtendRight) out << '>';
    if (props_ & kBlockRight) out << '}';
  }
}


}}
