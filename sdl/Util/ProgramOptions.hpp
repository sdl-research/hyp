/** \file

    boost program options without compiler warnings and clang-format friendly:

    add(g)("name", value(&x), "desc");
*/

#ifndef SDL_PROGRAMOPTIONS_JG_2013_04_22_HPP
#define SDL_PROGRAMOPTIONS_JG_2013_04_22_HPP
#pragma once

#include <sdl/LexicalCast.hpp>

#include <graehl/shared/warning_compiler.h>
#if HAVE_GCC_4_6
#include <graehl/shared/warning_push.h>
GCC_DIAG_IGNORE(delete-non-virtual-dtor)
#endif
#include <boost/program_options.hpp>
#if HAVE_GCC_4_6
#include <graehl/shared/warning_pop.h>
#endif

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
