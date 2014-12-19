




















    logConfigFile = Util::findFile()(logConfigFile);
    log4cxx::xml::DOMConfigurator::configure(logConfigFile);
    Util::findFile().activateLogging();
  }






