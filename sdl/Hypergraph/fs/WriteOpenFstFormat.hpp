



#include <ostream>



namespace Hypergraph {

namespace WriteOpenFstFormatHelper {

template <class Arc>
struct ArcWriter {


  void operator()(Arc const* arc) const {



  }



};


template <class Arc>

  shared_ptr<IHypergraph<Arc> const> hp = ensureProperties(hg, kFsm | kStoreOutArcs);

  typedef WriteOpenFstFormatHelper::ArcWriter<Arc> Writer;







  return out;
}




#endif
