



















#include <string>







namespace Util {

/**

*/
std::string logFileName(std::string const& appname);

struct InitLoggerOptions {

  // whether or not the application uses multiple threads
  // if set to true, then logger will print out thread id information
  bool multiThread;


  std::string file;

  std::string patternLayout;

  InitLoggerOptions()














































































};


                InitLoggerOptions const& opts = InitLoggerOptions());



















#endif
