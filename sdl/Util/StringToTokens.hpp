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

    split line into tokens (word or char or whole-line) with
    original-unicode-index TokenSpan

    used by StringToStringModule string->hg

    TODO: use AlignedChars
*/

#ifndef STRINGTOTOKENS_JG20121015_HPP
#define STRINGTOTOKENS_JG20121015_HPP
#pragma once

#include <vector>
#include <set>
#include <algorithm>

#include <sdl/Span.hpp>
#include <sdl/Util/IcuUtil.hpp>
#include <sdl/Util/Utf8.hpp>
#include <sdl/Util/Set.hpp>
#include <sdl/Util/AcceptString.hpp>
#include <sdl/Util/AcceptStringImpls.hpp>
#include <sdl/Util/NormalizeUtf8.hpp>
#include <sdl/Util/AlignedChars.hpp>

namespace sdl {
namespace Util {

void utf8ToCharPos(Pchar i, Pchar end, IAcceptString const& accept, NormalizeUtf8 const& fix,
                   bool normalizeConsecutiveWhitespace = true);

void utf8ToWordSpan(Pchar i, Pchar end, IAcceptString const& accept, NormalizeUtf8 const& fix);

void utf8ToWholeSpan(Pchar i, Pchar end, IAcceptString const& accept, NormalizeUtf8 const& fix);

inline void utf8ToWholeSpan(std::string const& str, IAcceptString const& accept, NormalizeUtf8 const& fix) {
  utf8ToWholeSpan(arrayBegin(str), arrayEnd(str), accept, fix);
}

inline void utf8ToWordSpan(std::string const& str, IAcceptString const& accept, NormalizeUtf8 const& fix) {
  if (!str.empty()) utf8ToWordSpan(arrayBegin(str), arrayEnd(str), accept, fix);
}

inline void utf8ToCharPos(std::string const& str, IAcceptString const& accept, NormalizeUtf8 const& fix,
                          bool normalizeConsecutiveWhitespace) {
  if (!str.empty())
    utf8ToCharPos(arrayBegin(str), arrayEnd(str), accept, fix, normalizeConsecutiveWhitespace);
}

std::string const kSpaceUtf8(" ");

// TODO: move more impl to cpp

// could make versions without position, span (faster, but more near-duplicate code - ignore spans if you
// don't care)

/**
   call accept(Unicode, Position) for every char in utf8 [i, end) not deleted
   after transforming by 'fix', with pre-deletion positions. if
   normalizeConsecutiveWhitespace, replace 1 or more unicode whitespace by " ", and ignore initial and
  trailing whitespace

  //TODO: need to use IcuNormalizeByChunks if nfc/nfkc. for
  //words (splitOnWhitespace) we could take the spans then normalize the
  //resulting tokens, though

*/
template <class Accept>
void utf8ToCharPosImpl(Pchar i, Pchar end, Accept const& accept, FixUnicode const& fix,
                       bool normalizeConsecutiveWhitespace = true) {
  bool lastWasSpace = false;
  bool firstWord = true;
  Position spaceStart = 0;
  for (Position pos = 0; i < end; ++pos) {
    Unicode c = utf8::unchecked::next(i);
    if (i > end) break;
    if (fix(c)) {
      // c was not deleted
      if (!normalizeConsecutiveWhitespace) {
        acceptUnicode(accept, c, pos);
      } else {
        bool const space = unicodeSpace(c);
        if (space) {
          if (!lastWasSpace) spaceStart = pos;
        } else {
          if (!firstWord && lastWasSpace) accept(kSpaceUtf8, TokenSpan(spaceStart, pos));
          acceptUnicode(accept, c, pos);
          firstWord = false;
        }
        lastWasSpace = space;
      }
    }
  }
}


/**
   call accept(Slice, TokenSpan) for every word in utf8 [i, end) transformed by
   'fix', with pre-'fix'-deletion TokenSpan.
*/
template <class Accept>
inline void utf8ToWordSpanImplNoNfc(Pchar i, Pchar end, Accept const& accept, FixUnicode const& fix) {
  Pchar beginWord = i;
  Position beginPos = 0;
  bool lastWasSpace = true;  // prevent empty token if string starts with space
  bool wordModified = false;

  std::string buf;
  std::back_insert_iterator<std::string> bufAppend(buf);
  // optimization: only need to use buf if 'fix' deletes something

  if (i == end) return;

  for (Position pos = 0;; ++pos) {
    Pchar beforeC = i;
    Unicode ci = utf8::unchecked::next(i);  // advances i.
    if (i > end) SDL_THROW_LOG(Utf8, InvalidInputException, "invalid utf8 input");
    Unicode c = ci;
    bool const cremoved = !fix(c);
    bool const cmodified = c != ci;
    if (i == end) {
      bool const spacelike = cremoved || unicodeSpace(c);
      if (spacelike) {
        if (!lastWasSpace) {
          if (wordModified)
            accept(buf, TokenSpan(beginPos, pos));
          else
            accept(Slice(beginWord, beforeC), TokenSpan(beginPos, pos));
        }
      } else {
        if (lastWasSpace) {
          beginWord = beforeC;
          beginPos = pos;
        }
        if (!wordModified && !cmodified)
          accept(Slice(beginWord, i), TokenSpan(beginPos, pos + 1));
        else {
          if (!wordModified) buf.assign(beginWord, beforeC);
          utf8::append(c, bufAppend);
          accept(buf, TokenSpan(beginPos, pos + 1));
        }
      }
      break;
    }
    if (cremoved) {
      if (!lastWasSpace) {
        if (!wordModified) {
          buf.assign(beginWord, beforeC);
          wordModified = true;
        }
      }
    } else {
      // c was not deleted
      bool const space = unicodeSpace(c);
      if (space) {
        if (!lastWasSpace) {
          if (wordModified)
            accept(buf, TokenSpan(beginPos, pos));
          else
            accept(Slice(beginWord, beforeC), TokenSpan(beginPos, pos));
        }
      } else if (lastWasSpace) {
        beginPos = pos;
        if ((wordModified = cmodified)) {
          toUtf8v(c, buf);
        } else {
          beginWord = beforeC;
          wordModified = false;
        }
      } else if (wordModified)
        utf8::append(c, bufAppend);
      lastWasSpace = space;
    }
  }
}

/// like IAcceptSlice but since only used by template, slightly incomplete (providing what's needed). TODO:
/// move the nfc processing to whole string in a way that translates TokenSpan
template <class Accept>
struct AcceptMaybeNfc {
  Accept const& accept;
  NfcOptions nfcopt;
  AcceptMaybeNfc(Accept const& accept, NfcOptions const& nfcopt) : accept(accept) {}
  void operator()(Slice utf8, TokenSpan span) const {
    // TODO: avoid std::string copy by adding fns to Nfc.hpp/cpp (icu::StringPiece?)
    std::string out;
    accept(nfcopt.normalized(utf8, out), span);
  }
  void operator()(std::string const& utf8, TokenSpan span) const {
    std::string out;
    accept(nfcopt.normalized(utf8, out), span);
  }
};

// TODO: does string->string tok postag parse preorder make sketchy assumptions
// about constraints spans? i think so. so you can't safely nfc normalize for
// such pipelines when input has constraints. test+recommend a safer
// token+metadata-based approach or repair somehow the constraints when
// stringtostring goes string-output (or is this already correct)?
template <class Accept>
inline void utf8ToWordSpanImpl(Pchar i, Pchar end, Accept const& accept, NormalizeUtf8 const& fix) {
  NfcOptions const& nfcopt = fix;
  if (nfcopt.enabled())
    utf8ToWordSpanImplNoNfc(i, end, AcceptMaybeNfc<Accept>(accept, nfcopt), fix);
  else
    utf8ToWordSpanImplNoNfc(i, end, accept, fix);
}


/**
   call accept(std::string, TokenSpan) for the entire unsplit utf8 [i, end)
   transformed by 'fix', with pre-deletion TokenSpan.
*/
template <class Accept>
void utf8ToWholeSpanImpl(Pchar i, Pchar end, Accept const& accept, NormalizeUtf8 const& fix) {
  std::string str;
  str.resize(end - i);
  char* out = arrayBegin(str);
  char* begin = out;
  TokenSpan wholeSpan(0, utf8length(str));
  if (!fix.cleanupEnabled())
    accept(str, wholeSpan);
  else {
    for (Position pos = 0; i < end; ++pos) {
      Unicode c = utf8::unchecked::next(i);
      if (i > end) break;
      if (fix(c)) out = utf8::append(c, out);
      // else c was deleted
    }
    str.resize(out - begin);
    accept(str, wholeSpan);
  }
}

template <class Accept>
void utf8ToWholeSpanImpl(std::string const& str, Accept const& accept, NormalizeUtf8 const& fix) {
  utf8ToWholeSpanImpl(arrayBegin(str), arrayEnd(str), accept, fix);
}

template <class Accept>
void utf8ToWordSpanImpl(std::string const& str, Accept const& accept, NormalizeUtf8 const& fix) {
  if (!str.empty()) utf8ToWordSpanImpl(arrayBegin(str), arrayEnd(str), accept, fix);
}

template <class Accept>
void utf8ToCharPosImpl(std::string const& str, Accept const& accept, NormalizeUtf8 const& fix,
                       bool normalizeConsecutiveWhitespace) {
  if (!str.empty())
    utf8ToCharPosImpl(arrayBegin(str), arrayEnd(str), accept, fix, normalizeConsecutiveWhitespace);
}

typedef std::vector<std::string> Tokens;

/**
   split utf8 input strings into unicode chars or whitespace-separate tokens,
   keeping track of where in the input string each token comes from.
*/
struct StringToTokens : NormalizeUtf8 {
  bool splitOnWhitespace, normalizeWhitespace, unsplit;

