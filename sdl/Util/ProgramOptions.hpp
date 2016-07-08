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

    boost program options without compiler warnings and clang-format friendly:

    add(g)("name", value(&x), "desc");
*/

#ifndef SDL_PROGRAMOPTIONS_JG_2013_04_22_HPP
#define SDL_PROGRAMOPTIONS_JG_2013_04_22_HPP
#pragma once

#include <sdl/LexicalCast.hpp>

#include <graehl/shared/warning_push.h>
GCC_DIAG_IGNORE(delete-non-virtual-dtor)
#include <boost/program_options.hpp>

#include <graehl/shared/warning_pop.h>

namespace sdl {

using boost::program_options::value;
using boost::program_options::options_description;
using boost::program_options::positional_options_description;
using boost::program_options::command_line_parser;
using boost::program_options::variables_map;
using boost::program_options::store;
using boost::program_options::notify;

typedef boost::program_options::options_description_easy_init AddOptionBase;

inline AddOptionBase add(options_description& desc) {
  return desc.add_options();
}

struct AddOption : AddOptionBase {
  explicit AddOption(options_description& desc) : AddOptionBase(desc.add_options()) {}
};


}

#endif
