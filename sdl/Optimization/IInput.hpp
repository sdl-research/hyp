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
#ifndef SDL_OPTIMIZATION_IINPUT_HPP
#define SDL_OPTIMIZATION_IINPUT_HPP
#pragma once

namespace sdl {
namespace Optimization {

/**
   General class that represents input in training and
   decoding. Specific tasks (e.g., TrainableCapitalizer) define what
   the input looks like for the task (see, for example,
   TrainableCapitalizer/Input.hpp). Task-specific input typically
   contains the input words along with other information, such as the
   POS tags of the input words, the parse tree, any aligned foreign
   source words, etc.
 */
class IInput {
 public:
  virtual ~IInput() {}
};


}}

#endif
