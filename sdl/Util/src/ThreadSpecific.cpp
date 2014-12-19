#include <sdl/Util/ThreadSpecific.hpp>

namespace sdl { namespace Util {

#if SDL_BOOST_TSS_BUG
struct DummyTssCleanupFunction : public impl::tss_cleanup_function {
  virtual void operator()(void* data) {}
};

static shared_ptr<impl::tss_cleanup_function> gDummyTssCleanupFunction(new DummyTssCleanupFunction());

shared_ptr<impl::tss_cleanup_function> makeDummyTssCleanupFunction() {
  return gDummyTssCleanupFunction;
}
#endif

}}
