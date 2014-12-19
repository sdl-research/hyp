/**
   TODO: when feeding non-ascii utf8 input to this in debug mode, we can get a
   boost isalpha_ assertion. CM-207
*/

#include <sdl/Hypergraph/ParserUtil.hpp>
#include <sdl/Hypergraph/ArcParser.hpp>
#include <sdl/Hypergraph/ArcParserImpl.hpp>

#include <string>

namespace sdl {
namespace Hypergraph {

ArcParser::ArcParser()
    : pImpl_(new Impl()) {}

ParserUtil::Arc* ArcParser::parse(const std::string& str) const {
  return pImpl_->parse(str);
}

}}
