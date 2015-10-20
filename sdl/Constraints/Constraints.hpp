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

    .
*/

#ifndef ALLCONSTRAINTS_JG_2014_06_12_HPP
#define ALLCONSTRAINTS_JG_2014_06_12_HPP
#pragma once

#include <set>

#include <sdl/Syms.hpp>
#include <sdl/Constraints/ConstraintsConfig.hpp>
#include <sdl/Util/Flag.hpp>
#include <sdl/xmt/XmtSharedExport.hpp>
#include <sdl/Util/Add.hpp>
#include <sdl/Hypergraph/Span.hpp>
#include <sdl/Constraints/Substitute.hpp>
#include <sdl/Util/PrintRange.hpp>
#include <sdl/Array.hpp>
#include <utility>

namespace sdl {

VERBOSE_EXCEPTION_DECLARE(ConstraintNestingException)

char const* const kViolationConstraintOpensStr = "violation of constraint-opens: all arcs leaving state opens(x) must have open #x, and open #x appears nowhere else";

enum { kNoWarnPastEnd = 0, kWarnPastEnd = 1 };
enum { kNoWarnOverlapping = 0, kWarnOverlapping = 1 };
enum { kNoReplacementOnly = 0, kReplacementOnly = 1 };

static std::string const kProtectDecoder("<protect-decoder>");
static std::string const kNoOutput("<no-output>");

struct EntityConstraint : EntityConstraintBase {
 private:
  mutable Position inputTokenId_;

 public:
  Position inputTokenId(TokenSpans& inputTokenSpans) const {
    if (inputTokenId_ == kNullPosition) {
      inputTokenId_ = inputTokenSpans.size();
      inputTokenSpans.push_back(span);
    }
    return inputTokenId_;
  }

  TokenSpan inputSpan;  // span might get modified as we transform things. TODO: modify span as we => hg span
  // (no need for hgspan field since we remember inputSpan)

  Replacing replaces() const {
    return (Replacing)((decoderOutput ? kDecoderReplacement : 0) | (finalOutput ? kFinalReplacement : 0));
  }

  /// after calling replacement(r), you can clear the returned replacement to mark it completed
  void clearReplacement(Replacing r) {
    if (r == kNoReplacement) return;
    replacementWritable(r).reset();
  }

  SpaceSeparatedTokensPtr& replacementWritable(Replacing r) {
    assert(r != kNoReplacement);
    if (r == kDecoderElseFinalReplacement) return !decoderOutput ? finalOutput : decoderOutput;
    return (r == kDecoderReplacement || !finalOutput) ? decoderOutput : finalOutput;
  }

  /// favors finaloutput unless kDecoderReplacement only requested
  SpaceSeparatedTokensPtr replacement(Replacing r) const {
    if (r == kDecoderElseFinalReplacement) return !decoderOutput ? finalOutput : decoderOutput;
    return r == kNoReplacement ? SpaceSeparatedTokensPtr()
                               : ((r == kDecoderReplacement || !finalOutput) ? decoderOutput : finalOutput);
  }

  /// \pre hasOutput()
  std::string const& describeOutput() const {
    assert(hasOutput());
    if (protectDecoder) return kProtectDecoder;
    if (finalOutput) return *finalOutput;
    if (decoderOutput) return *decoderOutput;
    return kNoOutput;  // shouldn't happen
  }

  Syms syms(IVocabulary& voc, Replacing replacing, BlockId enclosedInBlock = 0) const {
    Syms s;
    return syms(s, voc, replacing, enclosedInBlock);
  }

  /**
     \param addBlockid: if >0, prepend <xmt-blockN> #addBlockid and postpend </xmt-block>
  */
  Syms& syms(Syms& syms, IVocabulary& voc, Replacing replacing, BlockId enclosedInBlock = 0) const;

  /// this one doesn't really matter but std::vector wants it.
  EntityConstraint()
      : inputTokenId_(kNullPosition), inputSpan(kNullTokenSpan), hgspan(Hypergraph::kNullSpan) {}

  explicit EntityConstraint(EntityConstraintBase const& o)
      : EntityConstraintBase(o)
      , inputTokenId_(kNullPosition)
      , inputSpan(o.span)
      , hgspan(Hypergraph::kNullSpan) {}

  /// sets inputSpan too
  void initSpan(Position begin, Position end) {
    setSpan(begin, end);
    inputSpan = span;
  }

  /// id=0 means StringToHg will assign ids 1...n with getBlockOpenChecked(1) for first constraint
  EntityConstraint(Position begin, Position end)
      : inputTokenId_(kNullPosition), hgspan(Hypergraph::kNullSpan) {
    initSpan(begin, end);
  }

