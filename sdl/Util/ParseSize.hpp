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

    parse and print nonnegative numbers followed by optional k, m, g, or t (10^3,10^6,10^9,10^12) suffix, or K, M, G, or T (2^10,2^20,2^30,2^40).

    e.g. 1.5G

    someone might be confused, thinking 1m means 1 milli. but these are not for SI units
*/

#ifndef PARSESIZE_LW2012413_HPP
#define PARSESIZE_LW2012413_HPP
#pragma once


#include <graehl/shared/size_mega.hpp>

namespace sdl { namespace Util {

typedef graehl::size_t_bytes SizeBytes; // can boost::program_options::validate into this.
typedef graehl::size_metric SizeMetric; // for (double) counts of things (powers of 10)

template <class Size, class InputStream>
inline Size parseSize(InputStream &in) {
  return graehl::parse_size<Size>(in);
}

template <class InputStream>
inline std::size_t parseSizeBytes(InputStream &in)
{
  return parseSizeBytes<std::size_t>(in);
}

/// maxWidth, if positive, limits total number of characters. decimalThousand selects the 10^(3k) SI suffixes (k m g t) instead of 2^(10k) (K G M T)
template <class Size, class OutputStream>
inline OutputStream & printSize(OutputStream &out, Size size, bool decimalThousand = true, int maxWidth=-1)
{
  graehl::print_size(out, size, decimalThousand, maxWidth);
}


}}

#endif
