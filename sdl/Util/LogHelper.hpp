



*/





































































































#include <sstream>










#ifndef NDEBUG  // Debug mode: Enable trace & debug statements












#else  // NDEBUG // Release mode: Disable trace & debug statements






*/
#ifndef NTRACE_ONLY
// xmt should use this section


#else  // NTRACE_ONLY is set -- enable debug statements
// Only xmtserver should use this section




#endif

#endif  // NDEBUG















#define LOG_WARN_NAMESTR(loggerName, expression)                                                       \




#define LOG_ERROR_NAMESTR(loggerName, expression)                                                       \




#define LOG_FATAL_NAMESTR(loggerName, expression)                                          \















































#else  // NLOG is defined:

#include <string>






















// streams instead of %1% formatting


    std::stringstream UNLIKELYss;              \
    UNLIKELYss << expression;                  \

  } while (0)









  } while (0)












































































#endif
