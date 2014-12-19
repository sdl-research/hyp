




#ifndef GRAEHL_SHARED__NORMALIZE_RANGE_HPP
#define GRAEHL_SHARED__NORMALIZE_RANGE_HPP



#include <boost/range/numeric.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <boost/range/value_type.hpp>

namespace graehl {

template <class R>


  return boost::accumulate(r, z);
}

template <class W>
struct normalize_options {
  W addk_num;
  W addk_denom;


  void addUnseen(W n) {
    addk_denom += n * addk_num;  // there are n tokens with 0 count; intend addk_num to apply to them also
  }







};

template <class W>
struct normalize_addk {
  W knum;
  W denom;
  normalize_addk(std::size_t N, W sum_unsmoothed, W addk_num = 0, W addk_denom = 0) : knum(addk_num) {
    denom = sum_unsmoothed + knum * N + addk_denom;
  }
  normalize_addk(std::size_t N, W sum_unsmoothed, normalize_options<W> const& n) : knum(n.addk_num) {
    denom = sum_unsmoothed + knum * N + n.addk_denom;
  }



};

template <class R, class O>





}

template <class R, class O>


  return normalize_copy(sum(r), r, boost::begin(r), n);
}

template <class R>



  normalize_copy(sum, r, boost::begin(r), n);
}

template <class R>


  normalize_sum(sum(r), r, n);
}




#endif
