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
/** \file

    allow some algorithms to modify either input or output labels, or both.

     instead of configuring a bool operate-on-output option, we have:
     operate-on: [OperateOnOutput, OperateOnInput, OperateOnInputOutput]

     to use, your code can transform both input and output in sequence, protected by:

     // for labeled state of hg
     if (inputNeeded(operateOn)) { Sym sym=hg.inputLabel(state); ... }
     if (outputNeeded(operateOn, hg, state)) { Sym sym=hg.inputLabel(state); ... }

     the reason for the second is that you may have
     hg.outputLabelFollowsInput(state). if so, then by changing the input, you
     arleady changed the output. that is, we may store the input=output identity
     transducer more compactly, as an fsa (with hg.outputLabelFollowsInput()
     true for all states).
*/

#ifndef OPERATEON_JG2012927_HPP
#define OPERATEON_JG2012927_HPP
#pragma once

#include <sdl/Config/Init.hpp>
#include <sdl/Util/Enum.hpp>

namespace sdl {
namespace Hypergraph {

SDL_ENUM(OperateOn, 3, (OperateOnInput, OperateOnOutput, OperateOnInputOutput))

inline bool inputEnabled(OperateOn operateOn) {
  return operateOn == kOperateOnInput || operateOn == kOperateOnInputOutput;
}

inline bool outputEnabled(OperateOn operateOn) {
  return operateOn == kOperateOnOutput || operateOn == kOperateOnInputOutput;
}

/**
   this is tricky - see file comment for intended usage
*/
inline bool inputNeeded(OperateOn operateOn) {
  return inputEnabled(operateOn);
}

template <class Hypergraph>
bool outputNeeded(OperateOn operateOn, Hypergraph const& h) {
  return operateOn == kOperateOnOutput || operateOn == kOperateOnInputOutput && !h.outputLabelFollowsInput();
}

template <class Hypergraph>
bool outputNeeded(OperateOn operateOn, Hypergraph const& h, StateId s) {
  return operateOn == kOperateOnOutput || operateOn == kOperateOnInputOutput && !h.outputLabelFollowsInput(s);
}

inline bool outputOnly(OperateOn operateOn) {
  return operateOn == kOperateOnOutput;
}

struct OperateOnConfig {
  static std::string usage() {
    return "a string->string transform that may operate on input or output labels or both";
  }
  template <class Config>
  void configure(Config& config) {
    config("operate-on", &operateOn)
        .init(kOperateOnInput)(
            "which labels of a hypergraph to transform; OperateOnOutput has the effect of preserving the "
            "original FSA input symbol and placing the result on the output");
  }
  OperateOn operateOn;
  operator OperateOn() const { return operateOn; }
  OperateOnConfig() { Config::inits(this); }
};


}}

#endif
