















enum LogLevel {
  kLogAll = 100,
  kLogTrace = 10,
  kLogDebug = 5,
  kLogInfo = 1,
  kLogWarn = -1,
  kLogError = -5,
  kLogFatal = -10
};





inline LogLevelPtr logLevel(int i) {
  return NULL;
}

inline LogLevelPtr logLevel(std::string const& l, int i) {
  return NULL;
}


typedef log4cxx::LevelPtr LogLevelPtr;
typedef log4cxx::LoggerPtr LoggerPtr;

//FIXME: logLevel(0) -> pure virtual method called
inline LogLevelPtr logLevel(int i) {








#undef LEVELATLEAST
  return log4cxx::Level::getOff();
}

// defaults
inline LogLevelPtr logLevel(std::string const& l, int i) {
  log4cxx::LevelPtr r=logLevel(i);
  return l.empty() ? r : log4cxx::Level::toLevel(l, r);
}





