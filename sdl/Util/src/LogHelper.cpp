#include <sdl/Util/LogHelper.hpp>
#include <sdl/Util/RefCount.hpp>
#include <sdl/Util/InitLogger.hpp>

namespace sdl {
namespace Util {

#if SDL_LOG_SEQUENCE_NUMBER
static AtomicCount gAtomicLogSeqXmt(0);

std::size_t nextLogSeq() {
  std::size_t r = ++gAtomicLogSeqXmt;
  gLogSeqXmt = r;
  return r;
}
#endif

bool gFinishedLogging = false;

void finishLogging() {
  gFinishedLogging = true;
}

WithInitLogging::WithInitLogging(char const* name, LogLevel level)
{
  initLoggerConsole(name, level);
}

}}
