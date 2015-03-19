/** \file

    options for printing best paths.
*/

#ifndef DERIVATIONSTRINGOPTIONS_JG_2014_07_29_HPP
#define DERIVATIONSTRINGOPTIONS_JG_2014_07_29_HPP
#pragma once

#include <sdl/Hypergraph/PrintOptions.hpp>
#include <sdl/Hypergraph/Label.hpp>
#include <sdl/Config/Init.hpp>
#include <sdl/Vocabulary/SpecialSymbols.hpp>
#include <sdl/Util/Enum.hpp>

namespace sdl { namespace Hypergraph {

SDL_ENUM(WhichDerivationStates, 3, (WholeTree, LeafOnly, NonLeafOnly))

struct WhichSymbolOptions {
  void defaults() {
    Config::inits(this);
  }
  WhichSymbolOptions() { defaults(); }
  WhichSymbolOptions(bool) { }
  bool shows(Sym sym) const {
    if (sym.isLexical()) return true;
    if (types == kLexical_Only) return false;
    if (epsilon == kSkip_Epsilon && sym == EPSILON::ID) return false;
    if (block == kSkip_Block && Vocabulary::isBlockOrSubstituteSymbol(sym)) return false;
    return true;
  }
  template <class Config>
  void configure(Config &config, bool hide = false) {
    config("skip-epsilon", &epsilon).verbose()
        ("backward compatability for 'epsilon' option");
    config("lexical-only", &types).init(kAllSymbolTypes).verbose()
        ("backward compatability for 'types' option");

    config("types", &types).verbose(hide).init(kAllSymbolTypes)
        ("all-symbols includes <xmt-blockN> <tok> etc; lexical-only implies 'epsilon: skip-epsilon' and 'block: skip-block')");
    config("epsilon", &epsilon).verbose(hide).init(kSkip_Epsilon)
        ("skip or keep epsilon (<eps>)");
    config("block", &block).verbose(hide).init(kSkip_Block)
        ("skip or keep block/entity symbols (<xmt-blockN> </xmt-block> <xmt-entityN>)");
    config("label-type", &labelType).verbose(hide).init(kOutput).verbose()
        ("Write Input or Output labels? Note: Output is the default because it is correct in the typical cases - wherever no explicit output label is present it is implicitly identical to the input label.");
    config("leaf-only", &leafOnly).verbose(hide).init(kLeafOnly)
        ("(false, or whole-tree, means print all derivation states' nodes' labels) (true, or leaf-only; means don't print internal (e.g. nonterminal) labels)");
  }
  SymbolTypes types;
  SymbolEpsilonSkip epsilon;
  SymbolBlockSkip block;
  LabelType labelType;
  WhichDerivationStates leafOnly;
};

struct DerivationStringOptions : WhichSymbolOptions {

  void defaults() {
    Config::inits(this);
  }

  void setQuoted() {
    quote = kQuoted;
    defaults();
  }

  /// for eg ->zh (no spaces between chi tokens)
  void setChars() {
    quote = kUnquoted;
    space = "";
    defaults();
  }

  void setWords() {
    quoteSpaceDefaults();
    defaults();
  }

  DerivationStringOptions()
  {
    quoteSpaceDefaults();
    defaults();
  }

  DerivationStringOptions(bool) { quoteSpaceDefaults(); }

  void quoteSpaceDefaults() {
    quote = kUnquoted;
    space = " ";
  }

  DerivationStringOptions(SymbolQuotation quoted) {
    quote = quoted;
    space = " ";
    defaults();
  }

  DerivationStringOptions(SymbolQuotation quoted, std::string const& space_)
  {
    space = space_;
    quote = quoted;
    defaults();
  }

  SymbolQuotation quote;
  std::string space;
  template <class Config>
  void configure(Config &config, bool hide = false) {
    WhichSymbolOptions::configure(config);
    config.is("hypergraph nbest -> string");
    config("quote", &quote).verbose(hide).self_init()
        ("quote/escape symbols - if false (unquoted)O, you can't distinguish special (e.g. <eps>) from lexical symbols");
    config("space", &space).verbose(hide).self_init()
        ("separate tokens with this (always a space between weight and tokens)");
  }
};


}}

#endif
