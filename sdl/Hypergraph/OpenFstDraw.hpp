// Copyright 2014 SDL plc
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/** \file

    print openfst as graphviz/dot.
*/

#ifndef OPENFSTDRAW_LW201213_HPP
#define OPENFSTDRAW_LW201213_HPP
#pragma once

#include <sdl/Hypergraph/ToOpenFst.hpp>
#include <sdl/Hypergraph/HypergraphDrawer.hpp>
#include <sdl/graehl/shared/fileargs.hpp>

#include <sdl/Hypergraph/UseOpenFst.hpp>
#if HAVE_OPENFST
#include <sdl/Hypergraph/ToReplaceFst.hpp>
#include <fst/script/draw.h>
#include <fst/script/print.h>
#include <fst/script/prune.h>
#endif

#include <sdl/Util/Constants.hpp>
#include <sdl/Hypergraph/WeightUtil.hpp>
#include <sdl/graehl/shared/is_null.hpp>

namespace sdl {
namespace Hypergraph {

// TODO: for openfst, don't create axiom states except start.
struct DrawOptions {
  static char const* caption() { return "Draw"; }
  // openfst options:
  std::string title;
  std::string dest;
  bool accep;
  float width;
  float height;
  bool portrait;
  bool vertical;
  float ranksep;
  float nodesep;
  int fontsize;
  int precision;
  bool show_weight_one;

  std::ostream* o;
  // our options:
  std::string terminator;
  bool replaceFst;
  graehl::ostream_arg openFstPrintFile;
  bool stateNames;
  double pruneBeam;
#define FOR_DRAW_OPTIONS_INT(f)                                                                      \
  f(accep) f(width) f(height) f(portrait) f(vertical) f(ranksep) f(nodesep) f(fontsize) f(precision) \
      f(show_weight_one)
#define FOR_DRAW_OPTIONS(f) f(title) f(dest) FOR_DRAW_OPTIONS_INT(f)
#define FOR_OUR_DRAW_OPTIONS(f) f(replaceFst) f(openFstPrintFile) f(terminator) f(stateNames) f(pruneBeam)
  DrawOptions() {
    replaceFst = false;
    terminator = "\n";
    title = "Fsm";
    dest = "";  // ??     // drawn FST destination name - presumably from final state?
    stateNames = true;
#define FOR_DRAW_OPTIONS_SET_0(n) n = 0;
    FOR_DRAW_OPTIONS_INT(FOR_DRAW_OPTIONS_SET_0);
    // TODO: nonzero defaults
    accep = true;
    show_weight_one = false;
    width = 8.5;
    height = 11;
    portrait = true;
    fontsize = 10;
    precision = 3;
    pruneBeam = FloatLimits<double>::posInfinity;
    o = 0;  // don't draw, but maybe openFstPrintFile
  }

  graehl::ostream_arg oarg;
  template <class Config>
  void configure(Config& config) {
#if HAVE_OPENFST
    config.is("OpenFst Draw/Print");
#define FOR_DRAW_OPTIONS_ADD_OPTION(n) config(#n, &n)("openfst draw " #n);
    FOR_DRAW_OPTIONS(FOR_DRAW_OPTIONS_ADD_OPTION);

    config("replaceFst", &replaceFst)("use ToReplaceFst for fsm");
    config("openFstPrintFile",
           &openFstPrintFile)("also openfst print to this file (avoid drawing with -o=-0)");
    config("terminator",
           &terminator)("this string after every transducer drawn or printed (newline default)");
    config("stateNames", &stateNames)("name states for non-replaceFst openfst translation");
    config("pruneBeam", &pruneBeam)("unless nan, openfst prune with this beam before print/draw");
    //      FOR_OUR_DRAW_OPTIONS(FOR_DRAW_OPTIONS_ADD_OPTION)
    ;
#endif
  }

  void validate() {
    if (replaceFst) {
      SDL_WARN(Hypergraph.OpenFstDraw,
               "drawing a replaceFst will infinite-loop unless the Hg is finite (no cycles)");
    }
  }

