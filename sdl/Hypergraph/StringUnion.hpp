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

    build a hypergraph recognizer for a union of weighted strings, or the kleene
    star (0 or more repetitions of some string in the set), optionally: build a
    transducer which inserts markers into output between each recognized string

    see StringUnionFromWordList.hpp for building one from a wordlist file as in
    the HgTrie and StatisticalTokenizerTrain programs
*/

#ifndef SDL_HYPERGRAPH_STRINGUNION_HPP
#define SDL_HYPERGRAPH_STRINGUNION_HPP
#pragma once

// TODO: no need to add unigrams from whole trie; just rho + arcs leaving start
// state (that aren't already whole words). there's a difference only when there
// are seen chars that never appear word-initial. probably not important. would
// be nice to make it easy to have unk-weight same as normalized unigram-addk,
// though.

/* aka dictionary

//TODO: push weights left (doesn't matter until we have beam-compose-best)

*/

#include <sdl/Hypergraph/WeightedStrings.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/StringToHypergraph.hpp>
#include <sdl/Hypergraph/Label.hpp>
#include <sdl/Hypergraph/SymbolPrint.hpp>
#include <sdl/Hypergraph/Splice.hpp>
#include <sdl/Util/Sorted.hpp>
#include <sdl/Hypergraph/Adjacency.hpp>

namespace sdl {
namespace Hypergraph {

// TODO@MD from JG; this seems copied (mostly) from HgTrie options and some of the usage/options may not make
// sense
struct StringUnionOptions {
  std::string unkUnigramWeight;
  bool unweighted;
  bool xmtBlock, xmtBlockViaUnkUnigram;
  bool push_weights;

  bool loop;  // if !loop, the ignoreTags is meaningless

  std::string endOfTokenSequenceWeight, perTokenWeight;

  static std::string const& openToken() { return TOK_START::TOKEN; }  // { return "<tok>"; }
  static std::string const& closeToken() { return TOK_END::TOKEN; }  // { return "</tok>"; }

  void setWrapTokens() {
    beginString = openToken();
    setCloseToken();
  }

  void setCloseToken() { endString = closeToken(); }

  bool chars;
  std::string wordsep;
  std::string wGreedyAscii, wStartGreedyAscii;

  typedef std::string IgnoreTag;
  typedef std::vector<IgnoreTag> IgnoreTags;
  typedef std::string WhitespaceToken;
  typedef std::vector<WhitespaceToken> WhitespaceTokens;
  IgnoreTags ignoreTags;
  WhitespaceTokens whitespaceTokens;
  std::string opentag(IgnoreTag const& t) const { return "<" + t + ">"; }
  std::string closetag(IgnoreTag const& t) const { return "</" + t + ">"; }

  typedef std::string SepString;

  SepString endString, endStringInput;
  SepString beginString, beginStringInput;
  float whitespaceBreakCost;

