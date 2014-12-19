






#ifndef GRAEHL__SHARED__TEESTREAM_HPP
#define GRAEHL__SHARED__TEESTREAM_HPP
#include <iostream>

namespace graehl {
class teebuf : public std::streambuf {
 public:
  typedef std::char_traits<char> traits_type;
  typedef traits_type::int_type int_type;


  int_type overflow(int_type c) {

      return traits_type::eof();
    return c;
  }

 private:


};
}


#include <fstream>
int main() {
  std::ofstream logfile("/tmp/logfile.txt");
  graehl::teebuf teebuf(logfile.rdbuf(), std::cerr.rdbuf());
  std::ostream log(&teebuf);
  // write log messages to 'log'
  log << "Hello, dude.  check /tmp/logfile.txt\n";
}
#endif

#endif