  /// make another instance of a final-output constraint due to decoding using
  /// it in several places (to keep BlockSpans invariant).
  EntityConstraint(EntityConstraint const& cloneFrom, Hypergraph::Span hgspan)
      : EntityConstraintBase(cloneFrom)
      , inputTokenId_(kNullPosition)
      , inputSpan(cloneFrom.inputSpan)
      , hgspan(hgspan) {
    finalOutput = cloneFrom.finalOutput;
    decoderOutput = cloneFrom.decoderOutput;
  }

  friend inline std::ostream& operator<<(std::ostream& out, EntityConstraint const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out, IVocabulary* voc = 0) const {
    out << '[';
    EntityConstraintBase::print(out);
    out << " input-span=[" << inputSpan.first << "," << inputSpan.second << ")";
    out << ']';
  }

  Hypergraph::Span hgspan;

  /// intention: ordered by first, then by second on ties (so most recently added is soonest closed, per paren
  /// nesting)
  bool operator<(EntityConstraint const& o) const { return span < o.span; }

  void saveInputSpan() { inputSpan = span; }

  bool keep(KeepConstraints k) const { return keepsConstraints(k, (bool)finalOutput); }

  bool emptySpan() const { return span.first == span.second; }
};

typedef std::vector<EntityConstraint> EntityConstraints;

SDL_ENUM(DecodeConstraintType, 4, (NoDecodeConstraint, PrefixDecode, ForceDecode, LmPrefix))

struct DecodeConstraint {
  SpaceSeparatedTokens tokens;
  DecodeConstraintType typeOfTarget;
  bool enabled() const { return typeOfTarget != kNoDecodeConstraint; }
  bool isForce() const { return typeOfTarget == kPrefixDecode || typeOfTarget == kForceDecode; }
  bool isPrefixForce() const { return typeOfTarget == kPrefixDecode; }
  bool isLmPrefix() const { return typeOfTarget == kLmPrefix; }

  friend inline std::ostream& operator<<(std::ostream& out, DecodeConstraint const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const { out << " type=" << typeOfTarget << " tokens=" << tokens; }
  void clear() {
    typeOfTarget = kNoDecodeConstraint;
    tokens.clear();
  }
  DecodeConstraint() : typeOfTarget(kNoDecodeConstraint) {}

  DecodeConstraint(DecodeConstraint&& o) = default;
  DecodeConstraint& operator=(DecodeConstraint&& o) = default;
  DecodeConstraint(DecodeConstraint const& o) = default;
  DecodeConstraint& operator=(DecodeConstraint const& o) = default;
};

struct XMT_API Constraints : EntityConstraints {
  /// guard against accidental double-insertion of block symbols into hg
  DecodeConstraint decode;
  Util::Flag blockSymbolsInserted_;
  Util::Flag outputSubstituted_;
  Util::Flag unambiguous_;
 private:
  bool maybeUnsorted_;
  /// for markOutputSubstituted()
  Util::Flag anyFinalOutput_, anyDecoderOutput_;
 public:

  Constraints(Constraints&& o) = default;
  Constraints& operator=(Constraints&& o) = default;
  Constraints(Constraints const& o) = default;
  Constraints& operator=(Constraints const& o) = default;

  bool keepAny(KeepConstraints k) const {
    return k == kKeep_Constraints || keepsConstraints(k, needsSubstituteFinalOutput());
  }

  /// you will copy from constraintsIn to *this, then call notifyCloned() when done
  void prepareClone(EntityConstraints& constraintsIn) { constraintsIn.swap((EntityConstraints&)*this); }

  /// assuming you modified the EntityConstraints by adding clones
  void notifyCloned() { notifyMaybeUnsorted(); }

  void notifyChanged() {
    notifyMaybeUnsorted();
    notifyOutputsChanged();
  }

  void notifyMaybeUnsorted() { maybeUnsorted_ = true; }

  /// if you change a constraint to have no output where it previously had one,
  /// then you'll have to notifyOutputsChanged(). if you add an output then it's
  /// sufficient to just call this
  void notifyOutputChanged(EntityConstraint const& c);

  void notifyOutputsChanged();

  void markOutputSubstituted() {
    SDL_DEBUG(Constraints.markOutputSubstituted, "before: " << outputSubstituted_
                                                            << "; marking as substituted (true)");
    outputSubstituted_ = true;
  }
  bool needsSubstituteFinalOutput() const { return !outputSubstituted_ && anyFinalOutput(); }

  void clearEntityConstraints() { EntityConstraints::clear(); }

  void clear();

  Constraints() : maybeUnsorted_(false) {}