  StringToTokens(bool enableAllFixUtf8 = true)
      : splitOnWhitespace(true), normalizeWhitespace(true), unsplit(false) {
    FixUnicode::initAssumingValidUtf8(!enableAllFixUtf8);
  }

  template <class Config>
  void configure(Config& config) {
    NormalizeUtf8::configure(config);
    config.is("string to tokens");
    config("unsplit", &unsplit)
        .defaulted()(
            "'unsplit: true' means take the whole line as a single string - WARNING: "
            "hypergraphs/vocabularies "
            "don't allow whitespace inside tokens");
    config("split-on-whitespace", &splitOnWhitespace)
        .defaulted()("Split on whitespace? False means split on any character boundary.");
    config("normalize-whitespace", &normalizeWhitespace)(
        "Force replacement of utf8 whitespace[s] by a single ascii space (always happens if "
        "'split-on-whitespace'). Does not affect 'unsplit: true'.)")
        .init(true);
    config("how a single input line turns into tokens (characters, or unicode whitespace split)");
  }

  template <class Accept>
  void acceptValidUtf8Impl(std::string const& fixed, Accept const& accept) const {
    if (splitOnWhitespace)
      utf8ToWordSpanImpl(fixed, accept, *this);
    else if (unsplit)
      utf8ToWholeSpanImpl(fixed, accept, *this);
    else
      utf8ToCharPosImpl(fixed, accept, *this, normalizeWhitespace);
    accept.finishPhrase(0);
  }


