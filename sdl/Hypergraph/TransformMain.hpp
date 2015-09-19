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

#define TRANSFORM_TO_STR(x) #x
#define TRANSFORM_NAME(x) TRANSFORM_TO_STR(x)

#define HYPERGRAPH_PREPEND_HYP(MAINCLASS) sdl::Hypergraph::Hyp##MAINCLASS
#define HYPERGRAPH_NAMED_MAIN(MAINCLASS) GRAEHL_NAMED_MAIN(MAINCLASS, HYPERGRAPH_PREPEND_HYP(MAINCLASS))

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

#include <graehl/shared/hex_int.hpp>
#include <graehl/shared/string_to.hpp>

#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/Override.hpp>
#include <sdl/Util/PrintRange.hpp>
#include <sdl/Util/Flag.hpp>
#include <sdl/Util/StringBuilder.hpp>
#include <graehl/shared/configure_named_bits.hpp>

namespace sdl {
namespace Hypergraph {

enum {
  kViterbiSemiring = 1 << 0,
#if SDL_TRANSFORM_MAIN_LOG_WEIGHT
  kLogSemiring = 1 << 1,
#endif
#if SDL_TRANSFORM_MAIN_EXPECTATION_WEIGHT
  kExpectationSemiring = 1 << 2,
#endif
  kFeatureSemiring = 1 << 3
};
struct SemiringNames {
  template <class Bits>
  static void bits(Bits& bits) {
    bits("viterbi", kViterbiSemiring);
#if SDL_TRANSFORM_MAIN_LOG_WEIGHT
    bits("log", kLogSemiring);
#endif
#if SDL_TRANSFORM_MAIN_EXPECTATION_WEIGHT
    bits("expectation", kExpectationSemiring);
#endif
    bits("feature", kFeatureSemiring);
  }
};
typedef graehl::named_bits<SemiringNames, int> Semirings;

struct TransformMainBase : HypergraphMainBase {
 protected:
  void init() {
    firstInputFileHasMultipleHgs = false;
    logname = "LW.TransformMain";
  }

 public:
  enum LineInputs { kNoLineInputs = 0, kLineInputs = 1 };
  enum BestOutput { kNoBestOutput = 0, kBestOutput = 1 };
  enum ReloadInputs { kResidentInputs = 0, kReloadInputs = 1 };

  typedef ExpectationWeight expectationSemiring;
  typedef LogWeightTpl<SdlFloat> logSemiring;
  typedef ViterbiWeightTpl<SdlFloat> viterbiSemiring;
  typedef FeatureWeight featureSemiring;

  Semirings arcType;
  boost::optional<PrintProperties> hg_properties;

  typedef int SemiringsSet;


  char const* logname;  // set this if you use LOG_TRANSFORM

  TransformMainBase(std::string const& n, std::string const& usage, std::string const& ver, bool multiple,
                    HypergraphMainOpt hmainOpt = HypergraphMainOpt(),
                    Semirings semirings = Semirings::allbits, BestOutput bestOutputs = kNoBestOutput,
                    Semirings defaultSemiring = kViterbiSemiring, bool nbestHypergraphDefault = false)
      : HypergraphMainBase(n, usage, ver, multiple, hmainOpt)
      , arcType(defaultSemiring)
      , allowedSemirings(semirings)
      , bestOutputs(bestOutputs == kBestOutput)
      , bestOptDesc(MaybeBestPathOutOptions::caption())
      , optBestOutputs(nbestHypergraphDefault) {
    init();
  }

  Semirings allowedSemirings;

  // these control the appearance of cmdline options that activate
  // alternative input options and output options (lines = paths, or
  // multiple input hgs)
  bool bestOutputs;
  MaybeBestPathOutOptions optBestOutputs;
  OD bestOptDesc;

  std::string semiringsUsage() const { return to_string_impl(allowedSemirings); }

  void only(Semirings x) { arcType = allowedSemirings = x; }
  void addDefault(Semirings x) { allowedSemirings |= (arcType = x); }

  typedef ArcTpl<viterbiSemiring> viterbiArc;
  typedef ArcTpl<logSemiring> logArc;
  typedef ArcTpl<expectationSemiring> expectationArc;
  typedef ArcTpl<featureSemiring> featureArc;

  NO_INIT_OR_ASSIGN_MEMBER(TransformMainBase)
  Util::Flag configureProperties;
  template <class Config>
  void configure(Config& c) {
    // doesn't defer to cmdline_main because cmdline_main registers itself as configurable(this) also
    bool ambig = allowedSemirings.count_set_bits() > 1;
    std::string qual(ambig ? "" : "no choice - must be ");

    c("arc-type", &arcType)('a')
        .self_init()("Hypergraph arc weight semiring type: " + qual + semiringsUsage())
        .verbose(ambig);
    // TODO: use SDL_ENUM for arcType
    if (configureProperties)
      c("properties", &hg_properties)('p')("optional hypergraph properties suggestion (in-arcs,out-arcs,etc)");
    if (firstInputFileHasMultipleHgs && multifile)
      c("reload", &reloadOnMultiple)(
          "for each of the inputs, re-read the rest of the transducers again each time (saves memory) if "
          "there are more than 2 inputs");
  }