  StringUnionOptions()
      : unkUnigramWeight("100")
      , xmtBlock(false)
      , push_weights(true)
      , loop(true)
      , endString(" ")
      , endStringInput("")
      , beginString("")
      , beginStringInput("")
      , chars(true)
      , wordsep(" ")
      , wGreedyAscii("")
      , wStartGreedyAscii("")
      , unweighted(false)
      , whitespaceTokens(1, " ")
      , wildcardIsRho(true)
      , xmtBlockViaUnkUnigram(true)
      , whitespaceBreakCost(-100)
      , avoidEpsilon(true) {}
#if !defined(SDL_ASCII_AB)
  enum { kPrintableAsciiMin = '!', kPrintableAsciiMax = '~' };
#else
  // for debugging: smaller list of 'ascii' chars
  enum { kPrintableAsciiMin = 'a', kPrintableAsciiMax = 'b' };
#endif
  static char const* caption() { return "Tokenization Trie"; }
  bool wildcardIsRho, avoidEpsilon;
  template <class Config>
  void configure(Config& c) {
    c.is("Statistical tokenization options");
    c("dictionary as weighted (unigram) trie for statistical tokenization transducer");
    c("unweighted", &unweighted)("use weight one (unweighted) for all words");
    c('u')("unk-weight", &unkUnigramWeight)(
        "add a wildcard transition with this weight (to start state only) for oov chars that adds end-string "
        "symbols. since these are oov chars, ")
        .defaulted();
    c("wildcard-is-rho", &wildcardIsRho)(
        "the wildcard used in unk-weight only applies to chars not seen (in which case unk-weight doesn't "
        "matter. however, if ne.fsa-file starts accepting an unknown-to-wordlist character but doesn't "
        "complete it with a token, this should be false, or else you will fail to derive some strings. "
        "otherwise the default, true, is better")
        .defaulted();
    c('l')("loop",
           &loop)("accept String* rather than just String (final = start state unless weight-stop!=0)")
        .defaulted();
    c('s')("stop-weight", &endOfTokenSequenceWeight)(
        "go to final state from start with this weight (epsilon); use only with --loop. this is the "
        "probability of generating no more words")
        .defaulted();
    // TODO: separate end of tok weight vs end of sentence weight
    c('T')("tok-weight",
           &perTokenWeight)("cost for each token e.g. 1-[stop-weight]. unset means prob = 1 (cost 0)")
        .defaulted();
    c('t')("ignore-tags", &ignoreTags)("ignore tokens between <ignore-tag> ... </ignore-tag>").defaulted();
    c("xmt-block", &xmtBlock)(
        "force <xmt-blockN> and </xmt-block> in a token to be single-character words - effectively forcing a "
        "token to end on the left and start on the right of the block open or close tag. this already "
        "happens by default (see xmt-block-via-unk-unigram) if you allow the unk-weight novel-character "
        "backoff.")
        .defaulted();
    c("xmt-block-via-unk-unigram", &xmtBlockViaUnkUnigram)(
        "allow the unk-weight unigram cost to apply for xmt-block tokens (should be harmless except for the "
        "extra weight, and faster)")
        .defaulted();
    c('b')("begin-string",
           &beginString)("also output this token before each recognized word. empty means epsilon")
        .defaulted();
    c('e')("end-string", &endString)("also output this token after each recognized word. empty means epsilon")
        .defaulted();
    c('E')("input-end-string", &endStringInput)("input token expected after words. empty means epsilon")
        .defaulted();
    c('B')("input-begin-string", &beginStringInput)("input token expected before words. empty means epsilon")
        .defaulted();
    c('p')("push-weights", &push_weights)(
        "push weights as early toward start as possible before forming loop if any (TODO: implement)")
        .defaulted()
        .verbose()
        .todo();
    c("whitespace-break-cost", &whitespaceBreakCost)(
        "if for some reason you have whitespace-tokens allowed as part of named entities or words, you can "
        "increase this in order to see more of those space-embedded words. the default is a negative cost - "
        "a high reward for deleting the space and forcing a token break")
        .init(-100);
    c('w')("whitespace-tokens", &whitespaceTokens)(
        "TODO: these (multiple args allowed) tokens are deleted, but 1 or more of them force a token "
        "boundary on either side")
        .defaulted();
    c('A')("greedy-ascii-weight", &wGreedyAscii)(
        "combine consecutive printable non-space ascii (unicode codepoints 33-127, i.e. '!'-'~') chars into "
        "a single token, with this weight per character (empty weight string=disabled). this means that "
        "ascii characters don't propose unk-weight single-character words.")
        .defaulted();
    c("start-greedy-ascii-weight",
      &wStartGreedyAscii)("if set, a different weight for the first character of an ascii-run token.")
        .defaulted();
    c("avoid-epsilon", &avoidEpsilon)(
        "(in case begin and end string are both epsilon) make the trie nondeterministic in input symbol to "
        "avoid epsilon transitions")
        .init(true);
  }
};

/**
   constructing this has the effect of populating a Hg according to the StringUnionOptions.

   you can modify the built Hg by keeping the object around - see
   StatisticalTokenizerTrain/EntityRecognizerOptions for an example

*/

// TODO@MD from JG: again, this seems to duplicate a lot of the HgTrie code (and
// there may be some un-fixed bugs in this fork). the shared code can be reused
// via the appropriate template or base classes
template <class A>
struct BuildStringUnion {
  typedef A Arc;
  typedef typename A::Weight Weight;
  typedef WeightedStrings<Weight> WS;
  typedef IMutableHypergraph<A> HG;

