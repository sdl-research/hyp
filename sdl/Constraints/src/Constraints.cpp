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

    If every module in your pipeline outputs a hypergraph w/ correct BlockSpans,
    or a string while updating constraints to point at output spans, then we can
    apply final-output simply.

    TODO: for nbest string output, we can't use final-output (but with some
    effort, for MBR with hypergraph input, which selects a single string, we
    could)

    TODO: For modules that don't yet update constraints properly, we can't use
    final-output.
*/


#include <sdl/Constraints/Constraints.hpp>
#include <sdl/Constraints/ConstraintsConfig.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Constraints/ConstraintsForWord.hpp>
#include <sdl/Util/Sorted.hpp>
#include <sdl/Constraints/Substitute.hpp>
#include <utility>
#include <sdl/Vocabulary/HelperFunctions.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>
#include <sdl/Util/Utf8.hpp>
#include <sdl/Array.hpp>
#include <sdl/Util/ZeroInitializedArray.hpp>
#include <sdl/Util/MinMax.hpp>

namespace sdl {

/// this is the sort you must maintain in Constraints: in case of same left,
/// prefer longest first (matters only in case of final-output conflicts)
struct SortEndDescending {
  bool operator()(EntityConstraint const& a, EntityConstraint const& b) const {
    if (a.span == b.span) return a.hasOutput() && !b.hasOutput();
    return a.span.first < b.span.first || a.span.first == b.span.first && a.span.second > b.span.second;
  }
  bool operator()(EntityConstraint const* a, EntityConstraint const* b) const { return (*this)(*a, *b); }
};

void setMinimumSpans(EntityConstraints& c) {
  for (EntityConstraints::iterator i = c.begin(), e = c.end(); i != e; ++i) setMinTokenSpan(i->span);
}

struct IsUselessConstraint : public std::unary_function<EntityConstraint, bool> {

  IsUselessConstraint(Position end) : end_(end) {}

