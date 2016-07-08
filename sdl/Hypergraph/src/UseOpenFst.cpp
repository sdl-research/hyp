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
#include <sdl/Hypergraph/UseOpenFst.hpp>

#if HAVE_OPENFST

#include <graehl/shared/warning_push.h>

GCC6_DIAG_IGNORE(misleading-indentation)

GCC_DIAG_IGNORE(uninitialized)
GCC_DIAG_IGNORE(unused-local-typedefs)

#include <lib/compat.cc>
#include <lib/flags.cc>
#include <lib/fst.cc>
#include <lib/properties.cc>
#include <lib/symbol-table.cc>
#include <lib/util.cc>

#include <graehl/shared/warning_pop.h>

#endif
