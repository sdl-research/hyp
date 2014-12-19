#include <sdl/Util/StringToTokens.hpp>

namespace sdl { namespace Util {

void utf8ToCharPos(Pchar i, Pchar end, IAcceptString const& accept, FixUnicode const& fix, bool normalizeConsecutiveWhitespace)
{
  utf8ToCharPosImpl(i, end, accept, fix, normalizeConsecutiveWhitespace);
}

void utf8ToWordSpan(Pchar i, Pchar end, IAcceptString const& accept, FixUnicode const& fix)
{
  utf8ToWordSpanImpl(i, end, accept, fix);
}

void utf8ToWholeSpan(Pchar i, Pchar end, IAcceptString const& accept, FixUnicode const& fix)
{
  utf8ToWholeSpanImpl(i, end, accept, fix);
}

}}
