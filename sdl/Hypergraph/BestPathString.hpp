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

    sequences of symbols, or a formatted string from that, for terminal yields of a best path tree.
*/


#ifndef HYP__BESTPATHSTRING_JG201254_HPP
#define HYP__BESTPATHSTRING_JG201254_HPP
#pragma once

#include <sdl/LexicalCast.hpp>
#include <sdl/Hypergraph/BestPath.hpp>
#include <sdl/Hypergraph/GetString.hpp>
#include <boost/variant/static_visitor.hpp>
#include <functional>
#include <sdl/Util/StringBuilder.hpp>
#include <sdl/Util/WordToPhrase.hpp>
#include <sdl/Config/Init.hpp>
#include <sdl/Util/AcceptString.hpp>
#include <sdl/Util/SpaceToken.hpp>
#include <sdl/Util/Equal.hpp>

namespace sdl {
namespace Hypergraph {

typedef std::function<void(Syms&, IVocabulary*)> SymsRewrite;
struct IdentitySymsRewrite {
  void operator()(Syms&, IVocabulary*) const {}
};

inline SymsRewrite identitySymsRewrite() {
  return SymsRewrite(IdentitySymsRewrite());
}

unsigned const kReserveForBestPathString = 500;


template <class Arc>
void appendBestPathStringForDeriv(Util::StringBuilder& out, DerivationPtr const& deriv,
                                  IHypergraph<Arc> const& hg,
                                  DerivationStringOptions const& opts = DerivationStringOptions(kUnquoted),
                                  bool printWeight = false) {
  if (printWeight) out(deriv->weightForArc<Arc>())(' ');
  textFromDeriv(out, deriv, hg, opts);
}

template <class Arc>
Util::StringBuilder& bestPathString(Util::StringBuilder& out, IHypergraph<Arc> const& hg,
                                    BestPathOptions const& bestPathOpts = BestPathOptions(),
                                    DerivationStringOptions const& opts = DerivationStringOptions(kUnquoted),
                                    bool printWeight = false,
                                    char const* fallbackIfNoDerivation = "[no derivation exists]") {
  DerivationPtr deriv = bestPath(hg, bestPathOpts);
  if (!deriv)
    return out(fallbackIfNoDerivation);
  else {
    out.reserve(kReserveForBestPathString);
    appendBestPathStringForDeriv(out, deriv, hg, opts, printWeight);
    return out;
  }
}

template <class Arc>
std::string bestPathString(IHypergraph<Arc> const& hg, BestPathOptions const& bestPathOpts = BestPathOptions(),
                           DerivationStringOptions const& opts = DerivationStringOptions(kUnquoted),
                           bool printWeight = false,
                           char const* fallbackIfNoDerivation = "[no derivation exists]") {
  DerivationPtr deriv = bestPath(hg, bestPathOpts);
  if (!deriv)
    return fallbackIfNoDerivation;
  else {
    Util::StringBuilder out(kReserveForBestPathString);
    appendBestPathStringForDeriv(out, deriv, hg, opts, printWeight);
    return out.str();
  }
}

template <class Arc>
Syms& bestPathSymsAppend(Syms& syms, IHypergraph<Arc> const& hg,
                         BestPathOptions const& bestPathOpts = BestPathOptions(),
                         WhichSymbolOptions const& opts = WhichSymbolOptions()) {
  return symsFromDerivAppend(syms, bestPath(hg, bestPathOpts), hg, opts);
}

template <class Arc>
Syms& bestPathSymsAppend(Syms& syms, FeatureValue& cost, IHypergraph<Arc> const& hg,
                         BestPathOptions const& bestPathOpts = BestPathOptions(),
                         WhichSymbolOptions const& opts = WhichSymbolOptions()) {
  Derivation::DerivAndWeight<Arc> derivWeight(bestDerivWeight(hg, bestPathOpts));
  if (derivWeight.deriv) {
    cost = derivWeight.weight.value_;
    symsFromDerivAppend(syms, derivWeight.deriv, hg, opts);
  } else
    cost = std::numeric_limits<FeatureValue>::infinity();
  return syms;
}

template <class Arc>
Syms bestPathSyms(IHypergraph<Arc> const& hg, BestPathOptions const& bestPathOpts = BestPathOptions()) {
  Syms syms;
  bestPathSymsAppend(syms, hg, bestPathOpts);
  return syms;
}

template <class Arc>
std::string nbestString(IHypergraph<Arc> const& hg, unsigned nbest,
                        BestPathOutOptions opt = BestPathOutOptions()) {
  opt.nbest = nbest;
  std::ostringstream out;
  opt.out_nbest(out, hg);
  return out.str();
}

// Simplified interface (without best path options):
template <class Arc>
std::string bestPathString(IHypergraph<Arc> const& hg, DerivationStringOptions const& opts,
                           bool printWeight = false) {
  return bestPathString(hg, BestPathOptions(), opts, printWeight);
}

// Prints best weight and the associated string.
template <class Arc>
std::string bestPathStringWeighted(IHypergraph<Arc> const& hg,
                                   BestPathOptions const& bestPathOpts = BestPathOptions(),
                                   DerivationStringOptions const& opts = DerivationStringOptions()) {
  return bestPathString(hg, bestPathOpts, opts, true);
}

struct ToStringVisitor : public boost::static_visitor<> {
  ToStringVisitor(std::string& outString, FeatureValue& cost, SymsRewrite const& symsRewrite,
                  DerivationStringOptions const& stringOpt = DerivationStringOptions(kUnquoted, ""))
      : cost(cost), outString(outString), symsRewrite(symsRewrite), stringOpt(stringOpt) {}
  FeatureValue& cost;


