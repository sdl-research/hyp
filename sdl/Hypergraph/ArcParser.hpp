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

  ParserUtil::Arc* parse(const std::string& str) const;

  shared_ptr<Impl> pImpl_;

};

}}

#endif
