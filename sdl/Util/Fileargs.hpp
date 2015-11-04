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

 InputStream, OutputStream = shared_ptr to istream or ostream, plus filename
*/

#ifndef FILEARGS_GRAEHL_2015_11_03_HPP
#define FILEARGS_GRAEHL_2015_11_03_HPP
#pragma once

#include <string>
#include <graehl/shared/fileargs.hpp>
#include <graehl/shared/string_match.hpp>

namespace sdl {
namespace Util {

typedef graehl::file_arg<std::istream> InputStream;
typedef graehl::file_arg<std::ostream> OutputStream;

using graehl::stdin_arg;
using graehl::stdout_arg;
using graehl::special_input_filename;
using graehl::gz_filename;
using graehl::stdin_filename;
using graehl::stdout_filename;
using graehl::stderr_filename;
using graehl::null_filename;

std::string const kFileForIdHelp((graehl::file_arg_usage()+"; if empty, nothing; otherwise, for each segment id filename.gz -> filename.id.gz or filename -> filename.id"));

char const* const kStdoutFilename("-");
char const* const kStderrFilename("-2");
char const* const kNullFilename("-0");
std::string const kNullFilenameStr((kNullFilename));

std::string const kGzSuffix(".gz");
std::string const kPipePrefix("/proc/self/fd/");
std::string const kPipePrefix2("/dev/fd/");

inline bool isNullFilename(std::string const& path) {
  return path == kNullFilenameStr;
}

/// no need to check isNullFilename
inline bool isNullFile(InputStream const& in) {
  return !in;
}

inline bool isNullFile(OutputStream const& in) {
  return !in;
}

/**
   for per-id (per-request) output. for experimentation, not production. production should have the option of
   keeping all intermediate results in memory (with configurable debugging/recovery serialization)
*/
static std::string fileForId(std::string prefix, std::string const& id, std::string const& sep = "") {
  if (prefix.empty() || id.empty() || prefix == kStdoutFilename || prefix == kStderrFilename
      || prefix == kNullFilenameStr)
    return prefix;
  if (graehl::strip_suffix(prefix, kGzSuffix)) return prefix + sep + id + kGzSuffix;
  return prefix + sep + id;
}


}}

#endif
