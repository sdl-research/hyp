#include <sdl/Util/LoadLibrary.hpp>

namespace sdl { namespace Util {

# ifdef _WIN32

LoadedLib loadLib(const char* name) { return ::LoadLibrary(name); }
void unloadLib(LoadedLib aLib) { ::FreeLibrary(aLib); }
void* loadProc(LoadedLib aLib, const char* name) { return ::GetProcAddress(aLib, name); }

#else

LoadedLib loadLib(const char* name) { return ::dlopen(name, RTLD_LAZY); }
void unloadLib(LoadedLib aLib) { ::dlclose(aLib); }
void* loadProc(LoadedLib aLib, const char* name) { return ::dlsym(aLib, name); }

#endif

}}
