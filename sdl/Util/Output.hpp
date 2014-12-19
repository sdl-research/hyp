/** \file



*/











typedef graehl::file_arg<std::ostream> OutputStream;

/**









*/









  Output() : OutputStream("-") {}
  Output(std::string const& filename) : OutputStream(filename) {}






  void setFilename(const std::string& filename) {
    OutputStream::set(filename);
  }

  std::ostream& getStream() const {

  }

};












#endif
