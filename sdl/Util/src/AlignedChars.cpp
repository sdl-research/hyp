/** \file

    D117 Canonical Composition Algorithm: Starting from the second character in the coded character sequence
   (of a Canonical Decomposition or Compatibility Decomposition) and proceeding sequentially to the final
   character, perform the following steps:

    R1 Seek back (left) in the coded character sequence from the character C to find the last Starter L
   preceding C in the character sequence.

    R2 If there is such an L, and C is not blocked from L, and there exists a Primary Composite P which is
   canonically equivalent to the sequence <L, C>, then replace L by P in the sequence and delete C from the
   sequence..

*/

#include <sdl/Util/AlignedChars.hpp>
#include <sdl/Util/IcuHeaders.hpp>
#include <sdl/Util/Icu.hpp>

namespace sdl {
namespace Util {


unsigned const kICharsBufSize = 240;
void alignedNormalize(IChars& in, IcuNormalizer2Ptr normalizer, ITakeAlignedChars& out) {
  icu::UnicodeString u(kICharsBufSize, 0, 0);
  Unicode c;
  while (!IChars::done((c = in.next()))) u += (UChar32)c;
  alignedNormalizeUnicodeString(u, normalizer, out);
}

void alignedNormalizeUnicodeString(icu::UnicodeString const& u, IcuNormalizer2Ptr normalizer,
                                   ITakeAlignedChars& out) {
  Position start = 0;
  int32 len = u.length(), pos;
  UErrorCode err = U_ZERO_ERROR;
  int nfcPrefixLen = normalizer->spanQuickCheckYes(u, err);
  assert(U_SUCCESS(err));
  assert(len >= 0 && nfcPrefixLen >= 0);
  TokenSpan span;
  span.first = 0;
  icu::StringCharacterIterator it(u);
  while ((pos = it.getIndex()) < nfcPrefixLen) {
    assert(it.hasNext());
    Unicode c = it.next32PostInc();
    span.second = span.first + 1;
    out.takeWithSpan(c, span);
    ++span.first;
  }
  icu::UnicodeString remainder(u.tempSubString(nfcPrefixLen)), normalized;
  CharsFromUnicodeStringImpl chars(remainder); //TODO: docs say normalizeSecondAndAppend
  IcuNormalizeByChunks<CharsFromUnicodeStringImpl> norm(chars, normalizer);
  norm.takeAllWithSpan(out);
}

void alignedNormalizeStringPiece(icu::StringPiece utf8, IcuNormalizer2Ptr normalizer, ITakeAlignedChars& out) {
  Position start = 0;
  icu::UnicodeString u;
  if (toIcuReplacing(u, utf8)) SDL_THROW_LOG(AlignedChars, InvalidInputException, "invalid unicode");
  alignedNormalizeUnicodeString(u, normalizer, out);
}


}}
