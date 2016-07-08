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

    Include this to avoid warnings when including (even indirectly) ICU.

    Of course, you have to include this before anyone else includes ICU.

    from http://userguide.icu-project.org/strings/utf-8:

LocalUTextPointer putext(utext_openUnicodeString(0, &ustr, &err));

or

UText utext = UTEXT_INITIALIZER; LocalUTextPointer close(utext_openUnicodeString(&utext, &ustr, &err));

/// return UChar32 and advance, else U_SENTINEL (-1) and do not advance the index:
#define UTEXT_NEXT32(ut)  \
    ((ut)->chunkOffset < (ut)->chunkLength && ((ut)->chunkContents)[(ut)->chunkOffset]<0xd800 ? \
    ((ut)->chunkContents)[((ut)->chunkOffset)++] : utext_next32(ut))

UText *utext_openUTF8(UText *ut, const char *s, int64_t length, UErrorCode *status)

UText *  utext_open[Const]UnicodeString(UText *ut, icu::UnicodeString *s, UErrorCode *status);

utext_close(UText*)

(ICU C API) UTF-8 as Default Charset

ICU has many functions that take or return char * strings that are assumed to be in the default charset which
should match the system encoding. Since this could be one of many charsets, and the charset can be different
for different processes on the same system, ICU uses its conversion framework for converting to and from
UTF-16.

If it is known that the default charset is always UTF-8 on the target platform, then you should #define
U_CHARSET_IS_UTF8 1 in or before unicode/utypes.h. (For example, modify the default value there or pass
-DU_CHARSET_IS_UTF8=1 as a compiler flag.) This will change most of the implementation code to use dedicated
(simpler, faster) UTF-8 code paths and avoid dependencies on the conversion framework. (Avoiding such
dependencies helps with statically linked libraries and may allow the use of UCONFIG_NO_LEGACY_CONVERSION or
even UCONFIG_NO_CONVERSION [see unicode/uconfig.h].)

Low-Level UTF-8 String Operations

unicode/utf8.h defines macros for UTF-8 with semantics parallel to the UTF-16 macros in unicode/utf16.h. The
macros handle many cases inline, but call internal functions for complicated parts of the UTF-8 encoding form.
For example, the following code snippet counts white space characters in a string:

#include "unicode/stringpiece.h"
#include "unicode/uchar.h"
#include "unicode/utf8.h"
#include "unicode/utypes.h"

int32_t countWhiteSpace(StringPiece const& sp) {
  const char *s=sp.data();
  int32_t length=sp.length();
  int32_t count;
  for(int32_t i=0; i<length;) {
    UChar32 c;
    U8_NEXT(s, i, length, c);
    if(u_isUWhiteSpace(c)) {
      ++count;
    }
  }
  return count;
}

Dedicated UTF-8 APIs

ICU has some APIs dedicated for UTF-8. They tend to have been added for "worker functions" like comparing
strings, to avoid the string conversion overhead, rather than for "builder functions" like factory methods and
attribute setters.

For example, icu::Collator::compareUTF8() compares two UTF-8 strings incrementally, without converting all of
the two strings to UTF-16 if there is an early base letter difference.

ucnv_convertEx() can convert between UTF-8 and another charset, if one of the two UConverters is a UTF-8
converter. The conversion from UTF-8 to most other charsets uses a dedicated, optimized code path, avoiding
the pivot through UTF-16. (Conversion from other charsets to UTF-8 could be optimized as well, but that has
not been implemented yet as of ICU 4.4.)

ucasemap_utf8ToLower(), ucasemap_utf8ToUpper(), ucasemap_utf8ToTitle(), ucasemap_utf8FoldCase()
ucnvsel_selectForUTF8()

UText can be used with BreakIterator APIs (character/word/sentence/... segmentation). utext_openUTF8() creates
a read-only UText for a UTF-8 string.

As a workaround for Thai word breaking not working in BreakIterator UTF8, you can convert the string to UTF-16
and convert indexes to UTF-8 string indexes via u_strToUTF8(dest=NULL, destCapacity=0, *destLength gets UTF-8
index).

ICU 4.4 has a technology preview for UText in the regular expression API, but some of the UText regex API and
semantics are likely to change for ICU 4.6. (Especially indexing semantics.)

uiter_setUTF8() creates a UCharIterator for a UTF-8 string.

*/

#ifndef ICUHEADERS_JG_2013_04_22_HPP
#define ICUHEADERS_JG_2013_04_22_HPP
#pragma once

#ifdef U_CHARSET_IS_UTF8
#if !U_CHARSET_IS_UTF8
#warning \
    "-DU_CHARSET_IS_UTF8=1 must not change - include Util/IcuHeaders.hpp first or set 'cc -DU_CHARSET_IS_UTF8=1' for all compilations using this header"
#endif
#undef U_CHARSET_IS_UTF8
#endif

#define U_CHARSET_IS_UTF8 1

// all this is to avoid compile warnings about ICU headers:
#include <graehl/shared/warning_push.h>
GCC_DIAG_IGNORE(unused-but-set-variable)
GCC_DIAG_IGNORE(maybe-uninitialized)

#include <unicode/brkiter.h>
#include <unicode/chariter.h>
#include <unicode/errorcode.h>
#include <unicode/locid.h>
#include <unicode/normalizer2.h>
#include <unicode/parseerr.h>
#include <unicode/putil.h>
#include <unicode/schriter.h>
#include <unicode/translit.h>
#include <unicode/ucsdet.h>
#include <unicode/ucsdet.h>
#include <unicode/umachine.h>
#include <unicode/unistr.h>
#include <unicode/ustring.h>
#include <unicode/utext.h>
#include <unicode/utypes.h>

#include <graehl/shared/warning_pop.h>

#endif
