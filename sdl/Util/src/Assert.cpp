






namespace boost {

void assertion_failed(char const * expr, char const * function, char const * file, long line) {
  std::stringstream ss;
  ss << "Assertion failure:" << expr << ", in " << file << "::" << function << "at line number: " << line;

}

void assertion_failed_msg(char const * expr, char const* msg, char const * function, char const * file, long line) {
  std::stringstream ss;
  ss << msg << "- Assertion failure:" << expr << ", in " << file << "::" << function << "at line number: " << line;

}


}
