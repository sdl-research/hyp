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
#include <algorithm>
#include <fstream>
#include <istream>
#include <iterator>
#include <stdexcept>
#include <string>

#include <boost/filesystem/operations.hpp>  // exists

#include <sdl/Util/Debug.hpp>
#include <sdl/Util/FindFile.hpp>
#include <sdl/Util/LogHelper.hpp>

#include <sdl/Util/DefaultPrintRange.hpp>

namespace sdl {
namespace Util {

namespace bfs = boost::filesystem;

std::string const& requireExists(std::string const& filename) {
  bfs::path filePath(filename);
  if (!exists(filePath))
    SDL_THROW_LOG(FindFile, ConfigException, "File '" << filename << "' does not exist.");
  return filename;
}

std::string findExistingFile(std::string const& filename, Dirs const& searchDirs, bool doLogging) {

  bfs::path filePath(filename);
  UTIL_DBG_MSG(1, "findExistingFile: " << filePath);

  if (filePath.is_absolute()) {
    UTIL_DBG_MSG(1, "absolute: " << filename);
    return requireExists(filename);
  }

  if (searchDirs.empty()) {
    // TODO: test
    Dirs searchCwd(1, bfs::current_path().string());
    UTIL_DBG_MSG(1, "empty search dirs, using cwd: " << searchCwd[0]);
    if (doLogging) {
      SDL_INFO(FindFile, "No base directory specified - looking for "
                             << filename << " relative to cwd: " << searchCwd[0]);
    }
    return findExistingFile(filename, searchCwd, doLogging);
  }

  // Try each search dir.
  for (std::string const& dirName : searchDirs) {
    // TODO: test
    bfs::path tryPath(dirName);
    UTIL_DBG_MSG(1, "try in dir: " << tryPath << " for file: " << filePath);
    tryPath /= filePath;
    UTIL_DBG_MSG(1, "trying: " << tryPath);
    if (exists(tryPath)) {
      if (doLogging) {
        SDL_INFO(FindFile, "Found '" << tryPath.string() << "'");
      }
      return tryPath.string();
    }
  }

  SDL_THROW_LOG(FindFile, ConfigException,
                "Could not find file '"
                    << filename << "' "
                    << printer(searchDirs, RangeSep(":", "(search directories: ", ")")));
}

void splitFilePath(std::string const& resolvedPath, std::string* pStrDir, std::string* pStrFileName) {
  // TODO: test
  bfs::path filePath(resolvedPath);
  if (pStrDir) *pStrDir = filePath.parent_path().string();
  if (pStrFileName) *pStrFileName = filePath.filename().string();
}

std::string absPath(std::string const& pathName) {
  bfs::path path(pathName);
  return system_complete(path).string();
}

std::string dirname(std::string const& pathName) {
  return bfs::path(pathName).parent_path().string();
}

std::string basename(std::string const& pathName) {
  return bfs::path(pathName).filename().string();
}

void mkdirs(std::string const& path) {
  // TODO: test
  boost::filesystem::create_directories(path);
}

std::string mkdirParentPath(std::string const& pathName) {
  bfs::path p((pathName));
  bfs::path parent(p.parent_path());
  create_directories(parent);
  return parent.string();
}

std::string absParent(std::string const& pathName) {
  // TODO: test
  bfs::path path(pathName);
  return system_complete(path).parent_path().string();
}

void SearchDirs::appendParent(std::string const& fileInParent) {
  // TODO: test
  std::string const& parent = absParent(fileInParent);
  UTIL_DBG_MSG(1, "SearchDirs: appending dirname(" << fileInParent << ") = " << parent);
  append(parent);
}

// SearchDirs findFile;


}}