  template <class T>
  void operator()(shared_ptr<T>& pData) const {
    (*this)(*pData);
  }

  void operator()(std::string const& str) const { outString = str; }

  /**
     1-best string only (no weight / features)
  */
  template <class Arc>
  void operator()(Hypergraph::IHypergraph<Arc> const& hg) const {
    using namespace Hypergraph;
    BestPathOptions bestPathOpts;
    Syms tokens;
    bestPathSymsAppend(tokens, cost, hg, bestPathOpts);
    IVocabulary* voc = hg.getVocabulary().get();
    if (symsRewrite) symsRewrite(tokens, voc);
    outString = textFromSyms(tokens, *voc, stringOpt);
  }

  std::string& outString;
  SymsRewrite symsRewrite;
  DerivationStringOptions stringOpt;
};


struct AcceptBestOptions : NbestPathOptions, Util::SpaceTokenOptions {
  template <class Config>
  void configure(Config& config) {
    Util::SpaceTokenOptions::configure(config);
    NbestPathOptions::configure(config);
    config.is("AcceptBestOptions");
    config("string-cost", &stringCost).init((FeatureValue)1)("cost to use if pipeline has string output");
    config("nbest-for-cost", &nbestForCost)
        .init(false)("use nbest rank (1, 2, ...) instead of path cost or string-cost");
    config("detok", &detok)
        .init(true)(
            "treat hg output as single string (detokenized using hg-string opts), or multiple strings if "
            "'space-token' splits them");
    config("hg-string", &hgString)
        .verbose()("if detok, use this to convert hypergraph path to single string");
  }
  AcceptBestOptions() {
    hgString.setChars();
    Config::inits(this);
  }
  FeatureValue stringCost;
  bool nbestForCost;
  DerivationStringOptions hgString;
  bool detok;
  FeatureValue costMaybeNbest(NbestId n, FeatureValue cost) const {
    return nbestForCost ? (FeatureValue)n : cost;
  }
};


/**
   replace escSpace token -> space token.
*/
struct SymStrReplace {
  bool enabled() const { return !escSpace.empty(); }
  std::string escSpace;
  SymStrReplace(std::string const& escSpace = Util::kEscSpace) : escSpace(escSpace) {}
  void operator()(Syms& syms, IVocabulary* vocab) const {
    Sym const esc = vocab->sym(escSpace, kTerminal);
    if (esc) std::replace(syms.begin(), syms.end(), esc, vocab->add(Util::kSpace, kTerminal));
  }
};

/**
   replace escSpace token -> space token, given vocabulary in advance
*/
struct SymReplace {
  bool enabled() const { return escSpace; }
  Sym escSpace;
  Sym space;
  SymReplace(IVocabulary* vocab, std::string const& escSpaceStr = Util::kEscSpace,
             std::string const& spaceStr = Util::kSpace)
      : escSpace(escSpaceStr.empty() ? NoSymbol : vocab->sym(escSpaceStr, kTerminal))
      , space(vocab->add(spaceStr, kTerminal)) {}
  void operator()(Syms& syms, IVocabulary*) const {
    if (escSpace) std::replace(syms.begin(), syms.end(), escSpace, space);
  }
};

struct SymReplaceCache {
  bool enabled() const { return !escSpaceStr.empty(); }
  IVocabulary* vocab;
  Sym escSpace, space;
  std::string escSpaceStr, spaceStr;

