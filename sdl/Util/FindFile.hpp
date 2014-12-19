








#include <string>

#include <vector>






namespace Util {




/**







*/






















  SearchDirs() : pDirs(new Dirs()), doLogging() {}
  SearchDirs(SearchDirs const& o) : pDirs(o.pDirs), doLogging(o.doLogging) {}

  template <class Config>
  void configure(Config const& c) {




  void push(std::string const& searchDir) {









  void setDirs(Vec const& v) {




  void activateLogging(bool doLog = true) {  // Call this after logging is init.


















  shared_ptr<Dirs> pDirs;  // should allow for static pre-init detection
  bool doLogging;





// extern SearchDirs findFile;
inline SearchDirs& findFile() {
  static SearchDirs* d = new SearchDirs();  // TODO: leak
  return *d;
}




#endif
