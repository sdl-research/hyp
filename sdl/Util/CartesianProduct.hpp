









#include <vector>
#include <iostream>
#include <iterator>


namespace Util {







{
  if (curr == end) {
    result->push_back(tmpVec);
    return;
  }



    tmpVec.push_back(*it);
    cartesianProduct(curr + 1, end, tmpVec, result);
    tmpVec.pop_back();
  }
}


/**

*/




  cartesianProduct(input.begin(), input.end(), outputTemp, output);
}





