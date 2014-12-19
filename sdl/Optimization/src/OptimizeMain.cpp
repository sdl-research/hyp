










































































































    po::options_description options;
    options.add(generic);

    po::variables_map varMap;

    po::store(parsed, varMap);


    po::notify(varMap);

    if (varMap.count("search-dir")) {




















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






































































