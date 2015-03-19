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
