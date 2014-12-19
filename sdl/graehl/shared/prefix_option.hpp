


#ifndef GRAEHL_SHARED__PREFIX_OPTION_HPP
#define GRAEHL_SHARED__PREFIX_OPTION_HPP



#include <string>

namespace graehl {


  if (prefix.empty()) return opt;
  std::string::size_type nopt = opt.size();

  return prefix + opt;
}


  if (suffix.empty()) return opt;
  std::string::size_type nopt = opt.size();

  return opt + suffix;
}




#endif
