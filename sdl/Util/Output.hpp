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

    Output to stdout or filename as an ostream smartpointer (.gz extension =
    gzipped transparently).
*/

#ifndef SDL_LWUTIL_OUTPUT_HPP
#define SDL_LWUTIL_OUTPUT_HPP
#pragma once

#include <sdl/Util/Fileargs.hpp>

namespace sdl {
namespace Util {

/**
   Represents output, either to STDOUT (default, or filename '-'), or
   to a file if setFilename is called (or constructed with string
   argument). filename '-0' means no file (null pointer).

   an Output can be used as an OutputStream (so like shared_ptr<ostream>) that default
   constructs to stdin instead of no output

   both OutputStream and Output are configurable (via configure library or program_options directly)

*/
struct Output : OutputStream {

  static std::string help() { return "output file - " + graehl::file_arg_usage(); }
  static std::string helpId() { return "output file - " + kFileForIdHelp; }

  Output() : OutputStream("-") {}
  Output(std::string const& filename) : OutputStream(filename) {}
  Output(std::string const& filenamePrefix, std::string const& id)
      : OutputStream(fileForId(filenamePrefix, id)) {}

  operator bool() const { return (OutputStream const&)*this; }

  void setFilename(std::string const& filename) { OutputStream::set(filename); }

  std::ostream& getStream() const { return **this; }
};
}
}

namespace boost {
namespace program_options {
inline void validate(boost::any& v, std::vector<std::string> const& values, sdl::Util::Output* target_type,
                     int) {
  v = boost::any(sdl::Util::Output(graehl::get_single_arg(v, values)));
}


}}

#endif
