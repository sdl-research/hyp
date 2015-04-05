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
#include <sdl/Util/Icu.hpp>
#include <graehl/shared/from_strings.hpp>
#include <boost/range/iterator_range.hpp>
#include <sdl/IntTypes.hpp>

namespace sdl {
namespace Util {

bool neededIcuNormalize(icu::UnicodeString const& s, icu::UnicodeString &out, IcuNormalizer2Ptr norm) {
  UErrorCode err = U_ZERO_ERROR;
  bool const normed = norm->isNormalized(s, err);
  assert(U_SUCCESS(err));
  if (normed) return false;
  norm->normalize(s, out, err);
  assert(U_SUCCESS(err));
  return true;
}

bool maybeIcuNormalize(icu::UnicodeString const& s, icu::UnicodeString &out, IcuNormalizer2Ptr norm) {
  using namespace icu;
  UErrorCode err = U_ZERO_ERROR;
  int nfcPrefixLen = norm->spanQuickCheckYes(s, err);
  if (!U_SUCCESS(err))
    SDL_THROW_LOG(Icu, InvalidInputException, "ensure valid unicode before maybeIcuNormalize");
  assert(nfcPrefixLen >= 0);
  if (nfcPrefixLen == s.length()) {
    return false;
  } else {
    assert(nfcPrefixLen < s.length());
    out.setTo(s, 0, nfcPrefixLen);
    norm->normalizeSecondAndAppend(out, s.tempSubString(nfcPrefixLen), err);
    if (!U_SUCCESS(err))
      SDL_THROW_LOG(Icu, InvalidInputException, "ensure valid unicode before maybeIcuNormalize");
    return true;
  }
}

bool maybeIcuNormalize(icu::StringPiece utf8, std::string &out, IcuNormalizer2Ptr normalize) {
  icu::UnicodeString u(icu::UnicodeString::fromUTF8(utf8)), buf;
  if (maybeIcuNormalize(u, buf, normalize)) {
    fromIcu(buf, out);
    return true;
  } else
    return false;
}


namespace {
template <class Iter>
Iter findEndNull(Iter p) {
  while (*p) ++p;
  return p;
}
template <class Iter>
std::string showArrayNullTerminated(Iter begin, const char* sep = ", ") {
  return graehl::range_to_string(boost::make_iterator_range(begin, findEndNull(begin)), sep);
}
}

std::string icuLanguagesList(const char* sep) {
  return showArrayNullTerminated(Locale::getISOLanguages(), sep);
}

std::string icuCountriesList(const char* sep) {
  return showArrayNullTerminated(Locale::getISOCountries(), sep);
}


std::string parseErrorString(UParseError const& e) {
  if (!(e.preContext[0] || e.postContext[0])) return "";
  graehl::string_builder append;
  append("ICU error");
  if (e.line > 0) {
    append(" on line #");
    append(e.line);
  }
  if (e.offset > 0) {
    append(e.line > 0 ? " col #" : " on char #");
    append(e.offset);
  }
  append(" ... ");
  append(fromIcu(e.preContext));
  append(" [ERROR HERE]: ");
  append(fromIcu(e.postContext));
  append(" ...");
  return append.str();
}


UParseError& IcuErrorCode::storeParseError() {
  UParseError e;
  e.line = e.offset = -1;
  e.preContext[0] = e.postContext[0] = 0;
  optParse.reset(e);
  return *optParse;
}

std::string IcuErrorCode::parseError() const {
  if (!optParse) return "";
  std::string r = parseErrorString(*optParse);
  return r.empty() ? r : " parsing: " + r;
}

/*
  u_setDataDirectory() is not thread-safe. Call it before calling ICU APIs from multiple threads. If you use
  both u_setDataDirectory() and u_init(), then use u_setDataDirectory() first.
*/
void globalIcuData(std::string dirname) {
#ifdef _MSC_VER
  std::replace(dirname.begin(), dirname.end(), '/', '\\');  // ICU requires \ sep
#endif
  using namespace icu;
  u_setDataDirectory(dirname.c_str());
}

std::string platformPathString(std::vector<std::string> const& dirs) {
  char const* sep =
#ifdef _MSC_VER
      ";"
#else
      ":"
#endif
      ;
  return graehl::to_string_sep(dirs, sep);
}

unsigned toIcuReplacing(UnicodeString& ustr, icu::StringPiece utf8) {
  using namespace icu;
  int32 bytes = utf8.size();
  UChar* out = ustr.getBuffer(bytes);
  UErrorCode status = U_ZERO_ERROR;
  int32 actualSize, replaced;
  u_strFromUTF8WithSub(out, bytes, &actualSize, utf8.data(), bytes, 0xFFFD, &replaced, &status);
  if (!U_SUCCESS(status)) {
    ustr.releaseBuffer(0);
    return kIcuErrorInReplacing;
  } else {
    ustr.releaseBuffer(actualSize);
    return replaced;
  }
}


}}
