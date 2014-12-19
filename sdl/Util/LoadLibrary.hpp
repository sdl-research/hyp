



#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif



#ifdef _WIN32
typedef HMODULE LoadedLib;
#else
typedef void* LoadedLib;
#endif

LoadedLib loadLib(const char* name);

void unloadLib(LoadedLib aLib);

void* loadProc(LoadedLib aLib, const char* name);



#endif