  template <class A>
  void drawHg(IHypergraph<A> const& h) const {
    if (o) drawHypergraph(*o, h);
  }

#if HAVE_OPENFST
  fst::script::FstDrawerArgs drawArgs(std::ostream& o, fst::script::FstClass const& fstc,
                                      fst::SymbolTable const& isyms, fst::SymbolTable const& osyms,
                                      fst::SymbolTable const* pStateNames = 0) const {
    return fst::script::FstDrawerArgs(fstc, &isyms, &osyms, pStateNames, accep, title, width, height,
                                      portrait, vertical, ranksep, nodesep, fontsize, precision,
                                      show_weight_one, &o, dest);
  }

  fst::script::FstPrinterArgs printArgs(std::ostream& o, fst::script::FstClass const& fstc,
                                        fst::SymbolTable const& isyms, fst::SymbolTable const& osyms,
                                        fst::SymbolTable const* pStateNames = 0) const {
    return fst::script::FstPrinterArgs(fstc, &isyms, &osyms, pStateNames, accep, show_weight_one, &o, dest);
  }

  template <class FstArc>
  void print(std::ostream& o, fst::script::FstClass const& fstc, fst::SymbolTable const& isyms,
             fst::SymbolTable const& osyms, fst::SymbolTable const* pStateNames = 0) const {
    fst::script::FstPrinterArgs a = printArgs(o, fstc, isyms, osyms, pStateNames);
    fst::script::PrintFst<FstArc>(&a);
  }

  template <class FstArc>
  void drawClass(fst::script::FstClass const& fstc, fst::SymbolTable const& syms,
                 fst::SymbolTable const* pStateNames = 0) const {
    if (o) {
      fst::script::FstDrawerArgs fa = drawArgs(*o, fstc, syms, syms, pStateNames);
      fst::script::DrawFst<FstArc>(&fa);
      *o << terminator;
    }
    if (openFstPrintFile) {
      std::ostream& of = openFstPrintFile.stream();
      print<FstArc>(of, fstc, syms, syms, pStateNames);
      of << terminator;
    }
  }

  template <class FstArc>
  void drawClass(fst::script::FstClass const& fstc, fst::SymbolTable const* pStateNames = 0) const {
    drawClass<FstArc>(fstc, *fstc.InputSymbols(), pStateNames);
  }

  template <class FstArc>
  void drawFst(fst::Fst<FstArc>& fst, fst::SymbolTable const* pStateNames = 0) const {
    fst::script::FstClass fstc(&fst);
    drawClass<FstArc>(fstc, pStateNames);
  }

  template <class Arc>
  void drawReplaceFst(IHypergraph<Arc> const& h) const {
    // IVocabularySymbolTable syms(h.getVocabulary());
    typedef typename Arc::Weight Weight;
    typedef fst::ArcTpl<Weight> FstArc;

    fst::SymbolTable fsyms("drawReplaceFst");
    fst::ReplaceFst<FstArc>* r = toReplaceFst<FstArc>(h, &fsyms);
    drawFst(*r, &fsyms);
    delete r;
  }
  template <class A>
  void drawFsmOpenFst(IHypergraph<A> const& h) const {
    typedef typename A::Weight W;
#if TO_OPENFST_GETVALUE
    typedef fst::StdArc FstArc;
#else
    typedef fst::ArcTpl<W> FstArc;
#endif
    typedef typename FstArc::Weight FW;
    typedef ToOpenFst<A, FstArc> T;
    T t(h, stateNames);
    if (pruneBeam != FloatLimits<double>::posInfinity) { Prune(&t.fst, FW(pruneBeam)); }
    drawFst<FstArc>(t.getFst(), t.stateNames());
  }
  template <class A>
  void draw(IHypergraph<A> const& h) const {
    if (replaceFst)
      drawReplaceFst(h);  // FIXME: may be able to handle some non-recursive CFG too
    else if (h.isFsm()) {
      drawFsmOpenFst(h);
    } else
      drawHg(h);
  }
#else
  template <class A>
  void draw(IHypergraph<A> const& h) const {
    drawHg(h);
  }
#endif
};


}}

#endif
