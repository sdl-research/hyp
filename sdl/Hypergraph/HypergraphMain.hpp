// Copyright 2014 SDL plc
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

#include <sdl/Util/ProgramOptions.hpp>
#include <sdl/LexicalCast.hpp>
#include <sdl/graehl/shared/random.hpp>
#include <sdl/graehl/shared/cmdline_main.hpp>
#include <sdl/graehl/shared/assign_traits.hpp>
#include <sdl/Util/InitLogger.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Vocabulary/HelperFunctions.hpp>
#include <sdl/Util/FindFile.hpp>
#include <sdl/Util/Locale.hpp>
#include <sdl/graehl/shared/named_main.hpp>
#include <sdl/Util/Input.hpp>

#define HYPERGRAPH_PREPEND_HYP(MAINCLASS) sdl::Hypergraph::Hyp##MAINCLASS
#define HYPERGRAPH_NAMED_MAIN(MAINCLASS) GRAEHL_NAMED_MAIN(MAINCLASS, HYPERGRAPH_PREPEND_HYP(MAINCLASS))

namespace sdl {
namespace Hypergraph {

/// named options for HypergraphMainBase
struct HypergraphMainOpt : Util::InputsOptions {
  enum MultiFile { kNoMultiFile = 0, kMultiFile = 1 };
  enum RandomSeed { kNoRandomSeed = 0, kRandomSeed = 1 };
  Util::InitLoggerOptions logOpt;
  RandomSeed random;
  bool initlogger;
  bool searchDirsOpt;

  HypergraphMainOpt() { init(); }

  void init() {
    InputsOptions::positional = true;
    InputsOptions::enabled = true;
    InputsOptions::multifile = false;
    InputsOptions::min_ins = 1;
    logOpt.setRemoveAppenders(true).setOverwriteFile(true);
    searchDirsOpt = false;
    initlogger = true;
    random = kNoRandomSeed;
    helpOptions = true;
  }

  HypergraphMainOpt(MultiFile multiFile, RandomSeed randomSeed)  //=kNoRandomSeed
  {
    init();
    multifile = multiFile == kMultiFile;
    random = randomSeed;
  }

  static inline HypergraphMainOpt noInput(HypergraphMainOpt opt = HypergraphMainOpt()) {
    opt.enabled = false;
    return opt;
  }
  bool helpOptions;
  static inline HypergraphMainOpt noHelp(HypergraphMainOpt opt = HypergraphMainOpt()) {
    opt.helpOptions = false;
    return opt;
  }
};

struct HypergraphMainBase : graehl::main, HypergraphMainOpt {
  Util::Inputs inputs;
  NO_INIT_OR_ASSIGN_MEMBER(HypergraphMainBase)
  Util::Input const& first_input() const { return inputs.first_input(); }
  std::istream& in() const { return *first_input(); }
  Util::SearchDirs searchDirs;
  bool initlogger;
  IVocabularyPtr const& vocab() const {
    if (!pVoc) pVoc = Vocabulary::createDefaultVocab();
    return pVoc;
  }

  HypergraphMainBase(std::string const& n, std::string const& usage, std::string const& ver = "v1",
                     HypergraphMainOpt const& opt = HypergraphMainOpt())
      : graehl::main(n, usage, ver, false, opt.random, false)
      , HypergraphMainOpt(opt)
      , initlogger(opt.initlogger) {
    this->opt.add_help = opt.helpOptions;
    this->opt.add_log_file = false;  // using log4cxx instead
  }

  void multipleInputs(int maxin = 0) {
    max_ins = maxin;
    InputsOptions::enabled = true;
    multifile = true;
  }


 private:
  mutable IVocabularyPtr pVoc;

 public:
  virtual void finish_configure_more() {}

  virtual void finish_configure_extra() {
    inputs.opt = *this;
    if (inputs.opt.enabled) this->configurable(&inputs);
    if (initlogger) this->configurable(&logOpt);
    if (searchDirsOpt) this->configurable(&searchDirs);
    finish_configure_more();
  }

  void initlog() {
    if (initlogger) {
      Util::InitLoggerOptions& opts = logOpt;
      opts.setConsoleUnlessFile();
      opts.verbose = this->verbose;
      Util::initLogger(name(), opts);
    }
  }

  virtual void validate_parameters_extra() {
    initlog();
    inputs.validate();
    validate_parameters_more();
  }

  virtual void validate_parameters_more() {}
};


}}

#endif
