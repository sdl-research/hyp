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
#ifndef HYP__HYPERGRAPH_ARCPARSER_HPP
#define HYP__HYPERGRAPH_ARCPARSER_HPP
#pragma once

#include <string>

#include <sdl/SharedPtr.hpp>

#include <sdl/Hypergraph/ParserUtil.hpp>

namespace sdl {
namespace Hypergraph {

template<class Iter>
struct ArcParserImpl;

/// hide implementation (which uses slow to compile boost::spirit) of arc parser
struct ArcParser {

  typedef ArcParserImpl<std::string::const_iterator> Impl;

  ArcParser();

  ParserUtil::Arc* parse(std::string const& str) const;

  shared_ptr<Impl> pImpl_;

};

}}

#endif
