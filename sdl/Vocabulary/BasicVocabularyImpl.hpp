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

    an array of strings and a map from string to id (as used to make IVocabulary
    for a given symbol type).
*/

#ifndef VOCABULARY__BASICVOCABULARYIMPL_H_
#define VOCABULARY__BASICVOCABULARYIMPL_H_
#pragma once

#include <string>
#include <sdl/Util/Unordered.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Util/Constants.hpp>
#include <sdl/Util/LogHelper.hpp>

#include <sdl/Util/Map.hpp>
#include <sdl/Util/IndexedStrings.hpp>
#include <sdl/Sym.hpp>

namespace sdl {
namespace Vocabulary {

/**
   a vocabulary-like class (doesn't bother with the IVocabulary vtable where all
   tokens must be a given type. this is a string<->[0...#strings) where Sym
   corresponding to the string @index are (index + constant offset, constant type)

   all methods require input Sym have the same constant type
*/
class BasicVocabularyImpl {
  typedef Util::IndexedStrings Symbols;

 public:
  enum { kNullIndex = (SymInt)-1 };

  /// must call init() before using
  BasicVocabularyImpl() {
    type_ = (SymbolType)0;
    offset_ = 0;
    type_offset_ = 0;
    maxLstSize_ = (~(SymInt)0);  // can't add if you already have this many
    freezeEndIndex_ = 0;
  }

  /**
     very important: setting correct offset is necessary if you layer a shared (per-process) vocab underneath
     a mutable part
  */
  explicit BasicVocabularyImpl(SymbolType type, SymInt offset = 0) { init(type, offset); }

  void init(SymbolType type, SymInt offset = 0) {
    type_ = type;
    offset_ = offset;
    type_offset_ = type + offset;
    maxLstSize_ = (~(SymInt)0) - offset;  // can't add if you already have this many
    freezeEndIndex_ = 0;
  }

  virtual ~BasicVocabularyImpl() {}

  SymInt offset() { return offset_; }

  SymInt size() const { return (SymInt)symbols_.size(); }

  /**
     make permanent all symbols that were addSymbol at this time (grammar db
     symbols are always permanent)
  */
  void freeze() { freezeEndIndex_ = size(); }

  std::size_t countSinceFreeze() const { return symbols_.size() - freezeEndIndex_; }

  /**
     remove all addSymbol since last freeze (if no freeze, then all of them
     except whatever was permanent on vocab creation e.g. from grammar db).
  */
  void clearSinceFreeze() {
    assert(freezeEndIndex_ <= symbols_.size());
    SDL_INFO(evict.Vocabulary,
             "Shrinking " << (SymbolType)type_ << " vocabulary from " << symbols_.size() << " to " << freezeEndIndex_
                          << " symbols (these " << (symbols_.size() - freezeEndIndex_)
                          << " removed symbols should all be novel words seen in inputs recently processed - "
                             "if not, call IVocabulary::freeze() to keep your permanent symbols permanent");
    symbols_.shrink(freezeEndIndex_);
  }

  /**
     Loads Symbols from a range [i, end) of pair<index, string>.

     you're not allowed to have any missing indices. indices should be
     [0...size) but may come in any order
  */
  template <class Iterator>
  void load(Iterator i, Iterator end, SymbolType type) {
    reset();
    assert(type == type_);
    symbols_.reserve(end - i);
    for (; i != end; ++i) addEntry(i->first, i->second);
    doneLoading();
  }

  void doneLoading() {
    symbols_.rehash();
    freezeEndIndex_ = size();
  }

  /// call this for contiguous index w/o gaps or duplicates, then when done call doneLoading()
  void addEntry(SymInt index, std::string const& name) {
    if (index >= maxLstSize_) throw std::out_of_range("BasicVocab::load");
    symbols_.setDeferHash(index, name);
  }

  /// (everything before offset is a frozen or read only index, too)
  SymInt pastFrozenIndex() const { return offset_ + freezeEndIndex_; }

  SymInt offset_, maxLstSize_, freezeEndIndex_;
  SymbolType type_;

 public:
  Symbols symbols_;
  SymInt type_offset_;

  Sym symForIndex(SymInt i) const {
    Sym sym;
    assert(i < maxLstSize_);
    sym.id_ = i + type_offset_;
    return sym;
  }

  Sym add(Slice s) { return add(std::string(s.first, s.second)); }

  Sym add(std::string const& symbol) {
    SymInt i = symbols_.index(symbol);
    assert(i < maxLstSize_);
    return symForIndex(i);
  }

  SymInt indexAdding(std::string const& symbol) {
    SymInt i = symbols_.index(symbol);
    assert(i < maxLstSize_);
    return i;
  }

  SymInt indexAdding(Slice s) { return indexAdding(std::string(s.first, s.second)); }

  Sym addSymbolMustBeNew(Slice s) { return addSymbolMustBeNew(std::string(s.first, s.second)); }

  Sym addSymbolMustBeNew(std::string const& symbol) {
    SymInt const oldsz = size();
    SymInt const i = symbols_.index(symbol);
    if (size() == oldsz || i >= maxLstSize_)
      throw std::out_of_range("BasicVocab::addSymbolMustBeNew - string was not new");
    return symForIndex(i);
  }

  Sym add(std::string const& symbol, SymbolType type) {
    assert(type == type_);
    return add(symbol);
  }

  Sym addSymbolMustBeNew(std::string const& symbol, SymbolType type) {
    assert(type == type_);
    return addSymbolMustBeNew(symbol);
  }

  std::string const& str(Sym sym) const {
    assert(containsSym(sym));
    return symbols_[sym.index() - offset_];
  }

  SymInt index(std::string const& symbol) const { return symbols_.find(symbol); }

  Sym sym(std::string const& symbol) const {
    SymInt const i = symbols_.find(symbol);
    return i == kNullIndex ? NoSymbol : symForIndex(i);
  }

  bool boundsSym(Sym sym) const {
    return sym.index() < offset_ + symbols_.size();
  }

  bool containsSym(Sym sym) const {
    assert(sym.type() == type_);
    SymInt index = sym.index();
    assert(index >= offset_);
    return index - offset_ < symbols_.size();
  }

  bool containsIndex(SymInt i) const { return i - offset_ < symbols_.size(); }

  bool contains(std::string const& symbol) const { return symbols_.find(symbol) != kNullIndex; }

  SymInt getNumSymbols() const { return size(); }

  std::size_t getSize() const { return symbols_.size(); }

  void reset() {
    symbols_.clear();
    freezeEndIndex_ = 0;
  }

  void accept(IVocabularyVisitor& visitor) {
    SymInt const sz = size();
    Sym sym;
    sym.id_ = type_offset_;
    assert(type_offset_ + sz >= type_offset_);
    assert(offset_ + size() <= Sym::maxIndexForType(type_));
    for (SymInt i = 0; i < sz; ++i, ++sym.id_) visitor(sym, symbols_[i]);
  }
};


}}

#endif