  typedef std::vector<Syms> Strings;
  typedef std::vector<Weight> Weights;

  // if we add a new first-letter outarc, we use perTokenWeight, which is always as early as possible - ignore
  // push_weights. every arc out of start except for the endOfTokenSequenceWeight one bears perTokenWeight

  // must be in sorted order since we don't hash - we just check last added arc
  StateId addWord(Syms const& w) {
    StateId s = trieStart;
    SymsIndex i = 0, n = w.size();
    // if input-end-string is epsilon then build nondeterministic (copy of final arc w/ wt on it)
    if (n && !end) --n;
    for (; i < n; ++i) {  // TODO: slightly faster to find matching substring of previously added string and
      // add new nodes from there.
      // follow existing trie links
      ArcId nOut = hg.numOutArcs(s);
      if (nOut) {
        Arc const& a = *hg.outArc(s, nOut-1);
        if (hg.inputLabel(a.fsmSymbolState()) == w[i]) {
          s = a.head();
          continue;
        }
      }
      break;
    }
    // i points at the first letter in w that we need to add a state for (the previous word shared the prefix
    // w[0...i) due to sorting)
    for (; i < n; ++i) {
      StateId p = s;  // we need a copy because fn arguments aren't sequenced left->right below:
      hg.addArcFsa(p, s = hg.addState(), w[i], i ? one : perTokenWeight);
    }
    return s;
  }

  Weight one;
  Weight const& weight(std::size_t i) const { return opt.unweighted ? one : ws.weights[i]; }

  // TODO: for ws.opt.unweighted, we could share suffix sets as well as prefix sets (relatively easy achieved
  // recursively hashing the suffix sets to return the canonical representative)

  void buildTrie(WS const& ws) {
    std::size_t n = ws.strings.size();
    for (std::size_t i = 0; i < n; ++i) {
      Syms const& w = ws.strings[i];
      Weight const& wt = weight(i);
      StateId s = addWord(w);
      Weight wall = w.empty() ? times(wt, perTokenWeight) : wt;
      if (opt.avoidEpsilon && !end && !w.empty()) {
        addFinalSym(s, wall, w.back());
      } else {  // empty word is handled here no matter what (most people won't add an empty word, though)
        addFinal(s, wall);
      }
      // TODO: push_weights (postprocess for general (at least acyclic fsm) HG?)
    }
  }

  void addFinalSym(StateId s, Weight const& wt, Sym label) {
    hg.addArcFsm(s, loopEndState, hg.addState(label), s == start ? times(wt, perTokenWeight) : wt);
  }

  void addFinal(StateId s, Weight const& wt) { hg.addArcFsm(s, loopEndState, end.labelState, wt); }

  StateId start;  /// actual start state. if loop then this is also the loopEndState. <xmt-blockN> tags start
  /// from here (aren't wrapped in <tok>)
  StateId trieStart;  /// start of dictionary (may be after open tag)
  StateId loopEndState;  /// where you go after finishing word+weight and its </tok> tag if any. <xmt-blockN>
  /// tags go to here (aren't wrapped in </tok>)
  StateId endStringThenFinal;  /// return here for ->loopEndState with </tok> (no weight added)

  /**
     a solitary (nobody else shares it) end state so you can splice an entity hg w/ outarcs from final.
  */
  StateId safeEndStringThenFinal() {
    StateId newState = hg.addState();
    if (end)
      addFinal(newState, one);
    else
      hg.addArcFsa(newState, loopEndState);
    return newState;
  }

