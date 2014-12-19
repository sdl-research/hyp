








#include <cctype>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>



namespace Util {

/**
























 */

 public:
  FormattedOstreamIterator() {}















      *os_ << line_prefix;
  }

  ~FormattedOstreamIterator();

  FormattedOstreamIterator& operator=(char c);

  FormattedOstreamIterator& operator*();
  FormattedOstreamIterator& operator++();
  FormattedOstreamIterator operator++(int);















  void insert_word();

  void write_word(const std::string& word);

  std::ostream* os_;
  std::string word_buffer_;

  std::string line_prefix_;

  unsigned max_line_length_;
  unsigned current_line_length_;



};

























#endif
