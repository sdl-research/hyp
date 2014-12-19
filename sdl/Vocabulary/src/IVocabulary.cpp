#include <limits>
#include <sstream>
#include <stdexcept>
#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>




















}




}




}

unsigned IVocabulary::getNumSymbols(SymbolType symType) const {


}

std::size_t IVocabulary::getSize() const {
  return doGetSize() + specialSymbols().getSize();
}




}




}

void IVocabulary::print(std::ostream& out) const {
  out << "{" << category() << " ";








  out << " #frozen=" << (size() - unk - ro);
  out << "}";
}

bool IVocabulary::evictThread(Occupancy const&) {









  else

  return changed;
}
