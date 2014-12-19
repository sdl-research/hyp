// Copyright 2014 SDL plc
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