  bool operator()(EntityConstraint const& c) const {
    if (!c.valid()) {  // this is already handled in ConstraintConfig
      SDL_WARN(EntityConstraint, "Bad constraint: " << PrintSpan(c.span));
      return true;
    }
    if (!c.hasOutput() && (c.span.first == 0 && c.span.second >= end_))
      // useless whole sentence constraint with no output
      return true;
    bool isBad = (c.span.first >= end_);
    if (isBad) {
      SDL_WARN(Constraint, "Bad constraint (starts at or after " << end_ << "): " << PrintSpan(c.span));
    }
    return isBad;
  }
  Position end_;
};

void Constraints::removeUselessConstraints(Position segmentEnd) {
  Util::removeShrink(*this, IsUselessConstraint(segmentEnd));
}

void Constraints::fixConstraintEnds(Position segmentEnd, bool warn) {
  for (Constraints::iterator i = begin(), e = end(); i != e; ++i) {
    Position& spanEnd = i->span.second;
    if (spanEnd > segmentEnd) {
      if (warn) {
        SDL_WARN(Constraint, "Fixed bad constraint: " << PrintSpan(i->span) << " to end at " << segmentEnd);
      }
      spanEnd = segmentEnd;
    }
  }
}

void Constraints::checkSegmentEnd(Position segmentEnd) {
  if (!empty()) {
    TokenSpan const& span = back().span;
    if (span.second > segmentEnd || span.first >= segmentEnd)
      SDL_THROW_LOG(NoSwapConstraintsManager, InvalidInputException,
                    "Bad no-swap constraint '"
                        << PrintSpan(span) << "' extends past end of string of size (#unicode codepoints) "
                        << segmentEnd);
  }
}

static bool setDecodeConstraint(DecodeConstraintType constraintType, SpaceSeparatedTokens const& tokens,
                                DecodeConstraint& c) {
  if (tokens.empty()) return false;
  if (c.enabled())
    SDL_THROW_LOG(Constraint, ConfigException,
                  constraintType << " specified, but you can only have one type of decoder constraint: "
                                 << allowed_values(constraintType));
  c.tokens = tokens;
  c.typeOfTarget = constraintType;
  return true;
}

Constraints::Constraints(ConstraintsConfig const& config) {
  init(config);
}

inline std::string const& output(EntityConstraint const* c) {
  SDL_ASSERT_LOG(Constraints.disambiguate, c, "no previous constraint");
  std::string const* o = c->replacement(kFinalElseDecoderReplacement).get();
  SDL_ASSERT_LOG(Constraints.disambiguate, o, "constraint " << c << ": can't get output");
  return *o;
}


void Constraints::removeFormatConflicts(bool warn) {
  typedef std::vector<ConstraintIndex> OpenBlocksClosing;
  typedef std::vector<ConstraintIndex> OpenConflicts;
  assert(checkSorted());
  OpenBlocksClosing openBlocksClosing;  // only isFormat blocks, though
  OpenConflicts openConflicts;
  ConstraintIndex N = size();

  Util::ZeroInitializedHeapArray<char> remove(N);
  for (ConstraintIndex i = 0; i < N; ++i) {
    EntityConstraint const& c = (*this)[i];
    Position const c1 = c.span.first;

    // close ALL open !spread-format outputs
    if (c.isFormat && !openConflicts.empty()) {
      for (OpenConflicts::const_iterator ix = openConflicts.begin(), ex = openConflicts.end(); ix != ex; ++ix) {
        ConstraintIndex const x = *ix;
        Position const xclose = (*this)[x].span.second;
        if (xclose > c1) {
          remove[x] = true;
        }  // else close and keep x
      }
      openConflicts.clear();
    }

    // forget open format blocks already closed when we get to c1
    ConstraintIndex nopen = openBlocksClosing.size();
    if (nopen && c1 >= openBlocksClosing[nopen - 1])
      for (;;) {  // close (some) openBlocksClosing
        if (!--nopen) {
          openBlocksClosing.clear();
          break;
        } else if (c1 < openBlocksClosing[nopen]) {
          openBlocksClosing.resize(++nopen);
          break;
        }
      }
    assert(nopen == openBlocksClosing.size());

    if (c.hasOutput()) {
      if (!c.spreadFormat)
        if (nopen)
          remove[i] = true;
        else
          openConflicts.push_back(i);
    } else {  // block:
      Position const c2 = c.span.second;
      if (nopen) {
        bool const nests = openBlocksClosing[nopen - 1] >= c2;
        if (!nests)
          SDL_THROW_LOG(Constraints, ConstraintNestingException,
                        "Block constraint " << c << " does not nest inside format block ending at "
                                            << openBlocksClosing[nopen - 1]);
      }
      if (c.isFormat) openBlocksClosing.push_back(c2);
    }
  }
  ConstraintIndex i = 0, o = 0;
  for (;; ++i) {
    if (i == N) {
      this->clear();
      return;
    }
    if (!remove[i]) {
      for (;;) {
        (*this)[o++] = (*this)[i];
        for (;;) {
          if (++i == N) {
            this->resize(o);
            return;
          }
          if (!remove[i]) break;
        }
      }
    }
  }
}

void Constraints::spreadBlocksByOutputs() {
  assert(checkSorted());
  typedef std::vector<EntityConstraint*> OpenBlocks;
  OpenBlocks open;  // unlike in removeFormatConflicts, this is *all* non-output blocks
  TokenSpan lastOutputSpan = kNullTokenSpan;
  bool needSort = false;
  for (iterator ic = begin(), icend = end(); ic != icend; ++ic) {
    EntityConstraint& c = *ic;
    Position const c1 = c.span.first;
    OpenBlocks::iterator b = open.begin(), e = open.end(), i = b, o = b;
    for (; i < e; ++i) {
      EntityConstraint* open = *i;
      if (c1 < open->span.second) *o++ = open;  // still open
    }
    if (c.hasOutput()) {
      lastOutputSpan = c.span;
      for (; b != o; ++b) Util::maxEq((*b)->span.second, lastOutputSpan.second);
      open.erase(o, e);
    } else {
      if (c.span.first < lastOutputSpan.second) {
        assert(lastOutputSpan.first <= c.span.first);  // by sort
        c.span.first = lastOutputSpan.first;
        needSort = true;
        Util::maxEq(c.span.second, lastOutputSpan.second);
      }
      open.erase(o, e);
      open.push_back(&c);
    }
  }
  if (needSort) {
    maybeUnsorted_ = true;
    sort();
  }
}

void Constraints::removeDuplicateBlocks(bool collapseOutputAndBlock, bool warn) {
  assert(checkSorted());
  EntityConstraints::iterator i = begin(), e = end(), o = i, b = i;
  TokenSpan lastSpan = kNullTokenSpan;
  for (; i != e; ++i) {
    EntityConstraint& c = *i;
    if (lastSpan == c.span && (collapseOutputAndBlock || !c.hasOutput())) {
      if (warn)
        SDL_DEBUG_ALWAYS(Constraints.disambiguate, "removing duplicate-span constraint " << c << " vs. " << *o);
      o->isFormat |= c.isFormat;
    } else {
      if (o != i) *o = *i;
      ++o;
    }
    lastSpan = c.span;
  }
  if (o != e) erase(o, e);
}


/// use PickConstraints module for more sophisticated choices
void Constraints::disambiguateOutputs(bool warn) {
  if (!unambiguous_.first()) return;
  assert(checkSorted());
  removeDuplicateBlocks(false);
  removeFormatConflicts();
  Position currentOutputEnds = 0;
  EntityConstraints::iterator i = begin(), e = end(), o = i, b = i;
  EntityConstraint const* currentOutput = 0;
  TokenSpan lastSpan = kNullTokenSpan;
  for (; i != e; ++i) {
    EntityConstraint& c = *i;
    if (lastSpan == c.span) {
      // output constraints come first by sort
      if (warn) SDL_DEBUG_ALWAYS(Constraints.disambiguate, "removing duplicate-span constraint " << c);
      continue;
    }
    if (c.span.first >= currentOutputEnds) {  // safe parallel replacement
      if (c.hasOutput()) {
        currentOutput = &c;
        currentOutputEnds = c.span.second;
      }
    } else if (!c.empty() && !c.hasOutput() && currentOutput) {
      c.span.first = currentOutput->span.first;  // grow block to contain
      if (c.span.second < currentOutputEnds) c.span.second = currentOutputEnds;
    } else {
      if (warn)
        SDL_WARN(Constraints.disambiguate, "Constraint "
                                               << c << " blocked by previous output substitution constraint "
                                               << output(currentOutput) << " ending at " << currentOutputEnds
                                               << " (use PickConstraints module first to fine-tune)");
      goto skip;
    }
    if (o != i) *o = *i;
    ++o;
  skip:
    lastSpan = c.span;
  }
  if (o != e) erase(o, e);
  removeDuplicateBlocks(true);
}

inline void outputToSyms(std::string const* str, IVocabulary* vocab, Syms& syms, char const* what,
                         TokenSpan const& span) {
  if (!str) return;
  if (!vocab)
    SDL_THROW_LOG(Constraint, ConfigException,
                  "you specified a " << what << " entity constraint for a pipeline without a " << what
                                     << " vocabulary for " << PrintSpan(span) << ": " << str);
  Vocabulary::splitToTerminals(*str, syms, *vocab);
}

void Constraints::notifyOutputsChanged() {
  anyFinalOutput_.clear();
  anyDecoderOutput_.clear();
  for (EntityConstraints::const_iterator i = begin(), e = end(); i != e; ++i) notifyOutputChanged(*i);
}

void Constraints::notifyOutputChanged(EntityConstraint const& c) {
  unambiguous_ = false;
  if (c.decoderOutput || c.protectDecoder) {
    SDL_DEBUG(Constraint, "final-output (or protect) constraint: " << c);
    anyDecoderOutput_ = true;
  }
  if (c.finalOutput || c.protectFinal) {
    SDL_DEBUG(Constraint, "final-output (or protect) constraint: " << c);
    anyFinalOutput_ = true;
  }
}

void Constraints::clear() {
  maybeUnsorted_ = false;
  clearEntityConstraints();
  decode.clear();
  blockSymbolsInserted_.clear();
  outputSubstituted_.clear();
  anyFinalOutput_.clear();
  anyDecoderOutput_.clear();
  unambiguous_.clear();
}

void Constraints::init(ConstraintsConfig const& config, std::string const* inputString) {
  this->clear();
  for(ConstraintsConfig::Walls::const_iterator i = config.walls.begin(), e = config.walls.end(); i != e; ++i) {
    Position p = *i;
    EntityConstraint c;
    c.span.first = c.span.second = p;
    c.type = kJumpWall;
    this->push_back(c);
    SDL_DEBUG(Constraints, "wall constraint @"<<p<<": "<<c);
    //TODO: of course, we need to replace these when -> hg with special symbol
    //<jump-wall> still (and make sure they aren't deleted on account of being
    //empty first)
  }

  maybeUnsorted_ = true;

  setDecodeConstraint(kForceDecode, config.forceDecode, decode);
  setDecodeConstraint(kPrefixDecode, config.prefixForceDecode, decode);
  setDecodeConstraint(kLmPrefix, config.lmPrefix, decode);

  for (EntityConstraintConfigs::const_iterator i = config.begin(), e = config.end(); i != e; ++i) {
    this->push_back(EntityConstraint(*i));
    EntityConstraint& c = this->back();
    c.saveInputSpan();
    if (c.protectFinal) {
      if (c.finalOutput)
        SDL_WARN(Constraints.protect,
                 "overwriting redundant final-output specification (because protect or protect-final) for "
                     << c);
      if (inputString) {
        std::string const& utf8in = *inputString;
        try {
          char const* utf8 = arrayBegin(utf8in);
          TokenSpan utf8Span = Util::toUtf8ByteSpan(c.span, utf8, arrayEnd(utf8in));
          c.finalOutput = make_shared<std::string>(utf8 + utf8Span.first, utf8 + utf8Span.second);
          SDL_DEBUG_ALWAYS(protect.final.Constraint, "final-output set from protect "
                                                         << PrintSpan(c.span) << ": '" << *c.finalOutput
                                                         << "'");
          c.protectFinal = false;
        } catch (std::exception& e) {
          SDL_WARN(Constraints.protect,
                   e.what() << " - protect span " << c
                            << " doesn't index valid unicode codepoints in input utf8 string '" << utf8in
                            << "': ");
        }
      } else
        SDL_ERROR(Constraints.protect, "input string not available so can't activate protect for " << c);
    }
    notifyOutputChanged(c);
    assert(c.valid());
  }
  sort();
}

void ScanConstraints::fixSegmentEnd(Position len, bool warn) {
  constraints.fixSegmentEnd(len, warn);
  // we don't necessarily assign ids (only StringToHg does that for now)
  init();
}

bool Constraints::isSorted() const {
  return !maybeUnsorted_ || checkSorted();
}

bool Constraints::checkSorted() const {
  return Util::isSorted(*this, SortEndDescending());
}

void Constraints::sort() {
  if (maybeUnsorted_) {
    if (blockSymbolsInserted_) {
      assert(checkSorted());
    } else
      Util::sort(*this, SortEndDescending());
    maybeUnsorted_ = false;
  }
  assert(checkSorted());
}

SortedConstraints::SortedConstraints(EntityConstraints const& constraints, bool outputConstraintOnly)
    : Order(constraints.size()) {
  EntityConstraints& cons = const_cast<EntityConstraints&>(constraints);
  if (outputConstraintOnly) {
    Order::iterator o = begin(), b = o;
    for (EntityConstraints::iterator i = cons.begin(), e = cons.end(); i != e; ++i) {
      if (i->hasOutput()) {
        *o = &*i;
        ++o;
      }
    }
    resize(o - b);
  } else {
    for (ConstraintIndex i = 0, n = cons.size(); i < n; ++i) (*this)[i] = &cons[i];
  }
  Util::sort(*this, SortEndDescending());
}

void WordsInSpan::print(std::ostream& out) const {
  Util::printRange(out, *this);
}

void ScanConstraints::advance(TokenSpan span) {
  SDL_DEBUG(Constraints.advance, "@i=" << i << " advancing to token " << PrintSpan(span));
  advanceOpen(span.second);
  advanceClose(span.second);
}

void ScanConstraints::advance(Position p) {
  advanceOpen(p);
  advanceClose(p);
}


void ScanConstraints::advanceOpen(Position p) {
  for (; i < iend; ++i) {
    EntityConstraint& c = constraints[i];
    Position const constraintFirst = c.span.first;
    if (constraintFirst >= p)
      return;  // sorted by constraintFirst
    else {  //  if (contains(tokenSpan, constraintFirst))
      SDL_DEBUG(Constraints.advance, "opened for char #" << p << " next constraint @" << i << ": "
                                                         << constraints[i]);
      if (c.span.second == constraintFirst)
        openEmptyConstraint(i);
      else {
        openConstraint(i);
        openSet.insert(i);
      }
    }
  }
  SDL_DEBUG(Constraints.advance, "advanced; char #" << p << " => opened: " << Util::makePrintable(openSet)
                                                    << " pending next constraint @" << i);
}

void ScanConstraints::advanceClose(Position p) {
  OpenSet::const_iterator it = openSet.begin(), itbegin = it, itend = openSet.end();
  for (; it != itend; ++it) {
    ConstraintIndex i = *it;
    EntityConstraint& c = constraints[i];
    if (c.span.second <= p) {
      if (c.span.first < c.span.second) {  // openEmptyConstraint already handled the others
        closeConstraint(i);
        SDL_DEBUG(Constraints.advance, "closed for char #" << p << " constraint @" << i << ": "
                                                           << constraints[i]);
      }
    } else
      break;  // sorted by end of span

    // TODO: faster to iterate over all elements of a (usually small - # of
    // active constraints) vector vs. keeping them sorted?
  }
  openSet.erase(itbegin, it);
}

enum { kPredictFinalOutputExpansion = 30 };

shared_ptr<std::string const> withFinalOutput(std::string const& str, Constraints& constraints) {
  if (!constraints.anyFinalOutput()) return ptrNoDelete(str);
  shared_ptr<std::string> r(make_shared<std::string>());
  std::string& out = *r;
  out.reserve(str.size() + kPredictFinalOutputExpansion);
  Position since = 0;
  SortedConstraints sorted(constraints, kReplacementOnly);
  for (SortedConstraints::const_iterator i = sorted.begin(), e = sorted.end(); i != e; ++i) {
    EntityConstraint const& c = **i;
    if (c.finalOutput && since <= c.span.first) {  // safe parallel replacement
      out.append(&str[since], c.span.first - since);
      out.append(*c.finalOutput);
      since = c.span.second;
    }
  }
  out.append(&str[since], str.size() - since);
  constraints.markOutputSubstituted();
  return r;
}

Syms& EntityConstraint::syms(Syms& syms, IVocabulary& voc, Replacing replacing, BlockId enclosedInBlock) const {
  using namespace Vocabulary;
  if (enclosedInBlock) syms.push_back(getBlockOpenChecked(enclosedInBlock));
  SpaceSeparatedTokensPtr replaced(replacement(replacing));
  assert(replaced);
  splitToTerminals(*replaced, syms, voc);
  if (enclosedInBlock) syms.push_back(BLOCK_END::ID);
  return syms;
}


}
