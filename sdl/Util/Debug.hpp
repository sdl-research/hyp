


























//TODO: optionally include severity numeric level in header.



















#include <iostream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cstdlib> // atoi
#include <iostream>


#define OSTR_DBG std::cerr

// you must DECLARE_DBG_CH(ch) first
# define IFDBGENV(ch, level, x) do {if (DBG_MAX_LEVEL>=level && ch##_DBG_LEVEL>=level) {x;}}while(0)

#ifndef DBG_MAX_LEVEL
# define DBG_MAX_LEVEL 999
// decrease this to lose even overhead of if-enabled check; sets max effective level of getenv_int("{Channelname}_DBG");
#endif

#ifndef DBG_HEADER_SHOW_LVL
// show (LVL) after ChannelName
# define DBG_HEADER_SHOW_LVL 1
#endif

// TODO: should be controllable by environment var, too
#ifndef DBG_WITH_LINEINFO
// show :FILE:LINE after that
# define DBG_WITH_LINEINFO 1
#endif

#ifndef DBG_START_NEWLINE
// start new message with newline just in case
# define DBG_START_NEWLINE 1
#endif


#define THROW_EXCEPTION(ch, x) do {                                 \
  std::stringstream ss123;                                         \

  throw std::runtime_error(ss123.str());                           \
  } while(0)













#else
# define DBG_LINE_INFO
#endif




//static local var means env var is checked once (like singleton)
#define DECLARE_DBG_CH(ch) DECLARE_DBG_LEVEL(ch)

#if DBGHEADER_SHOW_LVL
# define DBGVERBOSITY(n) <<"(" << n<<")"
#else
# define DBGVERBOSITY(n)
#endif

#if DBG_START_NEWLINE
# define DBG_INIT_NL "\n"
#else
# define DBG_INIT_NL
#endif

#define DBGHEADER(ch, n) DBG_INIT_NL #ch DBGVERBOSITY(n) DBG_LINE_INFO <<": "


#  define DBGX(ch, n, x) IFDBGENV(ch, n, x)
#  define DBGM(ch, n, x) IFDBGENV(ch, n, OSTR_DBG << DBGHEADER(ch, n) << x)
#  define DBGMC(ch, n, x) IFDBGENV(ch, n, OSTR_DBG << x)

#  define DBGX(ch, n, x)
#  define DBGM(ch, n, x)
#  define DBGMC(ch, n, x)
#endif


// for example (needs to be in parent namespace of use):

//NOTE: identity macro of same name as debug level is required. I don't know how to automate declaring such macros.
//avoids expense of evaluating conditional dbg msgs in release:

# define UTIL(x) x



#endif

#define UTIL_DBG_MSG(n, x) DBGM(UTIL, n, x)
//Continue on same line (no new header). you can also use for self-contained info msgs you don't want to jump to by source line in emacs
#define CUTIL_DBG_MSG(n, x) DBGMC(UTIL, n, x)
#define UTIL_DBG_EXEC(n, x) DBGX(UTIL, n, x)



