/**





 */

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <string>



#include <boost/optional.hpp>






























namespace po = boost::program_options;





std::string getLibraryName(std::string const& stem) {
#if defined _WIN64 || defined _WIN32
  return stem + "-shared.dll";
#elif defined __APPLE__
  return "lib" + stem + "-shared.dylib";
#else
  return "lib" + stem + "-shared.so";
#endif
}

int main(int ac, char* av[]) {










  try {
    po::options_description generic("Command line options");

    std::string configFile;
    std::string modelName, searchSpaceSectionName, optimizeSectionName;
    bool help;
    bool checkIntersection;
    bool checkGradients;

    bool testMode;





















    po::options_description options;
    options.add(generic);

    po::variables_map varMap;

    po::store(parsed, varMap);


    po::notify(varMap);

    if (varMap.count("search-dir")) {
      findFile().setDirs(varMap["search-dir"].as<std::vector<std::string> >());
    }



    if (help) {

      Optimization::OptimizationProcedureOptions config;

                << "with the following options:\n\n";
      Config::showHelp(std::cout, &config);
      return 0;
    }




    // get a ResourceManager)
    ConfigNode configNode = Config::loadConfig(configFile);

    std::string weightsPath;
    for (unsigned i = 0, N = (unsigned)unrecognizedArgs.size(); i < N; ++i) {
      std::string const& arg = unrecognizedArgs[i];
      if (arg.size() > 3 && arg[0] == '-' && arg[1] == '-') {
        std::string::size_type sep = arg.find('=', 2);
        std::string::const_iterator a0 = arg.begin();
        std::string yamlPath, yamlVal;
        if (sep != std::string::npos) {
          yamlPath = std::string(a0 + 2, a0 + sep);
          yamlVal = std::string(a0 + sep + 1, arg.end());
        } else if (i + 1 < N) {
          ++i;
          yamlPath = std::string(a0 + 2, arg.end());
          yamlVal = unrecognizedArgs[i];
        } else

        if (!yamlVal.empty()) {

          configNode = Config::overrideConfig(configNode, yamlPath, yamlVal);
        }




      }
    }


    bool showEffective = true;
    int verbosity = 1;

    Optimization::OptimizationProcedureOptions optProcConfig;
    ConfigNode optimizeNode = configNode[optimizeSectionName];




                                               << optimizeSectionName << "' in " << configFile);
    }










    optProcConfig.testMode = testMode;




    std::string libName(getLibraryName(varMap["model"].as<std::string>()));
    Util::LoadedLib myLib = NULL;
    if (!(myLib = Util::loadLib((libName).c_str()))) {

    }

    void* loadFct = NULL;
    if (!(loadFct = Util::loadProc(myLib, "load"))) {

    }

    ConfigNode node = configNode[searchSpaceSectionName];
    if (!node) {

                                               << searchSpaceSectionName << "' in " << configFile);
    }



    typedef Optimization::ICreateSearchSpace<Optimization::Arc> SearchSpace;
    shared_ptr<SearchSpace> searchSpace(reinterpret_cast<SearchSpace*>(obj));


    Optimization::OptimizationProcedure optimizer(searchSpace, optProcConfig);



      Util::Performance performance("Optimize.optimize");
      optimizer.setCheckIntersectionNonEmpty(checkIntersection);
      optimizer.setCheckGradients(checkGradients);
      optimizer.optimize();
    }



    return -1;
  }



  return 0;
}