  SymReplaceCache(std::string const& escSpaceStr = Util::kEscSpace, IVocabulary* vocab = 0,
                  std::string const& spaceStr = Util::kSpace)
      : vocab(vocab), escSpaceStr(escSpaceStr), spaceStr(spaceStr) {
    initSyms();
  }

  void operator()(Syms& syms, IVocabulary* vocab_) const {
    const_cast<SymReplaceCache&>(*this).maybeInitSyms(vocab_);
    replace(syms);
  }

  void reset(IVocabulary* v) {
    vocab = v;
    initSyms();
  }

  void replace(Syms& syms) const {
    if (escSpace) std::replace(syms.begin(), syms.end(), escSpace, space);
  }

 private:
  void maybeInitSyms(IVocabulary* vocab_) {
    assert(vocab_);
    if (vocab_ != vocab) {
      vocab = vocab_;
      initSyms();
    }
  }
  void initSyms() {
    if (vocab && !escSpaceStr.empty()) {
      escSpace = vocab->sym(escSpaceStr, kTerminal);
      space = vocab->add(spaceStr, kTerminal);
    } else {
      escSpace = NoSymbol;
    }
  }
};

struct AcceptStringVisitor : public boost::static_visitor<> {
  /**
     \param *skipOriginalWord is a string that is skipped if it's an nbest
     output (the whole sentence is just that word), and must be stable against
     modifications to your vocabulary (i.e. you need to pass address of a *copy*
     of voc->str(id). if skipOriginalWord is null then the nbests are
     unfiltered
  */
  AcceptStringVisitor(std::string const* skipOriginalWord, Util::IAcceptString const& accept,
                      AcceptBestOptions const& options = AcceptBestOptions(),
                      SymsRewrite const& symsRewrite = identitySymsRewrite(), unsigned* nAccepted = 0)
      : skipOriginalWord(skipOriginalWord)
      , options(options)
      , accept(accept)
      , symsRewrite(symsRewrite)
      , voc()
      , skipOriginalSym(NoSymbol)
      , nAccepted(nAccepted) {}

  std::string const* skipOriginalWord;
  AcceptBestOptions const& options;
  SymsRewrite symsRewrite;
  Util::IAcceptString const& accept;
  mutable Sym skipOriginalSym;
  unsigned* nAccepted;

  void countAccepted() const {
    if (nAccepted) ++*nAccepted;
  }

  template <class T>
  void operator()(shared_ptr<T>& pData) const {
    (*this)(*pData);
  }

