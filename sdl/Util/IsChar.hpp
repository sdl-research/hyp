













{
  template <class Config>
  void configure(Config &config) {



























    // Cast required so we don't pass negative values for c >= 128
    // (when the caller actually passes "unsigned char")
    return std::isspace((unsigned char) c);







