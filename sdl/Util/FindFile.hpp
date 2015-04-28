// Copyright 2014-2015 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/** \file

    search path for regular files (not only executables)
*/

#ifndef SDL_UTIL_FINDFILE_HPP
#define SDL_UTIL_FINDFILE_HPP
#pragma once

#include <string>
#include <deque>
#include <vector>
#include <functional>
#include <sdl/SharedPtr.hpp>
#include <sdl/Util/Add.hpp>
#include <sdl/Util/DefaultPrintRange.hpp>

namespace sdl {
namespace Util {

std::string const& requireExists(
    std::string const& path);  // throw exception unless file exists. otherwise identity

/**
   Tries to find existing file or directory; if absolute, identity. else
   try relative to all the paths in searchDirs. special case: empty
   searchDirs allows paths relative to cwd. Throws exception if not found.

   \param doLogging Write log when file found in one of the dirs?
   Only optional because we use this for finding the logging config
   itself, before logging is initalized.
*/

typedef std::vector<std::string> Dirs;

std::string findExistingFile(std::string const& filename, Dirs const& searchDirs, bool doLogging = true);

void splitFilePath(const std::string& resolvedPath, std::string* pStrDir, std::string* pStrFileName);

std::string absPath(std::string const& path);

std::string absParent(std::string const& path);  // pStrDir part of splitFilePath(absPath(path)

/// \return (possibly relative) path.parent_path
std::string dirname(std::string const& path);

std::string basename(std::string const& path);

/// POST: path.parent_path exists and is a dir (else exception). \return path.parent_path
std::string mkdirParentPath(std::string const& path);

/// POST: path is a dir (else exception)
void mkdirs(std::string const& path);

// perhaps options objects could use virtual inheritance so there's only one instance of this
struct SearchDirs : std::unary_function<std::string, std::string> {
  SearchDirs() : pDirs(new Dirs()), doLogging() {}
  SearchDirs(SearchDirs const& o) : pDirs(o.pDirs), doLogging(o.doLogging) {}

  template <class Config>
  void configure(Config const& c) {
    c("Search path");
    c("search-dir", pDirs.get())("directories searched for relative pathnames after . - first match wins");
  }

  void push(std::string const& searchDir) {
    addFront(*pDirs, searchDir);  // TODO: deque?
  }

  void pop() { popFront(*pDirs); }

  void append(std::string const& searchDir) { pDirs->push_back(searchDir); }
  void appendParent(std::string const& fileInParent);

  template <class Vec>
  void setDirs(Vec const& v) {
    pDirs->clear();
    Util::append(*pDirs, v);
  }

  void activateLogging(bool doLog = true) {  // Call this after logging is init.
    doLogging = doLog;
  }

  std::string operator()(std::string const& pathname) const {
    if (!pDirs) return requireExists(pathname);
    return findExistingFile(pathname, *pDirs, doLogging);
  }

  template <class O>
  void print(O& o) const {
    o << Util::print(*pDirs, RangeSep(" ", "(search directories: ", " )"));
  }
  template <class C, class T>
  friend std::basic_ostream<C, T>& operator<<(std::basic_ostream<C, T>& o, SearchDirs const& self) {
    self.print(o);
    return o;
  }

  shared_ptr<Dirs> pDirs;  // should allow for static pre-init detection
  bool doLogging;
};

/// singleton. note: use before static initialization won't have any search
// paths but should be safe. could convert to thread-safe pointer-singleton
// later
// extern SearchDirs findFile;
inline SearchDirs& findFile() {
  static SearchDirs* d = new SearchDirs();  // TODO: leak
  return *d;
}


}}

#endif
