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

    characters spans or sub-word chunks that would be affected by CamelCase
    etc. (unicode aware)
*/

#ifndef ICUFIRSTWORDBREAKITERATOR_JG2012103_HPP
#define ICUFIRSTWORDBREAKITERATOR_JG2012103_HPP
#pragma once


#include <sdl/Util/IcuHeaders.hpp>
#include <sdl/SharedPtr.hpp>
#include <boost/noncopyable.hpp>
#include <unicode/brkiter.h>
#include <unicode/rbbi.h>
#include <unicode/utext.h>

namespace sdl {
namespace Util {

using icu::BreakIterator;
/**
   break the token into two words only; the first word is as a regular word
   break iterator and the second is whatever remains.

   only used in UnicodeString::toTitle, which promises:

   "This function uses only the setText(), first() and next() methods of the
   provided break iterator."

   actually, it also uses current(). exceptions will be thrown for any other
   methods since they're not tested yet.
*/
typedef icu::RuleBasedBreakIterator BreakIteratorBase;

// U_COMMON_API
struct IcuFirstWordBreakIterator : BreakIteratorBase, boost::noncopyable {
  typedef icu::CharacterIterator CharacterIterator;
  int32_t boundaryIndex;
  int32_t cursor, end;

  Locale const& locale;
  unique_ptr<BreakIterator> p;
  explicit IcuFirstWordBreakIterator(Locale const& locale, BreakIterator* pWordIterator = 0)
      : boundaryIndex(), cursor(), end(), locale(locale) {
    IcuErrorCode status;
    p.reset(pWordIterator ? pWordIterator : BreakIterator::createWordInstance(locale, status));
  }
  virtual void setText(UText* text, UErrorCode& status) {
    using namespace icu;
    cursor = 0;
    p->setText(text, status);
    end = (int32_t)utext_nativeLength(text);
  }
  virtual void setText(UnicodeString const& text) {
    cursor = 0;
    p->setText(text);
    end = text.length();
  }
  virtual int32_t first() { return (cursor = 0); }
  virtual int32_t next(void) {
    if (boundaryIndex == 0) {
      cursor = p->next();
      ++boundaryIndex;
    } else if (boundaryIndex == 1) {
      cursor = end;
      ++boundaryIndex;
    } else
      cursor = BreakIterator::DONE;
    return cursor;
  }

#define SDL_ICUCASE_UNIMPLEMENTED                                  \
  SDL_THROW_LOG(IcuFirstWordBreakIterator, UnimplementedException, \
                "UnicodeString::titleCase promised not to call methods other than setText, first, next.");

  virtual int32_t next(int32_t offset) {
    if (offset < 0) SDL_ICUCASE_UNIMPLEMENTED;
    while (offset--) next();
    return cursor;
  }
  virtual int32_t current() const { return cursor; }

  // because ICU is automatically generated from Java, it's tricky to subclass - objects aren't created by the
  // public via constructor, but are held by pointer, like in Java.
  static inline UClassID getStaticClassID() { return NULL; }
  virtual UClassID getDynamicClassID() const { return NULL; }

  // the rest of the methods are pure virtual in BreakIterator but are promised not to be called;
  virtual int32_t hashCode() const {
    SDL_ICUCASE_UNIMPLEMENTED;
    return 0;
  }

  UBool operator==(BreakIterator const& o) const {
    SDL_ICUCASE_UNIMPLEMENTED;
    try {
      return *p == *dynamic_cast<IcuFirstWordBreakIterator const&>(o).p;
    } catch (std::bad_cast&) {
      return *p == o;
    }
  }
  virtual BreakIterator* clone() const {
    SDL_ICUCASE_UNIMPLEMENTED;
    return new IcuFirstWordBreakIterator(locale, p->clone());
  }
  virtual CharacterIterator& getText() const {
    SDL_ICUCASE_UNIMPLEMENTED;
    return p->getText();
  }
  virtual UText* getUText(UText* fillIn, UErrorCode& status) const {
    SDL_ICUCASE_UNIMPLEMENTED;
    return p->getUText(fillIn, status);
  }
  virtual void adoptText(CharacterIterator* it) {
    SDL_ICUCASE_UNIMPLEMENTED;
    return p->adoptText(it);
  }
  virtual int32_t last() {
    SDL_ICUCASE_UNIMPLEMENTED;
    return p->last();
  }
  virtual int32_t previous() {
    SDL_ICUCASE_UNIMPLEMENTED;
    return p->previous();
  }
  virtual int32_t following(int32_t offset) {
    SDL_ICUCASE_UNIMPLEMENTED;
    return p->following(offset);
  }
  virtual int32_t preceding(int32_t offset) {
    SDL_ICUCASE_UNIMPLEMENTED;
    return p->preceding(offset);
  }
  virtual UBool isBoundary(int32_t offset) {
    SDL_ICUCASE_UNIMPLEMENTED;
    return p->isBoundary(offset);
  }
  virtual BreakIterator* createBufferClone(void* buf, int32_t& size, UErrorCode& status) {
    SDL_ICUCASE_UNIMPLEMENTED;
    return p->createBufferClone(buf, size, status);
  }
};


}}

#endif
