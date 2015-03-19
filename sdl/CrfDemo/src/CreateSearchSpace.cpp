#include <sdl/Optimization/Arc.hpp>
#include <sdl/Optimization/ICreateSearchSpace.hpp>
#include <sdl/Config-fwd.hpp>
#include <sdl/CrfDemo/CreateSearchSpace.hpp>
#include <sdl/IVocabulary-fwd.hpp>

namespace sdl {

namespace Resources {
struct ResourceManager;
}

namespace CrfDemo {

// Explicit template instantiation
template
class CreateSearchSpace<Optimization::Arc>;

}}

// A simple C factory function that we can load dynamically
extern "C" {

  using namespace sdl;

  Optimization::ICreateSearchSpace<Optimization::Arc>*
  load(Resources::ResourceManager& resourceManager
       , ConfigNode const& yamlConfig
       , bool b) {
    return new CrfDemo::CreateSearchSpace<Optimization::Arc>(yamlConfig);
  }
}
