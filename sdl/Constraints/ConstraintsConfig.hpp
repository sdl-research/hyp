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

    for IXMTServer, no dependency on Syms, etc. we use Constraints built from
    this in Pipeline/Request
*/

#ifndef CONSTRAINTSCONFIG_JG_2014_06_12_HPP
#define CONSTRAINTSCONFIG_JG_2014_06_12_HPP
#pragma once

#include <string>
#include <vector>
#include <sdl/Span.hpp>
#include <sdl/xmt/XmtSharedExport.hpp>
#include <sdl/SharedPtr.hpp>
#include <sdl/Util/Enum.hpp>
#include <sdl/Hypergraph/MixFeature.hpp>
#include <sdl/Features.hpp>

namespace sdl {

typedef std::string OutputConstraint;

inline bool enabled(OutputConstraint const& c) {
  return !c.empty();
}

typedef std::string SpaceSeparatedTokens;

inline std::string tokensUsage(std::string const& what) {
  return "if nonempty, these space-separated tokens are " + what;
}

inline std::string constraintUsage(std::string const& what, std::string const& scope = "entity") {
  return tokensUsage("used as " + what + " output for this " + scope + ", no matter what");
}

SDL_ENUM(KeepConstraints, 3, (Drop_Constraints, Keep_Final_Output_Constraints, Keep_Constraints));

inline bool keepsConstraints(KeepConstraints k, bool haveFinal) {
  return k == kKeep_Constraints || haveFinal && k == kKeep_Final_Output_Constraints;
}

SDL_ENUM(ConstraintType, 2, (NoSwap, JumpWall));

/// configurable stuff that's in the actual EntityConstraint struct (the full
/// set of user-facing options is available through the EntityConstraintConfig
/// is separate from that so we don't have to carry its redundant config-only
/// baggage)
struct EntityConstraintBase {
  /// as user specifies, half-open unicode codepoint spans. once converted to
  /// hg, these become closed hg state spans which are defined by the open/close
  /// xmt-block symbols (which may be recomputed from them as necessary)
  TokenSpan span;
  ConstraintType type;

  bool isWall() const {
    assert(type != kJumpWall || empty());
    return type == kJumpWall;
  }

  Position chars() const { return span.second - span.first; }
  Position outchars() const {
    return finalOutput ? finalOutput->size() : decoderOutput ? decoderOutput->size() : protect() ? chars() : 0;
  }

  void setSpan(Position left, Position right) {
    span.first = left;
    span.second = right;
  }

  EntityConstraintBase()
      : type(kNoSwap)
      , protectDecoder()
      , protectFinal()
      , finalOutputExactWhitespace()
      , isFormat()
      , spreadFormat() {}
  template <class Config>
  void configure(Config& config) {
    config(
        "entity constraint that applies to an input and forces translation to stay adjacent within the given "
        "span. entities may nest, but only the outermost's decoder-output or final-output will be effective");
    config.is("EntityConstraint");
    config("start", &span.first)
        .init(0)("unicode code point offset (0-based) for first character of constraint");
    config("end", &span.second)
        .init(0)(
            "unicode code point offset (0-based) for the character after the last affected by the "
            "constraint, i.e. the span is half-open [start, end)");
    config("protect-final", &protectFinal).init(false)("use for final-output the original input string");
    config("protect-decoder", &protectDecoder)
        .init(false)(
            "use for decoder-output the text for the span just prior to decoding (i.e. force decoder to "
            "identity translate whatever is in the span, but allow modules before decoder to modify it)");
    config("final-output-exact-whitespace", &finalOutputExactWhitespace)
        .init(false)(
            "make from final-output exactly one token (with internal whitespace exactly preserved) - not "
            "allowed in general but probably safe enough for final-output since it's reserved for the final "
            "stage in a pipeline");
    config("features", &features)("named feature:val for PickConstraints module");
    config("is-format", &isFormat)
        .init(false)(
            "if true, block any output that touches us but isn't contained by us (except for spread-format)");
    config("spread-format", &spreadFormat)
        .init(false)(
            "'spread-format: true' applies any format the constraint it touches to the whole output (by "
            "growing "
            "the 'is-format: true' blocks that touch it). for output-constraints w/ 'spread-format': false, "
            "we "
            "remove the constraint if it touches any format that doesn't contain it.");
    config("origin", &origin)("optional metadata: describe the source of this constraint");
    config("type", &type).verbose();
  }