  /// sets inputSpan to original span. //TODO: best to fix indices vs. inputString before disambiguateOutputs
  void init(ConstraintsConfig const& config, std::string const* inputString = 0);

  explicit Constraints(ConstraintsConfig const& config);

  /**
     remove overlapping *-output constraints (done immediately, before creating
     blockid symbols in a hg). happens already in constructor, so no need to call
     unless you add more later. we don't bother to decode the lattice in favor of
     longest replacements (although we greedily do that, in effect); user should
     read the warning and resolve the problem themselves (since only they know
     what's intended).

     when same-span constraints are present, keep just one (preferring one that has
     an output)
  */

  void disambiguateOutputs(bool warn = true);

  /**
     \pre must be sorted

     (doesn't expand format blocks because first we need to pick an unambiguous
     set of outputs; don't over-expand)

    is-format:
         if true, block any output that touches us but isn't contained by us (except for spread-format)

    spread-format:
         'spread-format: true' applies any format the constraint it touches to the whole output (by growing
         the 'is-format: true' blocks that touch it). for output-constraints w/ 'spread-format': false, we
         remove the constraint if it touches any format that doesn't contain it.

  */
  void removeFormatConflicts(bool warn = true);

  void spreadBlocksByOutputs();

  /**
     same-span non-output constraints get collapsed together. \pre must be sorted
  */
  void removeDuplicateBlocks(bool collapseOutputAndBlock = false, bool warn = false);

  /// clone output from original constraint into its own hgspan (for maintaining BlockSpans invariant)
  ConstraintIndex clone(EntityConstraint const& cloneFrom, Hypergraph::Span const& hgspan) {
    ConstraintIndex r = size();
    push_back(EntityConstraint(cloneFrom, hgspan));
    return r;
  }

  ConstraintIndex clone(EntityConstraint const& cloneFrom, Hypergraph::StateId leaving = Hypergraph::kNoState) {
    return clone(cloneFrom, Hypergraph::Span(leaving, Hypergraph::kNoState));
  }

  bool enabled() const { return decode.enabled() || !this->empty(); }

  EntityConstraint const& entity(BlockId id) const {
    assert(id <= size());
    return (*this)[id - 1];
  }
  EntityConstraint& entity(BlockId id) {
    assert(id <= size());
    return (*this)[id - 1];
  }

  /// remove the valid whole-sentence constraint if it has no output specified,
  /// and invalid start-past-end constraints
  void removeUselessConstraints(Position segmentEnd);

  /**
      For each constraint: If it ends after segment end it is fixed to end at
      segment end.
     *

      \param segmentEnd End position of segment
   */
  void fixConstraintEnds(Position segmentEnd, bool warn = kWarnPastEnd);

  /// leave only useful+valid constraints
  void fixSegmentEnd(Position segmentEnd, bool warn = kWarnPastEnd) {
    fixConstraintEnds(segmentEnd, warn);
    removeUselessConstraints(segmentEnd);
  }

  /// throw if anything needing fixSegmentEnd is left
  void checkSegmentEnd(Position segmentEnd);

  friend inline std::ostream& operator<<(std::ostream& out, Constraints const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const { out << Util::print(*this, Util::multiLine()); }

  /// \return false if you should call sort
  bool isSorted() const;

  /// like isSorted but for debugging assertions - actually checks instead of
  /// trusting maybeUnsorted_ == false
  bool checkSorted() const;

  /// post: ensures sorted, provided you call notifyMaybeUnsorted() whenever you
  /// add things out of order. you might want to call sort() after modifying
  /// spans if SpansLeftmostLargestFirst sort order might have changed (so
  /// e.g. filtering spans, shifting them right by some monotonically increasing
  /// amount, or inserting some in the correct position wouldn't require a
  /// sort() afterwards)
  void sort();

  bool anyFinalOutput() const { return anyFinalOutput_; }

  bool anyDecoderOutput() const { return anyDecoderOutput_; }

  bool anyReplacements() const { return anyFinalOutput() || anyDecoderOutput(); }
  bool replacing(Replacing r) const {
    if (r == kDecoderElseFinalReplacement || r == kFinalElseDecoderReplacement)
      return anyDecoderOutput_ || anyFinalOutput_;
    else
      return ((r & kDecoderReplacement) && anyDecoderOutput_) || ((r & kFinalReplacement) && anyFinalOutput_);
  }
  /// you can't sort after putting block index symbols in hg (TODO: we could
  /// sort a list of indices or pointers instead, not modifying the original
  /// EntityConstraints vector)
  void notifyBlockSymbolsInserted(bool set = true) { blockSymbolsInserted_ = set; }

  bool blockSymbolsInserted() const { return this->empty() || blockSymbolsInserted_; }


  /// call v(*this, i) for whatever overlaps query. O(n^2) - TODO: bitset per
  /// position, or interval tree? or nested containment list
  /// ( http://bioinformatics.oxfordjournals.org/content/23/11/1386.full )
  template <class Visitor>
  void visitIntersecting(TokenSpan query, Visitor const& v) {
    for (ConstraintIndex i = 0, N = size(); i < N; ++i)
      if (collides(query, (*this)[i].span)) v(*this, i);
  }

  void appendEmptyConstraintIds(std::vector<ConstraintIndex> &ids) const {
    for (ConstraintIndex i = 0, N = size(); i < N; ++i)
      if ((*this)[i].emptySpan())
        ids.push_back(i);
  }

};

void setMinimumSpans(EntityConstraints&);

/// nondestructively sorted view into constraints
struct SortedConstraints : std::vector<EntityConstraint*> {
  typedef std::vector<EntityConstraint*> Order;
  SortedConstraints(EntityConstraints const&, bool outputReplacementConstraintOnly = kNoReplacementOnly);
};

/**
   visit constraints left->right - for string->string modules that need to adjust span
   extents, with subclass-overridable open/close constraint events

   also used by StringTohg InsertBlockSymbols.
*/
struct ScanConstraints {
  Constraints& constraints;

