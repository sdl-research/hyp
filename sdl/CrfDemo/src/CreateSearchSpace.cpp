







namespace Resources {
struct ResourceManager;
}

namespace CrfDemo {

// Explicit template instantiation
template
class CreateSearchSpace<Optimization::Arc>;



// A simple C factory function that we can load dynamically
extern "C" {



  Optimization::ICreateSearchSpace<Optimization::Arc>*
  load(Resources::ResourceManager& resourceManager
       , ConfigNode const& yamlConfig
       , bool b) {
    return new CrfDemo::CreateSearchSpace<Optimization::Arc>(yamlConfig);
  }
}
