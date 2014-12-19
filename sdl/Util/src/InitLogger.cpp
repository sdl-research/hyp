





#include <log4cxx/logger.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/helpers/transcoder.h>
#endif







#include <iostream>

#include <boost/date_time/posix_time/posix_time.hpp>








namespace Util {

std::string logFileName(std::string const& appname) {



}

//TODO: free memory for valgrind?
#ifndef NLOG
void initLogger(std::string const& appname,







                LogLevelPtr level,

{
  defaultLocale();






  log4cxx::LoggerPtr g_pStatsDBLogger(log4cxx::Logger::getRootLogger());
  if (opts.removeAppenders)
    g_pStatsDBLogger->removeAllAppenders();
  log4cxx::LogString logStrLayout;


      opts.patternLayout;

  log4cxx::PatternLayout* pLayout = new log4cxx::PatternLayout(logStrLayout);
  if (!opts.file.empty()) {

    if (opts.verbose)

    log4cxx::LogString logFileName;

    log4cxx::FileAppender* pAppender = new log4cxx::FileAppender(pLayout, logFileName, !opts.overwriteFile);
    g_pStatsDBLogger->addAppender(pAppender);
  }
  if (opts.console) {
    g_pStatsDBLogger->addAppender(
        new log4cxx::ConsoleAppender(pLayout, log4cxx::ConsoleAppender::getSystemErr())
                                  );
  }
  if (opts.verbose) {

  }
  g_pStatsDBLogger->setLevel(level);
  if (opts.verbose)

  findFile().activateLogging();
}









#else
void initLogger(std::string const& appname,
                LogLevelPtr level,





void initLogger(std::string const& appname,









#endif // end ifndef NLOG



