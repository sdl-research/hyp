// Copyright 2014-2015 SDL plc
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

    include openfst headers if enabled, suppressing compiler warnings.
*/

#ifndef HYP__HYPERGRAPH_USEOPENFST_HPP
#define HYP__HYPERGRAPH_USEOPENFST_HPP
#pragma once

#if HAVE_OPENFST

#include <sdl/Util/IcuHeaders.hpp> // we need this included before openfst includes it.

#include <graehl/shared/warning_push.h>

GCC_DIAG_IGNORE(uninitialized)

#if HAVE_GCC_4_8
GCC_DIAG_IGNORE(maybe-uninitialized)
GCC_DIAG_IGNORE(unused-local-typedefs)
GCC_DIAG_IGNORE(unused-but-set-variable)
#endif

CLANG_DIAG_IGNORE(mismatched-tags)

#include <fst/compat.h>
#include <fst/fst.h>

// ToReplaceFst
#include <fst/replace.h>
#include <fst/symbol-table.h>

#include <graehl/shared/warning_pop.h>

#endif // HAVE_OPENFST

#endif
