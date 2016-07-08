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

#include <sdl/Vocabulary/HelperFunctions.hpp>
#include <sdl/Util/FindFile.hpp>
#include <sdl/Util/InitLogger.hpp>
#include <sdl/Util/Input.hpp>
#include <sdl/Util/Locale.hpp>
#include <sdl/Util/ProgramOptions.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/LexicalCast.hpp>
#include <graehl/shared/assign_traits.hpp>
#include <graehl/shared/cmdline_main.hpp>
#include <graehl/shared/named_main.hpp>
#include <graehl/shared/random.hpp>

namespace sdl {
namespace Hypergraph {

/// named options for HypergraphMainBase
struct HypergraphMainOpt {
  enum RandomSeed { kNoRandomSeed = 0, kRandomSeed = 1 };
  Util::InitLoggerOptions logOpt;
  RandomSeed random;
  bool initlogger;
  bool searchDirsOpt;

  HypergraphMainOpt() { init(); }

  void init() {
    logOpt.setRemoveAppenders(true).setOverwriteFile(true);
    searchDirsOpt = false;
    initlogger = true;
    random = kNoRandomSeed;
    helpOptions = true;
  }

  HypergraphMainOpt(RandomSeed randomSeed)  //=kNoRandomSeed
  {
    init();
    random = randomSeed;
  }

  bool helpOptions;
  static inline HypergraphMainOpt noHelp(HypergraphMainOpt opt = HypergraphMainOpt()) {
    opt.helpOptions = false;
    return opt;
  }
};

struct HypergraphMainBase : graehl::main, HypergraphMainOpt, Util::Inputs {
  NO_INIT_OR_ASSIGN_MEMBER(HypergraphMainBase)
  Util::SearchDirs searchDirs;
  bool initlogger;
  IVocabularyPtr const& vocab() const {
    if (!pVoc) pVoc = Vocabulary::createDefaultVocab();
    return pVoc;
  }

  void init() { graehl::main::init(); }

  HypergraphMainBase(std::string const& n, std::string const& usage, std::string const& ver = "v1",
                     bool multiple = true, HypergraphMainOpt const& opt = HypergraphMainOpt())
      : Util::Inputs(multiple)
      , graehl::main(n, usage, ver, false, opt.random, false)
      , HypergraphMainOpt(opt)
      , initlogger(opt.initlogger) {
    this->opt.no_ins();
    this->opt.add_verbose = true;
    this->opt.add_help = opt.helpOptions;
    this->opt.add_log_file = false;  // using log4cxx instead
    this->opt.add_quiet = false;
  }
  void multipleInputs(int maxin = 0) {
    assert(!configured);
    max_inputs = maxin;
    inputEnabled = true;
    multifile = true;
  }
  void disableDefaultCmdlineOptions() {
    inputEnabled = false;
    verbose = 0;
    opt.disable();
  }

 private:
  Util::Flag configured, validated;
  mutable IVocabularyPtr pVoc;

 public:
  virtual void finish_configure_more() {}

  virtual void finish_configure_extra() {
    if (inputEnabled) this->configurable((Util::Inputs*)this);
    if (initlogger) this->configurable(&logOpt);
    if (searchDirsOpt) this->configurable(&searchDirs);
    finish_configure_more();
    configured = true;
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
    Util::Inputs::validate();
    validate_parameters_more();
  }

  virtual void validate_parameters_more() {}
};


}}

#endif
