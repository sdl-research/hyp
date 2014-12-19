/** \file

    include openfst headers if enabled, suppressing compiler warnings.
*/

#ifndef HYP__HYPERGRAPH_USEOPENFST_HPP
#define HYP__HYPERGRAPH_USEOPENFST_HPP
#pragma once

#if HAVE_OPENFST

// prevents warnings from fst including icu

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
