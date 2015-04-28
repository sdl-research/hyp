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
#include <sdl/Util/IcuHeaders.hpp>
#include <sdl/Hypergraph/UseOpenFst.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <sdl/Hypergraph/Arc.hpp>
#include <sdl/Hypergraph/ArcParserFct.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/Weight.hpp>
#include <sdl/Hypergraph/ToReplaceFst.hpp>

#ifndef NDEBUG
#include <sdl/Hypergraph/HypergraphWriter.hpp>
#endif

#include <sdl/IVocabulary.hpp>
#include <sdl/Vocabulary/HelperFunctions.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/InitLogger.hpp>

#include <sdl/SharedPtr.hpp>
#include <sdl/Util/Forall.hpp>
#include <sdl/Util/ProgramOptions.hpp>

#include <fst/fst.h>
#include <fst/replace.h>
#include <fst/rmepsilon.h>
#include <fst/determinize.h>
#include <fst/minimize.h>
#include <fst/project.h>
#include <sdl/Util/Input.hpp>
#include <sdl/Util/Output.hpp>
#include <sdl/Hypergraph/HypergraphMain.hpp>
#include <sdl/Util/Delete.hpp>


namespace sdl {
namespace Hypergraph {

struct HypToReplaceFst {
  static int run_main(int ac, char* av[]) {
    namespace po = boost::program_options;
    try {
      po::options_description generic("Allowed options");
      sdl::AddOption opt(generic);
      opt("config-file", po::value<std::string>(), "config file name");
      opt("symbols", po::value<std::string>(), "FST symbol table to read and use (optional)");
      opt("stem", po::value<std::string>(), "stem for output files");
      opt("help", "produce help message");

      po::options_description hidden("Hidden options");
      hidden.add_options()("input-file", po::value<std::string>(), "input file");

      po::options_description cmdline_options;
      cmdline_options.add(generic).add(hidden);

      po::options_description config_file_options;
      config_file_options.add(generic).add(hidden);

      po::positional_options_description p;
      p.add("input-file", -1);

      po::variables_map vm;
      store(po::command_line_parser(ac, av).options(cmdline_options).positional(p).run(), vm);

      if (vm.count("config-file")) {
        Util::Input ifs(vm["config-file"].as<std::string>());
        store(parse_config_file(*ifs, config_file_options), vm);
        notify(vm);
      }

      if (vm.count("help")) {
        std::cout << generic << "\n\n";
        std::cout << "Convert hypergraph to an OpenFst ReplaceFst object\n";
        return EXIT_FAILURE;
      }

#if HAVE_OPENFST
      bool userProvidedSymbolTable = false;
      Util::AutoDelete<fst::SymbolTable> syms;
      if (vm.count("symbols")) {
        if (vm.count("stem")) {
          throw std::runtime_error("Do not provide both --stem and --symbols.");
        }
        std::string filename = vm["symbols"].as<std::string>();
        syms.reset(fst::SymbolTable::ReadText(filename));
        userProvidedSymbolTable = true;
      } else {
        syms.reset(new fst::SymbolTable(""));
      }

      std::string stem;
      if (vm.count("stem")) {
        if (vm.count("symbols")) {
          throw std::runtime_error("Do not provide both --stem and --symbols.");
        }
        stem = vm["stem"].as<std::string>();
      } else {
        stem = "replace";
      }

      std::string file;
      if (vm.count("input-file")) {
        file = vm["input-file"].as<std::string>();
      }

      // TODO: set level etc. on cmdline
      sdl::Util::initLogger("HypToReplaceFst", sdl::Util::logLevel(sdl::Util::kLogDebug));

      typedef ViterbiWeightTpl<float> Weight;
      typedef ArcTpl<Weight> Arc;

      IVocabularyPtr pVoc = Vocabulary::createDefaultVocab();

      Util::Input in(file);

      MutableHypergraph<Arc> hg;
      hg.setVocabulary(pVoc);
      parseText(*in, file, &hg);
      assert(hg.checkValid());

      // Could also use LogArc and ".log-fst"
      typedef fst::StdArc FArc;
      const std::string fstName = stem + ".std-fst";

      // Convert to OpenFst ReplaceFst:
      Util::AutoDelete<fst::Fst<FArc> > result(sdl::Hypergraph::toReplaceFst<FArc>(hg, syms));

      fst::VectorFst<FArc> minimized;

      typedef fst::MutableFst<FArc> IFst;
      // TODO: most of these explicit IFst casts are overcautious because of unfamiliarity w/ OpenFst API
      {
        // Copy to non-lazy OpenFst object and write to disk
        fst::VectorFst<FArc> nonlazy;
        std::cerr << "Expanding Hypergraph (will not terminate if cyclic) ... ";
        nonlazy = *result;  // copy
        std::cerr << "Done.\n";

        std::cerr << "Optimizing ... ";
        fst::Project((IFst*)&nonlazy, fst::PROJECT_OUTPUT);
#if 0
        fst::RmEpsilon((IFst*)&nonlazy);
        fst::DeterminizeFst<FArc>((IFst&)nonlazy, &minimized);
#else
        fst::Determinize(fst::RmEpsilonFst<FArc>(nonlazy), &minimized);
#endif
      }
      // TODO: can we result.release() now?
      fst::Minimize((IFst*)&minimized);
      std::cerr << "Done.\n";

      fst::FstWriteOptions opts;

      if (userProvidedSymbolTable) {
        minimized.Write(std::cout, opts);
      } else {
        std::cerr << "Writing " << fstName << '\n';
        Util::Output out(fstName);
        minimized.Write(*out, opts);

        const std::string symsName = stem + ".syms";
        std::cerr << "Writing " << symsName << '\n';
        syms->WriteText(symsName);
      }

#else
      std::cerr << "ERROR: Recompile with OpenFst!\n";
      return EXIT_FAILURE;
#endif
    } catch (std::exception& e) {
      std::cerr << e.what() << '\n';
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  }
};


}}

HYPERGRAPH_NAMED_MAIN(ToReplaceFst)
