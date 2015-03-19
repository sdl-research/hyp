/** \file

    in case you want to maintain spaces in a character-based token sequence - we
    don't allow space in our tokens.
*/

#ifndef SPACETOKEN_JG_2013_12_21_HPP
#define SPACETOKEN_JG_2013_12_21_HPP
#pragma once

namespace sdl { namespace Util {

/**
   pbmt training doesn't work on raw space tokens, and xmt doesn't allow
   whitespace in tokens, so we may 'escape' them w/ kEscSpace
*/
namespace {
std::string const kSpace = " ";
std::string const kEscSpace("_space_");
}

struct SpaceTokenOptions {
  std::string spaceToken;
  SpaceTokenOptions() {
    spaceToken = kEscSpace;
  }
  template <class Config>
  void configure(Config &config) {
    config("space-token", &spaceToken).self_init()
        ("(for input-type hypergraph) token used to encode a space character (we turn it into token breaks in the resulting unknown word rule) - shouldn't need changing from default, but do train your pbmt pipeline with that token in the target bitext + lm if you have multi-word target names from a single source word). empty string disables.");
  }
};


}}

#endif
