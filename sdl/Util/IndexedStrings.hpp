/** \file

    like a map<T, size_t> and a vector<T> such that vector[map[x]] == x for all
    x that were added. (used by BasicVocabularyImpl with T = string, for example)
*/

#ifndef UTIL__INDEXED_STRINGS__JG_2014_03_04_HPP
#define UTIL__INDEXED_STRINGS__JG_2014_03_04_HPP
#pragma once

#include <graehl/shared/indexed.hpp>
#include <sdl/Sym.hpp>
#include <vector>
#include <string>
#include <sdl/Util/StableVector.hpp>

namespace sdl {
namespace Util {

typedef std::vector<std::string> UnstableStrings;

typedef StableStrings IndexedStringsValues;

using graehl::indexed;

typedef indexed<std::string, SymInt, IndexedStringsValues> IndexedStrings;

/// simple vector<string> so can't iterate safely while adding
typedef indexed<std::string, SymInt, UnstableStrings> UnstableIndexedStrings;


}}

#endif
