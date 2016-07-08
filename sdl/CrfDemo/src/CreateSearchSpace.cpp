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
#include <sdl/CrfDemo/CreateSearchSpace.hpp>
#include <sdl/Optimization/Arc.hpp>
#include <sdl/Optimization/ICreateSearchSpace.hpp>
#include <sdl/Config-fwd.hpp>
#include <sdl/IVocabulary-fwd.hpp>

namespace sdl {

namespace Resources {
struct ResourceManager;
}

namespace CrfDemo {

SDL_NAME_ENUM(TransitionModelType);

// Explicit template instantiation
template class CreateSearchSpace<Optimization::Arc>;
}
}

// A simple C factory function that we can load dynamically
extern "C" {

using namespace sdl;

Optimization::ICreateSearchSpace<Optimization::Arc>* loadCss(Resources::ResourceManager& resourceManager,
                                                             ConfigNode const& yamlConfig, bool b) {
  return new CrfDemo::CreateSearchSpace<Optimization::Arc>(yamlConfig);
}


}