  bool empty() const { return span.first == span.second; }
  bool valid() const { return span.first <= span.second; }

  typedef shared_ptr<SpaceSeparatedTokens> SpaceSeparatedTokensPtr;
  SpaceSeparatedTokensPtr decoderOutput, finalOutput;

  void setFinalOutput(std::string const& o) { finalOutput = sdl::make_shared<SpaceSeparatedTokens>(o); }

  std::string origin;
  NamedFeatureValues features;
  bool protectDecoder, protectFinal, finalOutputExactWhitespace, isFormat, spreadFormat;

  bool protect() const { return protectDecoder || protectFinal; }

  bool hasOutput() const { return decoderOutput || finalOutput || protect(); }

  friend inline std::ostream& operator<<(std::ostream& out, EntityConstraintBase const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const {
    out << "span=" << PrintSpan(span);
    if (type != kNoSwap) out << ' ' << type;
    if (decoderOutput) out << " decoder-output='" << *decoderOutput << "'";
    if (finalOutput) out << " final-output='" << *finalOutput << "'";
    if (!origin.empty()) out << " origin='" << origin << "'";
    if (isFormat) out << " is-format!";
    if (spreadFormat) out << " spread-format!";
    if (!features.empty()) printNamedFeatures(out << " features: ", features);
  }
};

struct XMT_API EntityConstraintConfig : EntityConstraintBase {
  //  Hypergraph::MixFeature<> optional; // set optional.weight to enable
  bool protect;
  EntityConstraintConfig() : protect() {}
  template <class Config>
  void configure(Config& config) {
    EntityConstraintBase::configure(config);
    /*
    config("optional", &optional).todo()
        ("set optional.weight to make decoder-output optional (output will contain unconstrained translations
    and decoder-output, with an optional.id=optional.weight feature to determine choice of best path");
    */
    config("protect", &protect).init(false)("if true, forces both protect-final and protect-decoder");
    config("decoder-output", &decoderOutput)
        (constraintUsage("decoder") + ". if final-output is given as well, then it wins in the end"
         "(but decoder-output will still affect other decisions made by the decoder via the target language model)");
    config("final-output", &finalOutput)(constraintUsage("final (capitalized, detokenized)"));
  }
  void validate();
  friend inline std::ostream& operator<<(std::ostream& out, EntityConstraintConfig const& self) {
    self.print(out);
    return out;
  }
};

inline char const* type_string_impl(EntityConstraintConfig const&) {
  return "EntityConstraint";
}

inline void validate(EntityConstraintConfig& c) {
  c.validate();
}

typedef std::vector<EntityConstraintConfig> EntityConstraintConfigs;

struct XMT_API ConstraintsConfig : EntityConstraintConfigs {
  typedef std::vector<Position> Walls;
  Walls walls;
  SpaceSeparatedTokens forceDecode, prefixForceDecode, lmPrefix;
  template <class Config>
  void configure(Config& config) {
    config("lm-prefix", &lmPrefix)(tokensUsage(
        "starting LM context that translation will be appended to (instead of the usual start-of-sentence "
        "context)"));
    config("force-decode", &forceDecode)(tokensUsage("the decoder's entire translation, if possible"));
    config("prefix-force-decode",
           &prefixForceDecode)(tokensUsage("the beginning of the decoder's translation, if possible"));
    config.is("Constraints");
    config("entity and decoding constraints (at most one of lm-prefix, force-decode, force-decode-prefix)");
    config("constraints", static_cast<EntityConstraintConfigs*>(this))(
        "each span constraint forms an <xmt-blockN> bracket pair, and optionally specifies decoder and final "
        "output for that span");
    config("walls", &walls)(
        "(unicode codepoint) indices that block PhraseDecoder reordering: everything up to this character "
        "must be "
        "translated before anything that comes after");
  }
  bool enabled() const {
    return !(empty() && lmPrefix.empty() && forceDecode.empty() && prefixForceDecode.empty() && walls.empty());
  }
  void validate();
  friend inline std::ostream& operator<<(std::ostream& out, ConstraintsConfig const& self) {
    self.print(out);
    return out;
  }
  void print(std::ostream& out) const;

  /// postcondition: !enabled()
  void clear();
};

inline void validate(ConstraintsConfig& c) {
  c.validate();
}


}

#endif
