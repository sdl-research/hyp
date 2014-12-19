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

    int main(...) boilerplate for "o(f(f(g(a), g(b),...g(n))))" with
   command-line specified --arc-type

   see ../src/HypDeterminize.cpp (simple unary)
   and ../src/HypCompose.cpp (binary)
   and ../src/HypInvert.cpp (an in-place transform)
   and ../src/HypEmpty.cpp (in->print stats only)

   optional multi-line strings of tokens/chars input

   optional best-path output (instead of single hg)

   TODO: make instantiated best-path-printing for each ArcTpl<Weight> so it's not compiled for each cmdline
   prog

   TODO: separator for multiple HG output to same file '\0' or other? and for multiple HG input from same
   file?

   Transform.hpp provides caching of weight-specific setup across invocations
   (but doesn't handle command line); since we so far run for one arc/weight
   type per process in command-line context, there's no need for that complexity

   for building blah-main.cpp:

   #define HG_TRANSFORM_MAIN
   #include <sdl/Hypergraph/TransformMain.hpp>

   struct blah : sdl::Hypergraph::TransformMain<transform> {...};

   INT_MAIN(blah)

*/

#ifndef HYP__HG_TRANSFORMMAIN_HPP
#define HYP__HG_TRANSFORMMAIN_HPP
#pragma once

#include <sdl/Hypergraph/HypergraphMain.hpp>
#include <sdl/LexicalCast.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/WeightsFwdDecls.hpp>
#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/ExpectationWeight.hpp>
#include <sdl/Hypergraph/FeatureWeight.hpp>
#include <sdl/Hypergraph/WeightUtil.hpp>
#include <sdl/Hypergraph/HypergraphWriter.hpp>
#include <sdl/Hypergraph/ArcParserFct.hpp>
#include <sdl/Hypergraph/InputHypergraphs.hpp>
#include <sdl/Hypergraph/BestPath.hpp>

#include <sdl/graehl/shared/bit_arithmetic.hpp>
#include <sdl/graehl/shared/hex_int.hpp>
#include <sdl/graehl/shared/word_spacer.hpp>

#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Override.hpp>
#include <sdl/Util/PrintRange.hpp>

#define INFO_TRANSFORM(x) LOG_INFO_NAMESTR(this->logname, x)
#define DEBUG_TRANSFORM(x) LOG_DEBUG_NAMESTR(this->logname, #x << " = " << x)
#define WARN_TRANSFORM(x) LOG_WARN_NAMESTR(this->logname, x)
#define ERROR_TRANSFORM(x) LOG_ERROR_NAMESTR(this->logname, x)

namespace sdl {
namespace Hypergraph {

struct TransformMainBase : HypergraphMainBase {
 protected:
  void init() {
    multiInputs = false;
    logname = "LW.TransformMain";
  }

 public:
  enum LineInputs { kNoLineInputs = 0, kLineInputs = 1 };
  enum BestOutput { kNoBestOutput = 0, kBestOutput = 1 };
  enum ReloadInputs { kResidentInputs = 0, kReloadInputs = 1 };

  typedef ExpectationWeight expectationSemiring;
  typedef LogWeightTpl<float> logSemiring;
  typedef ViterbiWeightTpl<float> viterbiSemiring;
  typedef FeatureWeight featureSemiring;

  std::string arcType;
  graehl::hex_int<Properties> default_properties;

  enum { viterbi = 1, log = 2, expectation = 4, feature = 8 };
  enum { kAllSemirings = (viterbi | log | expectation | feature) };

  typedef int semirings_type;
  char const* logname;  // set this if you use LOG_TRANSFORM

  TransformMainBase(std::string const& n, std::string const& usage, std::string const& ver,
                    HypergraphMainOpt hmainOpt = HypergraphMainOpt(), int semirings = kAllSemirings,
                    BestOutput bestOutputs = kNoBestOutput, int defaultSemiring = viterbi,
                    bool nbestHypergraphDefault = false)
      : HypergraphMainBase(n, usage, ver, hmainOpt)
      , arcType(semiringsList(defaultSemiring))
      , default_properties((kDefaultProperties | kStoreOutArcs))
      , allowedSemirings(semirings)
      , bestOutputs(bestOutputs == kBestOutput)
      , bestOptDesc(MaybeBestPathOutOptions::caption())
      , optBestOutputs(nbestHypergraphDefault) {
    init();
  }

  semirings_type allowedSemirings;

  // these control the appearance of cmdline options that activate
  // alternative input options and output options (lines = paths, or
  // multiple input hgs)
  bool bestOutputs;
  MaybeBestPathOutOptions optBestOutputs;
  OD bestOptDesc;

  std::string semiringsUsage() const { return semiringsList(allowedSemirings); }

  static std::string semiringsList(semirings_type x) {
    std::ostringstream o;
    graehl::word_spacer sp('|');
    if (x & viterbi) o << sp << "viterbi";
    if (x & log) o << sp << "log";
    if (x & expectation) o << sp << "expectation";
    if (x & feature) o << sp << "feature";
    return o.str();
  }

  static semirings_type semiringFor(std::string const& s) {
    if (s == "viterbi") return viterbi;
    if (s == "log") return log;
    if (s == "expectation") return expectation;
    if (s == "feature") return feature;
    if (s == "VHG") return viterbi;
    if (s == "LHG") return log;
    if (s == "EHG") return expectation;
    if (s == "FHG") return feature;
    SDL_THROW_LOG(Hypergraph, InvalidInputException, "unknown semiring type " << s);
  }

  void only_viterbi() {
    allowedSemirings = 0;
    default_viterbi();
  }
  void default_viterbi() {
    arcType = "viterbi";
    allowedSemirings |= viterbi;
  }
  void only_log() {
    allowedSemirings = 0;
    default_log();
  }
  void default_log() {
    arcType = "log";
    allowedSemirings |= log;
  }
  void only_expectation() {
    allowedSemirings = 0;
    default_expectation();
  }
  void default_expectation() {
    arcType = "expectation";
    allowedSemirings |= expectation;
  }
  void only_feature() {
    allowedSemirings = 0;
    default_feature();
  }
  void default_feature() {
    arcType = "feature";
    allowedSemirings |= feature;
  }

  typedef ArcTpl<viterbiSemiring> viterbiArc;
  typedef ArcTpl<logSemiring> logArc;
  typedef ArcTpl<expectationSemiring> expectationArc;
  typedef ArcTpl<featureSemiring> featureArc;

  NO_INIT_OR_ASSIGN_MEMBER(TransformMainBase)

  template <class Config>
  void configure(Config& c) {
    // doesn't defer to cmdline_main because cmdline_main registers itself as configurable(this) also
    bool ambig = graehl::count_set_bits(allowedSemirings) > 1;
    std::string qual(ambig ? "" : "no choice - must be ");
    c.is("standard hypergraph options");

    c("arc-type", &arcType)('a')("Weight semiring: " + qual + semiringsUsage()).verbose(ambig);
    // TODO: use SDL_ENUM for arcType

    c("properties", &default_properties)('p')("Hypergraph property bit vector suggestion if nonzero").init(0);
    if (multiInputs && this->multifile == HypergraphMainOpt::kMultiFile)
      c("reload", &reloadOnMultiple)(
          "for each of the inputs, re-read the rest of the transducers again each time (saves memory) if "
          "there are more than 2 inputs");
  }

  bool multiInputs;  // multiple inputs[0] lines or hgs
  bool reloadOnMultiple;  // for inputs 2...n, free then re-parse for each input (saves memory if n is large)

  void finish_configure_more() OVERRIDE {
    this->configurable(this);
    if (bestOutputs)
      this->configurable(&optBestOutputs);
    else
      optBestOutputs.disableNbest();
  }

  Properties properties(int i) const {  // i=0 means output
    return default_properties ? (Properties)default_properties : kStoreInArcs;
  }

  virtual void validate_parameters_more() OVERRIDE {
    semirings_type sr = semiringFor(arcType);
    if (!(sr & allowedSemirings))
      SDL_THROW_LOG(Hypergraph, InvalidInputException, "semiring type " + arcType + " not allowed - use one of "
                                              << semiringsUsage());
  }
};

template <class CRTP>  // google CRTP if you're confused
struct TransformMain : TransformMainBase {
  // override as appropriate; these are fn rather than anon. enum so we can have the correct type
  // (BOOST_STATIC_CONSTANT might work too)

  //* \return kMultiFile for binary transforms e.g. compose */
  MultiFile multiFile() const { return impl().has2() ? kMultiFile : kNoMultiFile; }
  static LineInputs lineInputs() { return kLineInputs; }
  static BestOutput bestOutput() { return kNoBestOutput; }
  static RandomSeed randomSeed() { return kNoRandomSeed; }
  static ReloadInputs reloadInputs() { return kResidentInputs; }
  static int semirings() { return kAllSemirings; }
  static int defaultSemiring() { return TransformMainBase::viterbi; }
  static bool nbestHypergraphDefault() { return true; }
  bool printFinal() const { return true; }
  enum {
    has_input_transform = false,
    has_inplace_input_transform = false,
    has_inplace_transform1 = false,
    has_transform1 = true,
    has_inplace_transform2 = false,
    has_transform2 = false,
    out_every_default = false
  };
  // TODO: make out_every produce output with filename in.name() in case of
  // transform1, or a+b+...c for transform2 (meaning out_file option is ignored
  // if runtime option is on)

  NO_INIT_OR_ASSIGN_MEMBER(TransformMain)

  void setLines() { optInputs.setLines(); }
  void setChars() {
    optInputs.setLine();
    optInputs.setChars();
  }
  TransformMain(std::string const& n, std::string const& usage, std::string const& ver)
      : TransformMainBase(n, usage, ver, HypergraphMainOpt(multiFile(), CRTP::randomSeed()),
                          CRTP::semirings(), CRTP::bestOutput(), CRTP::defaultSemiring(),
                          CRTP::nbestHypergraphDefault())
      , inputOptDesc(InputHypergraphs::caption()) {
    HypergraphMainOpt::multifile = multiFile();
    TransformMainBase::multiInputs = (CRTP::lineInputs() == kLineInputs);
    reloadOnMultiple = (CRTP::reloadInputs() == kReloadInputs);
    configuredInputs = false;
  }

  InputHypergraphs optInputs;
  bool configuredInputs;
  void configureInputs() {
    if (configuredInputs) return;
    this->configurable(&optInputs);
    configuredInputs = true;
  }

  void before_finish_configure() OVERRIDE { configureInputs(); }

  OD inputOptDesc;
  CRTP& impl() { return *static_cast<CRTP*>(this); }
  CRTP const& impl() const { return *static_cast<CRTP const*>(this); }

  int run_exit() OVERRIDE {
    std::string const& w = this->arcType;
    bool r;
    if (w == "viterbi") r = runWeight<viterbiSemiring>();
#if SDL_TRANSFORM_MAIN_LOG_WEIGHT
    else if (w == "log")
      r = runWeight<logSemiring>();
#endif
#if SDL_TRANSFORM_MAIN_EXPECTATION_WEIGHT
    else if (w == "expectation")
      r = runWeight<expectationSemiring>();
#endif
    else if (w == "feature")
      r = runWeight<featureSemiring>();
    else
      SDL_THROW_LOG(Hypergraph, InvalidInputException, name() + ": unknown weight semiring type name: " + w);
    if (!r) {
      warn("Aborted early (transform returned false).");
      return 1;
    } else
      return 0;
  }

  // separately named fns so just one needs to be overriden in CRTP

  // if has_*input_transform (can override directly):
  template <class Arc>
  bool inputTransformInPlaceP(shared_ptr<IMutableHypergraph<Arc> >& h,
                              unsigned n) {  // from n=1...#input files.
    if (impl().has_inplace_input_transform) return impl().inputTransformInPlace(*h, n);
    shared_ptr<IMutableHypergraph<Arc> > i = h;
    MutableHypergraph<Arc>* m = new MutableHypergraph<Arc>(impl().properties(n));
    h.reset(m);
    return impl().inputTransform((IHypergraph<Arc> const&)*i, m, n);
  }
  template <class Arc>
  bool inputTransformInPlace(IMutableHypergraph<Arc>& i, unsigned n) {  // from n=1...#input files.
    SDL_THROW_LOG(Hypergraph, UnimplementedException,
                  "unimplemented: has_inplace_input_transform -> inputTransformInPlace for " << this->name());
    return true;
  }
  template <class Arc>
  bool inputTransform(IHypergraph<Arc> const& i, IMutableHypergraph<Arc>* o,
                      unsigned n) {  // from n=1...#input files.
    SDL_THROW_LOG(Hypergraph, UnimplementedException,
                  "unimplemented: has_input_transform -> inputTransform for " << this->name());
    return true;
  }

  // if has_*transform1, override either this or one of below
  template <class Arc>
  bool transform1InPlaceP(shared_ptr<IMutableHypergraph<Arc> >& h) {
    if (impl().has_inplace_transform1) return impl().transform1InPlace(*h);
    shared_ptr<IMutableHypergraph<Arc> > i = h;
    MutableHypergraph<Arc>* m = new MutableHypergraph<Arc>(impl().properties(0));
    h.reset(m);
    return impl().transform1((IHypergraph<Arc> const&)*i, m);
  }
  // if has_inplace_transform1
  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc>& h) {
    SDL_THROW_LOG(Hypergraph, UnimplementedException, "unimplemented transform1InPlace for " << this->name());
  }
  // else just has_transform1
  // most unary transform will ONLY override this
  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc>* o) {
    SDL_THROW_LOG(Hypergraph, InvalidInputException, "unimplemented transform1 for " << this->name());
    return true;
  }

  // if has2().
  // First ptr is taken as result for reduction/fold of N Hgs to
  // 1. please don't meaningfully modify i2.
  template <class Arc>
  bool transform2InPlaceP(shared_ptr<IMutableHypergraph<Arc> >& io, shared_ptr<IMutableHypergraph<Arc> >& i2) {
    if (impl().has_inplace_transform2) return impl().transform2InPlace(*io, *i2);
    shared_ptr<IMutableHypergraph<Arc> > i = io;
    io.reset(new MutableHypergraph<Arc>(impl().properties(0)));
    io->setVocabulary(this->vocab());
    return impl().transform2pp(i, i2, io.get());
  }

  // if has_inplace_transform2
  template <class Arc>
  bool transform2InPlace(IMutableHypergraph<Arc>& io, IMutableHypergraph<Arc>& i2) {
    return impl().transform2InPlacec(io, i2);
  }

  template <class Arc>
  bool transform2InPlacec(IMutableHypergraph<Arc>& io, IHypergraph<Arc> const& i2) {
    SDL_THROW_LOG(Hypergraph, UnimplementedException,
                  "unimplemented: has_inplace_input_transform2 -> transform2InPlace for " << this->name());
    return true;
  }

  // else:

  template <class Arc>
  bool transform2pp(shared_ptr<IMutableHypergraph<Arc> >& i, shared_ptr<IMutableHypergraph<Arc> >& i2,
                    IMutableHypergraph<Arc>* o) {
    // can update 1st or 2nd arg to point to new Hg - although you should be satisfied if you specified
    // properties() already
    return impl().transform2mm(*i, *i2, o);
  }

  template <class Arc>
  bool transform2mm(IMutableHypergraph<Arc>& i, IMutableHypergraph<Arc>& i2, IMutableHypergraph<Arc>* o) {
    return impl().transform2mc(i, (IHypergraph<Arc> const&)i2, o);
  }

  template <class Arc>
  bool transform2mc(IMutableHypergraph<Arc>& i, IHypergraph<Arc> const& i2, IMutableHypergraph<Arc>* o) {
    return impl().transform2((IHypergraph<Arc> const&)i, i2, o);
  }

  // most will override this:
  template <class Arc>
  bool transform2(IHypergraph<Arc> const& i, IHypergraph<Arc> const& i2, IMutableHypergraph<Arc>* o) {
    SDL_THROW_LOG(Hypergraph, UnimplementedException, "unimplemented: has_input_transform2 -> transform2 for "
                                                      << this->name());
    return true;
  }

  bool has2() const { return impl().has_inplace_transform2 || impl().has_transform2; }
  bool has1() const { return impl().has_inplace_transform1 || impl().has_transform1; }

  bool hasInputTransform() const { return impl().has_inplace_input_transform || impl().has_input_transform; }

  bool outEvery() const {
    assert(!impl().out_every_default());
    return false;
  }

  char const* transform2sep() const { return " * "; }

  Properties inputProperties(unsigned input) const { return withArcs(impl().properties(input + 1)); }

  Properties outputProperties() const { return withArcs(impl().properties(0)); }

// #define HTRANSFORM2_MSG(v, msg, aname, bname) HYPERGRAPH_DBG_MSG(v, "Transforming by " << this->name() <<
// "(a, b) with a=" << aname << " b=" << bname << " " << msg << '\n')
#define HTRANSFORM2_MSG(v, msg, aname, bname)                                                         \
  SDL_TRACE(Hypergraph.TransformMain, "Transforming by " << this->name() << "(a, b) with a=" << aname \
                                                         << " b=" << bname << " " << msg);

  template <class Weight>
  struct Cascade {
    typedef ArcTpl<Weight> Arc;
    typedef MutableHypergraph<Arc> H;
    typedef shared_ptr<IMutableHypergraph<Arc> > Hp;
    typedef std::vector<Hp> Hps;
    TransformMain& main;

    istream_args const& ins;
    Hps cascade;  // parallel to ins (input is first). the rest are lazily loaded from ins if needed.
    std::string const& appname;

    Cascade(TransformMain& main) : main(main), ins(main.ins), cascade(ins.size()), appname(main.name()) {}

    std::string const& name() const { return appname; }

    CRTP& impl() { return main.impl(); }
    CRTP const& impl() const { return main.impl(); }

    // cascade[0] is an in/out hg.
    bool transformInput(unsigned inputLine, bool reload, bool free) {
      Hp olast;  // h: recent input. o: recent output
      std::string olast_name;
      if (!cascade.size()) return false;
      olast = cascade[0];
      olast_name = ins[0].name;
      std::ostringstream o_name;
      o_name << olast_name;
      Util::Sep oname_sep = main.impl().transform2sep();
      if (this->ins.empty()) SDL_THROW_LOG(Hypergraph, InvalidInputException, "no input files");
      for (unsigned input = 1, ninput = (unsigned)cascade.size(); input < ninput; ++input) {
        Hp& h = cascade[input];  // h: input hg (that we may free)
        Util::Input in(ins[input]);
        if (!h) {
          h.reset(new H(main.inputProperties(input)));
          if (!*in)
            SDL_THROW_LOG(Hypergraph.TransformMain, FileException, "invalid input hg file: " << in.name);
          h->setVocabulary(main.vocab());
          if (inputLine > 1 && reload)
            if (!(in->seekg(0)))
              SDL_THROW_LOG(Hypergraph.TransformMain, FileException,
                            "Couldn't seek back to beginning with --reload and >2 inputs");
          parseText(*in, in.name, h.get());
          if (impl().hasInputTransform()) {
            SDL_TRACE(Hypergraph.TransformMain, "input: " << input << ", pre-transform");
            bool inputOk = impl().inputTransformInPlaceP(h, input);
            if (!inputOk) return false;
            SDL_TRACE(Hypergraph.TransformMain, "input: " << input << ", post-transform");
          }
        }
        // now h holds input, olast holds recent output
        if (impl().has2()) {
          std::string aname = o_name.str();
          HTRANSFORM2_MSG(6, "a@" << (void*)olast.get() << ":\n" << *olast << "\nb@" << (void*)h.get()
                                  << "=:\n" << *h,
                          aname, in.name);
          o_name << oname_sep << in.name;
          bool ok = impl().transform2InPlaceP(olast, h);
          olast_name = o_name.str();
          HTRANSFORM2_MSG(6, "result=" << olast_name << " (success=" << ok << "):\n" << *olast, aname,
                          in.name);
          if (!ok) return false;
        }
        if (input == 1) cascade[0].reset();
        if (free) h.reset();
      }
      if (impl().has1()) {
        bool finalok = impl().transform1InPlaceP(olast);
        if (!finalok) return false;
      }
      if (impl().printFinal()) { main.optBestOutputs.output(main.out(), *olast, graehl::utos(inputLine)); }
      return true;
    }
  };

  /// Override if desired: it's guaranteed that impl().prepare will be
  /// called for exactly one Arc type - the same as in all the
  /// transform* calls.  this may e.g. create a TransformHolder or
  /// other shared_ptr<void> that's used across all inputs.
  template <class Arc>
  void prepare() {}

  template <class Weight>
  bool runWeight() {
    typedef ArcTpl<Weight> Arc;
    impl().template prepare<Arc>();
    if (!multiInputs) {
      optInputs.lines = false;
      optInputs.multiple = false;
    }
    optInputs.setIn(first_input());

    bool t3 = ins.size() > 2 && impl().has2();
    bool reload = (t3 && reloadOnMultiple);
    bool free = optInputs.single() || reload;

    Cascade<Weight> c(*this);

    typedef shared_ptr<IMutableHypergraph<Arc> > Hp;
    Hp& h = c.cascade[0];
    bool allok = true;
    for (;;) {
      h.reset(new MutableHypergraph<Arc>(inputProperties(0)));
      h->setVocabulary(this->vocab());
      if (!optInputs.nextHypergraph(h.get())) break;
      if (impl().hasInputTransform()) {
        unsigned input = 0;
        SDL_TRACE(Hypergraph.TransformMain, "input: " << input << ", pre-transform");
        bool inputOk = impl().inputTransformInPlaceP(h, input);
        if (!inputOk) return false;
        SDL_TRACE(Hypergraph.TransformMain, "input: " << input << ", post-transform");
      }
      if (!c.transformInput((unsigned)optInputs.lineno, reload, free)) allok = false;
    }
    return allok;
  }
};


}}

#endif