  /// for runPipeline string output.
  bool operator()(std::string const& str) const {
    if (skipOriginalWord && str == *skipOriginalWord) return false;
    accept(str);
    countAccepted();
    return accept.finishPhrase(options.costMaybeNbest(1, options.stringCost));
  }

  /// for runPipeline hg output. remember to catch EmptySetException so you can
  /// indicate that no paths were accepted
  template <class Arc>
  void operator()(Hypergraph::IHypergraph<Arc> const& hg) const {
    using namespace Hypergraph;
    voc = hg.getVocabulary().get();
    skipOriginalSym = skipOriginalWord ? voc->addTerminal(*skipOriginalWord) : NoSymbol;
    hgBase = &hg;
    SDL_DEBUG(Hypergraph.BestPathString, "RunPipeline visitor for Translitate etc. computing "
                                             << options.nbest << "-best for hg:\n " << hg);
    options.visit_nbest(hg, *this, Hypergraph::kThrowEmptySetException);
  }

  /// for visit_nbest for runPipeline hg output. to help compiler, we needed to template on Weight instead of
  /// Arc
  template <class Weight>
  bool operator()(Hypergraph::DerivationPtr const& deriv, Weight const& weight, NbestId n) const {
    assert(voc);
    assert(deriv);
    Syms tokens;  // TODO: unnecessary copy - visit
    symsFromDerivAppend(tokens, deriv, static_cast<IHypergraph<ArcTpl<Weight> > const&>(*hgBase),
                        options.hgString);
    if (symsRewrite) symsRewrite(tokens, voc);
    if (!options.detok) {
      if (!skipOriginalSym || !(tokens.size() == 1 && tokens[0] == skipOriginalSym))
        accept.phrase(tokens, *voc);
    } else {
      Util::StringBuilder wordBuf;  // we may have to concatenate several tokens into one
      std::string const& spaceToken = options.spaceToken;
      if (spaceToken.empty()) {
        textFromSyms(wordBuf, tokens, *voc, options.hgString);
        if (skipOriginalWord) {
          SDL_DEBUG(xmt.RunPipeline.runPipelineToStrings, "'" << wordBuf << "' vs original '" << *skipOriginalWord
                                                              << " equal=" << (wordBuf == *skipOriginalWord));
          if (wordBuf == *skipOriginalWord) return true;  // continue nbests
        }
        accept(wordBuf);  // single token result
      } else {
        // split into several tokens
        Sym const spaceSym = voc->add(spaceToken, kTerminal);
        Psym begin = tokens.begin(), end = tokens.end();
        Psym word0 = begin;
        unsigned nSpaceSep = 0;
        for (Psym i = begin; i < end; ++i) {
          if (*i == spaceSym)
            if (word0 < i) {  // nonempty token-slice
              ++nSpaceSep;
              wordBuf.clear();
              textFromSyms(wordBuf, SymSlice(word0, i), *voc, options.hgString);
              accept(wordBuf);
              word0 = i + 1;
            }
        }
        if (word0 < end) {
          wordBuf.clear();
          textFromSyms(wordBuf, SymSlice(word0, end), *voc, options.hgString);
          if (!nSpaceSep && skipOriginalWord) {
            SDL_DEBUG(xmt.RunPipeline.runPipelineToStrings,
                      "single word '" << wordBuf << "' vs original '" << *skipOriginalWord
                                      << " equal=" << (wordBuf == *skipOriginalWord));
            if (wordBuf == *skipOriginalWord) return true;  // continue nbests
          }
          accept(wordBuf);
        }
      }
    }
    FeatureValue const cost = deriv->weight<Weight>().value_;
    countAccepted();
    return accept.finishPhrase(options.costMaybeNbest(n + 1, cost));  // 1-best has index 0

    // TODO: might want a separate indicator for preferring e.g. 1-best to
    // original but not 2-best, or else use log(1/(n+1))
  }

 private:
  mutable IVocabulary* voc;
  mutable HypergraphBase const* hgBase;
};


}}

#endif