  bool firstInputFileHasMultipleHgs;  // multiple inputs[0] lines or hgs
  bool reloadOnMultiple;  // for inputs 2...n, free then re-parse for each input (saves memory if n is large)

  void finish_configure_more() OVERRIDE {
    this->configurable(this);
    if (bestOutputs) {
      this->configurable(&optBestOutputs);
    } else
      optBestOutputs.disableNbest();
  }

  Properties properties_else(Properties p = kFsmOutProperties) const {
    return hg_properties ? withArcs(*hg_properties) : p;
  }

  virtual void validate_parameters_more() OVERRIDE {
    if (!(arcType.i & allowedSemirings.i))
      SDL_THROW_LOG(Hypergraph, InvalidInputException,
                    "semiring type " << arcType << " not allowed - use one of " << semiringsUsage());
  }
};

template <class CRTP>  // google CRTP if you're confused
struct TransformMain : TransformMainBase {
  // override as appropriate; these are fn rather than anon. enum so we can have the correct type
  // (BOOST_STATIC_CONSTANT might work too)
  static LineInputs lineInputs() { return kLineInputs; }
  static BestOutput bestOutput() { return kNoBestOutput; }
  static RandomSeed randomSeed() { return kNoRandomSeed; }
  static ReloadInputs reloadInputs() { return kResidentInputs; }
  static Semirings semirings() { return Semirings::allbits; }
  static Semirings defaultSemiring() { return kViterbiSemiring; }
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
  TransformMain(std::string const& n, std::string const& usage, std::string const& ver = "v1")
      : TransformMainBase(n, usage, ver, impl().has2(), HypergraphMainOpt(CRTP::randomSeed()),
                          CRTP::semirings(), CRTP::bestOutput(), CRTP::defaultSemiring(),
                          CRTP::nbestHypergraphDefault())
      , inputOptDesc(InputHypergraphs::caption()) {
    TransformMainBase::firstInputFileHasMultipleHgs = true;
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
    bool r;
    switch ((unsigned)arcType) {
      case kViterbiSemiring:
        r = runWeight<ViterbiWeightTpl<SdlFloat> >();
        break;
#if SDL_TRANSFORM_MAIN_LOG_WEIGHT
      case kLogSemiring:
        r = runWeight<LogWeightTpl<SdlFloat> >();
        break;
#endif
#if SDL_TRANSFORM_MAIN_EXPECTATION_WEIGHT
      case kExpectationSemiring:
        r = runWeight<ExpectationWeight>();
        break;
#endif
      case kFeatureSemiring:
        r = runWeight<FeatureWeight>();
        break;
      default:
        SDL_THROW_LOG(Hypergraph, InvalidInputException,
                      this->name() << ": unknown weight semiring type name: " << arcType);
    }

    if (!r) {
      warn("Aborted early (transform returned false).");
      return 1;
    } else
      return 0;
  }

  // separately named fns so just one needs to be overriden in CRTP

