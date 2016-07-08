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

    mkstemp but world readable by default.
*/

#ifndef TEMPORARYFILE_JG_2013_08_12_HPP
#define TEMPORARYFILE_JG_2013_08_12_HPP
#pragma once

#include <sdl/Util/Errno.hpp>
#include <graehl/shared/os.hpp>

namespace sdl {
namespace Util {

enum { kCreateTemporary = false, kUnlinkTemporary = true };

inline std::string temporaryFile(std::string const& tmpXXX = "/tmp/tmp.xmt.XXXXXXXX",
                                 bool worldReadable = true, bool unlinkFile = false) {
  return graehl::safe_tmpnam(tmpXXX, !unlinkFile, worldReadable);
}


}}

#endif
