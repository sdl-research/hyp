#include <sdl/Hypergraph/UseOpenFst.hpp>

#if HAVE_OPENFST

#include <graehl/shared/warning_push.h>
GCC_DIAG_IGNORE(uninitialized)
#if HAVE_GCC_4_8
GCC_DIAG_IGNORE(unused-local-typedefs)
#endif

#include <lib/util.cc>
#include <lib/fst.cc>
#include <lib/flags.cc>
#include <lib/properties.cc>
#include <lib/symbol-table.cc>
#include <lib/compat.cc>
#include <graehl/shared/warning_pop.h>

#endif
