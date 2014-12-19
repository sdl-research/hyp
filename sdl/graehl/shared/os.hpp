









#include <fstream>
#include <string>
#include <cstdio>
#include <stdexcept>
#include <stdlib.h>
#include <cstring>
#include <sstream>



#if (defined(_WIN32) || defined(__WIN32__) || defined(WIN32)) && !defined(__CYGWIN__)
#define OS_WINDOWS


#else




#include <unistd.h>

#endif

#if !defined(MEMMAP_IO_WINDOWS) && !defined(MEMMAP_IO_POSIX)
#if defined(OS_WINDOWS) || defined(__CYGWIN__)
#define MEMMAP_IO_WINDOWS
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#else
#define MEMMAP_IO_POSIX
#endif
#endif

#ifdef MEMMAP_IO_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <windows.h>

#undef min
#undef max
#undef DELETE

#endif

#ifdef OS_WINDOWS
#include <direct.h>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#endif

// static local var means env var is checked once (like singleton)





#define DECLARE_ENV_C(n, f, v) DECLARE_ENV(f, v) static const int n = f();




















static char getenv_buf[getenv_maxch];
inline char* getenv(char const* key) {

  return GetEnvironmentVariableA(key, getenv_buf, getenv_maxch) ? getenv_buf : NULL;
}





namespace graehl {
typedef DWORD Error;
inline long get_process_id() {
  return GetCurrentProcessId();
}
}  // ns

#else

#include <unistd.h>
namespace graehl {
typedef int Error;
inline long get_process_id() {
  return getpid();
}
}  // ns
#include <errno.h>
#include <string.h>
#endif

namespace graehl {

inline int getenv_int(char const* key) {
  char const* s = getenv(key);
  return s ? atoi_nows(s) : 0;
}

inline char* getenv_str(char const* key) {
  return getenv(key);
}

















  int ret = ::system(cmd.c_str());


}

inline std::string get_current_dir() {
#ifdef OS_WINDOWS
  char* malloced = _getcwd(NULL, 0);
#else
  char* malloced = ::getcwd(NULL, 0);
#endif

  std::string ret(malloced);
  free(malloced);
  return ret;
}

template <class O>


  o << get_current_dir();

}

inline Error last_error() {
#ifdef MEMMAP_IO_WINDOWS
  return ::GetLastError();
#else
  return errno;
#endif
}

inline std::string error_string(Error err) {
#ifdef MEMMAP_IO_WINDOWS
  LPVOID lpMsgBuf;


    throw std::runtime_error("couldn't generate Windows error message string");
  std::string ret((char*)lpMsgBuf);
  ::LocalFree(lpMsgBuf);
  return ret;
#else
  return strerror(err);
#endif
}

inline std::string last_error_string() {
  return error_string(last_error());
}

inline bool create_file(const std::string& path, std::size_t size) {
#ifdef _WIN32





  return ::CloseHandle(fh);
#else
  return ::truncate(path.c_str(), size) != -1;
#endif
}

inline bool remove_file(const std::string& filename) {
  return 0 == remove(filename.c_str());
}


  std::string filename;
  std::fstream file;
  bool exists;

    choose_name();
    open();
    file << c;
    // DBP(c);
    reopen();
  }

    choose_name();
    open(mode);
  }








    filename = std::tmpnam(NULL);


  }
  void open(std::ios::openmode mode = std::ios::in | std::ios::out | std::ios::trunc) {
    file.open(filename.c_str(), mode);

  }

    file.flush();
    file.seekg(0);
  }




    close();
    remove();
  }
};


  throw std::runtime_error(module + ": " + last_error_string());
}

#define TMPNAM_SUFFIX "XXXXXX"
#define TMPNAM_SUFFIX_LEN 6





}





  const unsigned MY_MAX_PATH = 1024;
  char tmp[MY_MAX_PATH + 1];
  std::strncpy(tmp, filename_template.c_str(), MY_MAX_PATH - TMPNAM_SUFFIX_LEN);











  int fd = ::mkstemp(tmp);





  ::close(fd);




  return tmp;
}




}










    return false;

}

//!< returns dir/name unless dir is empty (just name, then). if name begins with / then just returns name.


  if (!name.empty() && name[0] == pathsep)  // absolute name
    return name;
  if (basedir.empty())  // relative base dir
    return name;
  if (basedir[basedir.length() - 1] != pathsep)  // base dir doesn't already end in pathsep
    return basedir + pathsep + name;
  return basedir + name;
}

// FIXME: test


  using namespace std;
  string::size_type p = fullpath.rfind(pathsep);
  if (p == string::npos) {
    dir = ".";
    file = fullpath;
  } else {
    dir = fullpath.substr(0, p);
    file = fullpath.substr(p + 1, fullpath.length() - (p + 1));
  }
}










































































}  // graehl

#endif
