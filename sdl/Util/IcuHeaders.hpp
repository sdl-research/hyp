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

    Include this to avoid warnings when including (even indirectly) ICU.

    Of course, you have to include this before anyone else includes ICU.
*/

#ifndef ICUHEADERS_JG_2013_04_22_HPP
#define ICUHEADERS_JG_2013_04_22_HPP
#pragma once

// all this is to avoid compile warnings about ICU headers:
#include <graehl/shared/warning_push.h>
#if HAVE_GCC_4_4
GCC_DIAG_IGNORE(unused-but-set-variable)
#endif
#if HAVE_GCC_4_6
GCC_DIAG_IGNORE(maybe-uninitialized)
#endif

#include <unicode/unistr.h>
#include <unicode/ucsdet.h>
#include <unicode/parseerr.h>
#include <unicode/putil.h>
#include <unicode/ucsdet.h>
#include <unicode/translit.h>
#include <unicode/errorcode.h>
#include <unicode/utext.h>
#include <unicode/locid.h>
#include <unicode/utypes.h>
#include <unicode/normalizer2.h>
#include <unicode/ustring.h>
#include <unicode/chariter.h>
#include <unicode/schriter.h>
#include <unicode/brkiter.h>
#include <unicode/normalizer2.h>
#include <unicode/umachine.h>

#include <graehl/shared/warning_pop.h>

#endif