  StateId getEndStringThenFinal() {
    if (endStringThenFinal == Hypergraph::kNoState) {
      if (end) {
        endStringThenFinal = hg.addState();
        addFinal(endStringThenFinal, one);
      } else
        endStringThenFinal = loopEndState;
    }
    return endStringThenFinal;
  }

  typedef IHypergraph<A> IHG;
  typedef shared_ptr<IHG const> PIHG;

  void addEntityRecognizer(IHG const& entityHg, SplicePrefixOptions const& preOpt) {
    addEntityRecognizer(ptrNoDelete(entityHg), preOpt);
  }

  void addEntityRecognizer(PIHG const& entityHg, SplicePrefixOptions const& preOpt) {
    FullSpliceOptions opt(preOpt);
    StateId* sf = opt.sf[SpliceStateOptions::kTargetHg];
    sf[SpliceStateOptions::kStart] = trieStart;
    StateId entityFinal = entityHg->final();
    bool outFromFinal = countOutArcs(*entityHg, entityFinal);
    sf[SpliceStateOptions::kFinal] = outFromFinal ? safeEndStringThenFinal() : getEndStringThenFinal();
    Splice<A> splice(entityHg, opt);
    inplace(hg, splice);
  }

  typedef std::vector<StateId> LastTrie;
  Weight unkUnigramWeight, endOfTokenSequenceWeight, perTokenWeight;

 private:
  // always (private) build() in constructor? then don't need to track if it's done or not ... maybe there are
  // things we want to modify first before building, though (haven't decided).
  void build() {
    Sym wildcard = opt.wildcardIsRho ? RHO::ID : SIGMA::ID;
    // it's okay that this is rho instead of sigma only because we always add
    // every observed letter explicitly as a unigram token. but take care if you use an AutoRule ne-hg that
    // introduces new tokens without accepting them as unigrams - this might leave you with no tokenization of
    // some inputs
    assert(ws.voc == voc);
    hg.setStart(trieStart = start = hg.addState());
    StateId realFinal = loopEndState = (opt.loop ? start : hg.addState());
    hg.setFinal(realFinal);
    if (begin) {
      trieStart = hg.addState();
      hg.addArcFsm(start, trieStart, begin.labelState, Weight::one());
    }
    LastTrie triep;
    if (endOfTokenSequenceWeight != Weight::one()) {
      if (!opt.loop)
        SDL_THROW_LOG(Hypergraph, InvalidInputException, "--stop-weight is allowed only with --loop");
      realFinal = hg.addState();
      hg.addArcFsa(loopEndState, realFinal, EPSILON::ID, endOfTokenSequenceWeight);
      hg.setFinal(realFinal);
    }

    bool unkUnigram = !opt.unkUnigramWeight.empty();
    if (unkUnigram)
      hg.addArcFsa(trieStart, getEndStringThenFinal(), wildcard, times(unkUnigramWeight, perTokenWeight));
    if (opt.xmtBlock && !(opt.xmtBlockViaUnkUnigram && unkUnigram))
      for (Sym b = BLOCK_START::ID; b <= BLOCK_END::ID; ++b)
        hg.addArcFsa(trieStart, getEndStringThenFinal(), b);  // preserve on input

    for (LabelPair t : tags) {  // don't-tokenize brackets
      StateId stIgnore = hg.addState();
      hg.addArcFsa(start, stIgnore, input(t));  // open tag
      hg.addArcFsa(stIgnore, stIgnore, RHO::ID);  // stay in ignore state ...
      hg.addArcFsa(stIgnore, start, output(t));  // unless close tag
    }
    buildTrie(ws);
    bool greedyWeight = !opt.wGreedyAscii.empty();
    bool useascii = greedyWeight || !opt.wStartGreedyAscii.empty();
    if (useascii) {  // this must come last.
      Weight wa, wsa;
      if (greedyWeight)
        wa.set(opt.wGreedyAscii);
      else
        wa = Weight::one();

      if (opt.wStartGreedyAscii.empty())
        wsa = wa;
      else
        wsa.set(opt.wStartGreedyAscii);

      timesBy(perTokenWeight, wsa);
      StateId stAscii
          = hg.addState();  // doesn't reuse endStringThenFinal because we want to stay here consuming ascii*.
      hg.addArcFsm(stAscii, loopEndState, end.labelState);
      std::string s(".");
      for (char c = (char)StringUnionOptions::kPrintableAsciiMin;
           c <= (char)StringUnionOptions::kPrintableAsciiMax; ++c) {
        s[0] = c;
        StateId asciiCharSt = hg.addState(lexicalSymbol(s, *voc));
        hg.addArcFsm(trieStart, stAscii, asciiCharSt, wsa);
        hg.addArcFsm(stAscii, stAscii, asciiCharSt, wa);
      }
    }
    if (!opt.whitespaceTokens.empty()) {
      Weight wSpace;
      wSpace.value_ = opt.whitespaceBreakCost;
      LabelPair deleteSpace;
      deleteSpace.second = EPSILON::ID;
      for (std::string const& spaceStr : opt.whitespaceTokens) {
        SDL_INFO(StatisticalTokenizerTrain, "adding whitespace-token " << spaceStr);
        deleteSpace.first = lexicalSymbol(spaceStr, *voc);
        StateId spaceState = hg.addState(deleteSpace);
        hg.addArcFsm(start, start, spaceState, wSpace);
      }
    }
  }