  /// index into constraints
  struct ByEnd {
    Constraints const& constraints;
    ByEnd(Constraints const& constraints) : constraints(constraints) {}
    bool operator()(Position a, Position b) const {
      return constraints[a].span.second < constraints[b].span.second;
    }
  };

  typedef Position ConstraintIndex;

  typedef std::multiset<ConstraintIndex, ByEnd, std::allocator<ConstraintIndex> > OpenSet;
  // explicit allocator needed for MSVC 2010

  OpenSet openSet;

  ScanConstraints(Constraints& inplace) : constraints(inplace), openSet(ByEnd(constraints)) { init(); }

  void fixSegmentEnd(Position len, bool warn = kWarnPastEnd);

  ConstraintIndex i, iend;

  void init() {
    constraints.sort();
    i = 0;
    iend = constraints.size();
    SDL_DEBUG(Constraints.ScanConstraints.init, constraints);
  }

  bool done() const { return i == iend; }

  /// gets called on leaving each constraint block as you encounter spans
  /// left->right with advance(span).
  virtual void openConstraint(ConstraintIndex) {}

  /// gets called on leaving each constraint block as you encounter spans
  // left->right with advance(span). close may not exactly nest vs. open in case
  // of matching span end (but you can figure it out by keeping track of the
  // indices yourself. or you may not care
  virtual void closeConstraint(ConstraintIndex) {}

  /// called for empty spans w/ first==second. StringToHg needs to treat these
  /// separately to allow ReplaceSpan on empty hg spans
  virtual void openEmptyConstraint(ConstraintIndex id) {
    openConstraint(id);
    closeConstraint(id);
  }

  /// call openConstraint(ConstraintIndex) for every constraint opened by moving to character index p
  void advanceOpen(Position p);

  /// call closeConstraint(ConstraintIndex) for every constraint closed by
  /// character index p (including one-past-end character)
  void advanceClose(Position p);

  /// calls openConstraint for everything that opens along the way to p. calls closeConstraint for everything
  /// that closes by p (simply calls advanceOpen then advanceClose)
  void advance(Position p);

  void advance(TokenSpan tokenSpan);
};


/// see also Hypergraph/FinalOutput replaceFinalOutput. works even if constraints aren't sorted.
shared_ptr<std::string const> withFinalOutput(std::string const& str, Constraints& constraints);

/// for TransformationRequest / getDerivation - call before adding any tokenspans for decoder/final-output
/// replacement
inline void notifyInputTokensSet(TokenSpans* inputTokenSpans, unsigned* numInputTokensBeforeFinalOutput) {
  if (numInputTokensBeforeFinalOutput && inputTokenSpans) {
    unsigned n = inputTokenSpans->size();
    unsigned& nbefore = *numInputTokensBeforeFinalOutput;
    if (nbefore != (unsigned)-1) return;
    SDL_DEBUG(TransformationRequest.notifyInputTokensSet, "before: " << nbefore << " now set to: " << n);
    nbefore = n;
  } else if (inputTokenSpans) {
    SDL_ERROR(TransformationRequest.applyFinalOutput, "token spans set "
                                                          << Util::makePrintable(*inputTokenSpans)
                                                          << " but #-input-tokens not updated");
  }
}


}

#endif