  // if has_*input_transform (can override directly):
  template <class Arc>
  bool inputTransformInplaceP(shared_ptr<IMutableHypergraph<Arc> >& h,
                              unsigned n) {  // from n=1...#input files.
    if (impl().has_inplace_input_transform) return impl().inputTransformInplace(*h, n);
    shared_ptr<IMutableHypergraph<Arc> > i = h;
    assert(n);
    MutableHypergraph<Arc>* m = new MutableHypergraph<Arc>(inputProperties(n - 1));
    h.reset(m);
    return impl().inputTransform((IHypergraph<Arc> const&)*i, m, n);
  }
  template <class Arc>
  bool inputTransformInplace(IMutableHypergraph<Arc>& i, unsigned n) {  // from n=1...#input files.
    SDL_THROW_LOG(Hypergraph, UnimplementedException,
                  "unimplemented: has_inplace_input_transform -> inputTransformInplace for " << this->name());
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
  bool transform1InplaceP(shared_ptr<IMutableHypergraph<Arc> >& h) {
    if (impl().has_inplace_transform1) return impl().transform1Inplace(*h);
    shared_ptr<IMutableHypergraph<Arc> > i = h;
    MutableHypergraph<Arc>* m = new MutableHypergraph<Arc>(outputProperties());
    h.reset(m);
    return impl().transform1((IHypergraph<Arc> const&)*i, m);
  }
  // if has_inplace_transform1
  template <class Arc>
  bool transform1Inplace(IMutableHypergraph<Arc>& h) {
    SDL_THROW_LOG(Hypergraph, UnimplementedException, "unimplemented transform1Inplace for " << this->name());
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
  bool transform2InplaceP(shared_ptr<IMutableHypergraph<Arc> >& io, shared_ptr<IMutableHypergraph<Arc> >& i2) {
    if (impl().has_inplace_transform2) return impl().transform2Inplace(*io, *i2);
    shared_ptr<IMutableHypergraph<Arc> > i = io;
    io.reset(new MutableHypergraph<Arc>(outputProperties()));
    io->setVocabulary(this->vocab());
    return impl().transform2pp(i, i2, io.get());
  }

  // if has_inplace_transform2
  template <class Arc>
  bool transform2Inplace(IMutableHypergraph<Arc>& io, IMutableHypergraph<Arc>& i2) {
    return impl().transform2Inplacec(io, i2);
  }

  template <class Arc>
  bool transform2Inplacec(IMutableHypergraph<Arc>& io, IHypergraph<Arc> const& i2) {
    SDL_THROW_LOG(Hypergraph, UnimplementedException,
                  "unimplemented: has_inplace_input_transform2 -> transform2Inplace for " << this->name());
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

  bool outEvery() const { return false; }

  char const* transform2sep() const { return " * "; }

  Properties inputProperties(unsigned input) const { return implProperties(input + 1); }

  Properties outputProperties() const { return implProperties(0); }

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

    Hps cascade;  // parallel to inputs (input is first). the rest are lazily loaded from inputs if needed.
    std::string const& appname;

    Cascade(TransformMain& main) : main(main), cascade(main.inputs.size()), appname(main.name()) {}

    std::string const& name() const { return appname; }

    CRTP& impl() { return main.impl(); }
    CRTP const& impl() const { return main.impl(); }

    // cascade[0] is an in/out hg.
    bool transformInput(unsigned inputLine, bool reload, bool free) {
      Hp olast;  // o: recent output
      std::string olast_name;
      if (!cascade.size()) return false;
      olast = cascade[0];
      olast_name = main.inputs[0].name;
      std::ostringstream o_name;
      o_name << olast_name;
      Util::Sep oname_sep = main.impl().transform2sep();
      for (unsigned input = 1, ninput = (unsigned)cascade.size(); input < ninput; ++input) {
        Hp& h = cascade[input];  // h: input hg (that we may free)
        Util::Input in(main.inputs[input]);
        if (!h) {
          h.reset(new H(main.inputProperties(input)));
          SDL_TRACE(Hypergraph.transformInput, "reading hypergraph with properties "
                                                   << PrintProperties(h->properties()));
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
            bool inputOk = impl().inputTransformInplaceP(h, input);
            if (!inputOk) return false;
            SDL_TRACE(Hypergraph.TransformMain, "input: " << input << ", post-transform");
          }
        }
        // now h holds input, olast holds recent output
        if (impl().has2()) {
          std::string const& aname = o_name.str();
          HTRANSFORM2_MSG(6, "a@" << (void*)olast.get() << ":\n" << *olast << "\nb@" << (void*)h.get()
                                  << "=:\n" << *h,
                          aname, in.name);
          o_name << oname_sep << in.name;
          bool ok = impl().transform2InplaceP(olast, h);
          olast_name = o_name.str();
          HTRANSFORM2_MSG(6, "result=" << olast_name << " (success=" << ok << "):\n" << *olast, aname, in.name);
          if (!ok) return false;
        }
        if (input == 1) cascade[0].reset();
        if (free) h.reset();
      }
      if (impl().has1()) {
        bool finalok = impl().transform1InplaceP(olast);
        if (!finalok) return false;
      }
      if (impl().printFinal()) {
        main.optBestOutputs.output(main.out(), *olast, graehl::utos(inputLine));
      }
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
    optInputs.multiple = !firstInputFileHasMultipleHgs;
    optInputs.setIn(this->input());

    bool t3 = inputs.size() > 2 && impl().has2();
    bool reload = (t3 && reloadOnMultiple);
    bool free = optInputs.single() || reload;

    Cascade<Weight> cascade(*this);

    typedef shared_ptr<IMutableHypergraph<Arc> > Hp;
    Hp& h = cascade.cascade[0];
    bool allok = true;
    unsigned ninputs = 0;
    for (;;) {
      h.reset(new MutableHypergraph<Arc>(inputProperties(0)));
      SDL_TRACE(TransformMain, "reading hypergraph with properties " << PrintProperties(h->properties()));
      h->setVocabulary(this->vocab());
      if (!optInputs.nextHypergraph(h.get())) break;
      ++ninputs;
      if (impl().hasInputTransform()) {
        unsigned input = 0;
        SDL_TRACE(Hypergraph.TransformMain, "input: " << input << ", pre-transform");
        bool inputOk = impl().inputTransformInplaceP(h, input);
        if (!inputOk) return false;
        SDL_TRACE(Hypergraph.TransformMain, "input: " << input << ", post-transform");
      }
      if (!cascade.transformInput((unsigned)optInputs.lineno, reload, free)) allok = false;
    }
    return allok && ninputs;
  }

  /// i=0 means output (i=1 means input+output if in-place)
  Properties properties(int i) const { return kFsmOutProperties; }

  /// can override this to *not* favor cmdline props for some hgs
  Properties properties_default(int i) const {
    return hg_properties ? withArcs(*hg_properties) : implProperties(i);
  }

 private:
  Properties implProperties(int i) const { return withArcs(impl().properties(i)); }
};


}}

#endif
