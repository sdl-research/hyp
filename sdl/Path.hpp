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
#ifndef PATH_JG2012615_HPP
#define PATH_JG2012615_HPP
#pragma once

/** \file

    Configure for Path (boost::filesystem::path).
*/

#include <sdl/LexicalCast.hpp>
#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
# define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
#ifndef BOOST_FILESYSTEM_VERSION
# define BOOST_FILESYSTEM_VERSION 3
#endif
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <graehl/shared/leaf_configurable.hpp>

namespace boost {
namespace filesystem {

//for configure ADL:
inline std::string type_string(path &)
{
  return "filesystem path";
}
inline std::string to_string_impl(path const& val)
{
  return val.string();
}

inline void string_to_impl(std::string const& val, path& out) {
  out = path(val);
}

}
}

LEAF_CONFIGURABLE_EXTERNAL(boost::filesystem::path)

namespace sdl {

typedef boost::filesystem::path Path;

inline Path & extendPathFrom(Path const& base, Path &relative) {
  if (!relative.is_absolute())
    relative = base / relative;
  return relative;
}

/**
   returns base/relative.
*/
inline std::string extendedPath(Path const& base, Path relative) {
  return extendPathFrom(base, relative).string();
}

/**
   returns whether exists(file) prior to rm.
*/
inline bool removeFile(Path const& path) {
  return remove(path);
}

inline bool removeFile(std::string const& path) {
  return remove(Path(path));
}

}

#endif
