/** \file

    inherit 'struct YourMain : Hypergraph::HypergraphMainBase' and call
    'this->configurable(&config)' in constructor, then implement 'void run() {}'

    then

    INT_MAIN(sdl::YourMain);

*/

#ifndef HYP__HYPERGRAPHMAIN_HPP
#define HYP__HYPERGRAPHMAIN_HPP
#pragma once

#ifndef HG_MAIN_USE_CONFIGURE
#define HG_MAIN_USE_CONFIGURE 1
#endif

#define GRAEHL_CMDLINE_MAIN_USE_CONFIGURE HG_MAIN_USE_CONFIGURE
#define TRANSFORM_TO_STR(x) #x
#define TRANSFORM_NAME(x) TRANSFORM_TO_STR(x)

#include <sdl/LexicalCast.hpp>
#include <sdl/graehl/shared/random.hpp>
#include <sdl/graehl/shared/cmdline_main.hpp>
#include <sdl/graehl/shared/assign_traits.hpp>
#include <sdl/Util/InitLogger.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Vocabulary/HelperFunctions.hpp>
#include <sdl/Util/FindFile.hpp>
#include <sdl/Util/Locale.hpp>

sdl::Util::DefaultLocaleFastCout initCout;

namespace sdl {
namespace Hypergraph {

/// named options for HypergraphMainBase
struct HypergraphMainOpt {
  enum MultiFile { kNoMultiFile = 0, kMultiFile = 1 };
  enum RandomSeed { kNoRandomSeed = 0, kRandomSeed = 1 };
  bool initlogger;
  Util::InitLoggerOptions logOpt;
  bool input;
  bool searchDirsOpt;
  MultiFile multifile;
  RandomSeed random;

  HypergraphMainOpt() { init(); }

  void init() {
    logOpt.setRemoveAppenders(true).setOverwriteFile(true);
    searchDirsOpt = false;
    initlogger = true;
    input = true;
    random = kNoRandomSeed;
    multifile = kNoMultiFile;
    helpOptions = true;
  }

  HypergraphMainOpt(MultiFile multiFile, RandomSeed randomSeed)  //=kNoRandomSeed
  {
    init();
    multifile = multiFile;
    random = randomSeed;
  }

  static inline HypergraphMainOpt noInput(HypergraphMainOpt opt = HypergraphMainOpt()) {
    opt.input = false;
    return opt;
  }
  bool helpOptions;
  static inline HypergraphMainOpt noHelp(HypergraphMainOpt opt = HypergraphMainOpt()) {
    opt.helpOptions = false;
    return opt;
  }
};

struct HypergraphMainBase : graehl::main, HypergraphMainOpt {
  sdl::Util::DefaultLocaleFastCout* forceLinkInitCout;

  NO_INIT_OR_ASSIGN_MEMBER(HypergraphMainBase)
  Util::SearchDirs searchDirs;
  bool initlogger;
  IVocabularyPtr const& vocab() const {
    if (!pVoc) pVoc = Vocabulary::createDefaultVocab();
    return pVoc;
  }

  HypergraphMainBase(std::string const& n, std::string const& usage, std::string const& ver,
                     HypergraphMainOpt const& opt = HypergraphMainOpt())
      : graehl::main(n, usage, ver, opt.multifile, opt.random, opt.input)
      , HypergraphMainOpt(opt)
      , initlogger(opt.initlogger) {
    forceLinkInitCout = &initCout;
    this->opt.add_help = opt.helpOptions;
    init();
  }

 private:
  mutable IVocabularyPtr pVoc;

  void init() {
    HypergraphMainOpt::init();
    opt.add_log_file = false;  // using log4cxx instead
  }

 public:
  virtual void finish_configure_more() {}

  virtual void finish_configure_extra() {
    if (initlogger) this->configurable(&logOpt);
    if (searchDirsOpt) this->configurable(&searchDirs);
    finish_configure_more();
  }

  void initlog() {
    if (initlogger) {
      Util::InitLoggerOptions& opts = logOpt;
      opts.setConsoleUnlessFile();
      opts.setVerbose(this->verbose);
      Util::initLogger(name(), opts);
    }
  }

  virtual void validate_parameters_extra() {
    initlog();
    validate_parameters_more();
  }

  virtual void validate_parameters_more() {}
};


}}

#endif