 public:
  WS const& ws;
  HG& hg;

  struct Separator {
    LabelPair labels;
    StateId labelState;
    bool epsInput;  // epsilon
    bool epsOutput;
    bool epsBoth;
    void init(HG& hg, std::string const& inputOrEmpty, std::string const& outputOrEmpty) {
      IVocabulary& voc = *hg.getVocabulary();
      labelState = hg.addState(
          labels = makeLabelPair(lexicalOrEpsilon(inputOrEmpty, voc), lexicalOrEpsilon(outputOrEmpty, voc)));
      epsInput = (input(labels) == EPSILON::ID);
      epsOutput = (output(labels) == EPSILON::ID);
      epsBoth = epsInput && epsOutput;
    }
    operator bool() const { return !epsBoth; }
  };
  Separator begin, end;

  IVocabularyPtr voc;
  StringUnionOptions opt;
  typedef std::vector<LabelPair> Tags;
  Tags tags;  // input = open, output = close

  /// if s is empty, leaves w alone
  static void maybeParseWeight(std::string const& s, Weight& w) {
    if (!s.empty()) w.set(s);
  }

  BuildStringUnion(WS const& ws, HG& hg, StringUnionOptions const& opt = StringUnionOptions())
      : ws(ws), hg(hg), opt(opt), one(Weight::one()) {
    hg.clear(kFsmOutProperties);
    hg.setVocabulary(ws.voc);
    assert(ws.voc == hg.getVocabulary());
    voc = hg.getVocabulary();

    endStringThenFinal = Hypergraph::kNoState;

    endOfTokenSequenceWeight = one;
    perTokenWeight = one;

    maybeParseWeight(opt.endOfTokenSequenceWeight, endOfTokenSequenceWeight);
    maybeParseWeight(opt.perTokenWeight, perTokenWeight);
    maybeParseWeight(opt.unkUnigramWeight, unkUnigramWeight);

    begin.init(hg, opt.beginStringInput, opt.beginString);
    end.init(hg, opt.endStringInput, opt.endString);

    for (std::string const& s : opt.ignoreTags)
      tags.push_back(lexicalPair(opt.opentag(s), opt.closetag(s), *voc));
    SDL_TRACE(Hypergraph.StringUnion, "tags=" << printer(tags, Util::stateRange(voc)));
    build();
  }
};


}}

#endif
