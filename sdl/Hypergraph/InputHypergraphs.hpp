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

    options for hypergraphs from file(s).
*/

#ifndef HYP__HG_INPUT_HYPERGRAPHS_HPP
#define HYP__HG_INPUT_HYPERGRAPHS_HPP
#pragma once

#include <sdl/Hypergraph/LineToHypergraph.hpp>
#include <sdl/Hypergraph/ArcParserFct.hpp>
#include <graehl/shared/fileargs.hpp>
#include <sdl/Config/Init.hpp>
#include <sdl/Util/Nfc.hpp>
#include <sdl/Util/LineOptions.hpp>
#include <sdl/Util/Input.hpp>

/*

  This enables optional line-by-line string input, where each line is interpreted as a sequence of labels and converted into a little hypergraph, right? It's a good workaround for now, since we don't have the one-hypergraph-per-line format yet. Util::Once we have that, we can just use the existing HgConvertStrings binary to do this conversion, and the user can pipe his text lines through that.
*/

namespace sdl {
namespace Hypergraph {

struct InputHypergraphs : LineToHypergraph
{
  bool multiple;
  bool lines;
  void setLine()
  {
    lines = true;
  }
  void setLines()
  {
    multiple = true;
    setLine();
  }

  Util::Input in;
  bool inArg;
  InputHypergraphs()
      : inArg()
      , lineno()
  {
    defaults();
  }

  std::size_t lineno;

  void defaults() {
    lines = false; // allows override by StatisticalTokenizer bin
    Config::inits(this);
  }

  static char const* caption()
  {
    return "Input Hypergraphs: optionally convert sentences (token strings) one per line to input hypergraphs";
  }
  static inline std::string usage() {
    return ParseTokensOptions::usage()
        +" optionally converting input to single-string hypergraphs";
  }
  template <class Config>
  void configure(Config &c)
  {
    LineToHypergraph::configure(c);
    c.is("Input Hypergraphs");
    c(caption());
    if (inArg)
      c("input-hypergraphs-file", &in).init(Util::stdinInput())
          ("input lines or hypergraph from this file");
    c("multiple-input-hgs", &multiple).init(true)
        ("allow multiple consecutive inputs");
    c("lines", &lines).defaulted()
        ("plain text input lines converted to hg (instead of a single hypergraph file)");
  }
  void setIn(Util::InputStream const& newIn)
  {
    in.init(newIn);
    lineno = 0;
  }
  /**
      parse the next input into hg. if lines, the input is a sequence of
      tokens (otherwise it's a text-format HG); if multiple-input-hgs, then the
      entire in stream is consumed line by line (TODO: support some convention
      for parsing a sequence of text-format HGs - none exists yet).
     *

      \return true if there was another hg, false if there are no more
   */
  template <class A>
  bool nextHypergraph(IMutableHypergraph<A> *phg)
  {
    IMutableHypergraph<A> &hg = *phg;
    assert(hg.getVocabulary());
    if (!in) return false;
    ++lineno;
    if (lines) {
      std::string line;
      if (!getlineNormalized(*in, line))
        return false;
      LineToHypergraph::toHypergraph(line, phg, lineno);
      return true;
    } else {
      if (lineno>1)
        return false;
      hg.clear();
      parseText(*in, in.name, &hg, nfc);
      return true;
    }
  }
  bool single() const
  {
    return !multiple || !lines; //TODO: change to && once we support sequence of consecutive hgs on input
  }
};


}}

#endif