  template <class Accept>
  void acceptImpl(std::string line, Accept const& accept) const {
    NormalizeUtf8::normalizeNfc(line);
    acceptValidUtf8Impl(line, accept);
  }

  template <class Accept>
  void acceptImpl(std::string line, Accept const& accept, Constraints &c) const {
    if (constraintsIndexUnicodes(c)) {
      NormalizeUtf8::normalizeNfc(line, c); //TODO: track # of codepoints changed and update c
      acceptValidUtf8(line, accept);
    } else
      acceptImpl(std::move(line), accept);
  }

  void acceptValidUtf8(std::string const& fixed, IAcceptString const& accept) const {
    if (splitOnWhitespace)
      utf8ToWordSpan(fixed, accept, *this);
    else if (unsplit)
      utf8ToWholeSpan(fixed, accept, *this);
    else
      utf8ToCharPos(fixed, accept, *this, normalizeWhitespace);
    accept.finishPhrase(0);
  }

  /// used by StringToStringModule StringToHgHelper for string->hg. TODO: use AlignedChars for nfc norm
  void accept(std::string line, IAcceptString const& accept) const {
    acceptImpl(std::move(line), accept);
  }

  void accept(std::string line, IAcceptString const& accept, Constraints &c) const {
    NormalizeUtf8::normalizeNfc(line, c); //TODO: track # of codepoints changed and update c
    acceptValidUtf8(line, accept);
  }

  void split(std::string const& line, Tokens& tokens) const {
    // faster than using IAcceptString - this is a common operation
    acceptImpl(line, PushBackWithSpan(tokens));
  }

  void splitValidUtf8(std::string const& line, Tokens& tokens) const {
    acceptValidUtf8Impl(line, PushBackWithSpan(tokens));
  }

  void split(std::string const& line, Tokens& tokens, TokenSpans* pTokenSpans) const {
    acceptImpl(line, PushBackWithSpan(tokens, pTokenSpans));
  }

  void splitValidUtf8(std::string const& line, Tokens& tokens, TokenSpans* pTokenSpans) const {
    acceptValidUtf8Impl(line, PushBackWithSpan(tokens, pTokenSpans));
  }
};


}}

#endif
