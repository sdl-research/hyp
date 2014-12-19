/** \file

    mostly used for unit tests.
*/


#ifndef HYP__HYPERGRAPH_HELPERFUNCTIONS_HPP
#define HYP__HYPERGRAPH_HELPERFUNCTIONS_HPP
#pragma once

#include <string>
#include <sdl/SharedPtr.hpp>

#include <sdl/Vocabulary/HelperFunctions.hpp>
#include <sdl/IVocabulary-fwd.hpp>
#include <sdl/Hypergraph/FwdDecls.hpp>

namespace sdl {
namespace Hypergraph {

template<class Arc>
IMutableHypergraph<Arc>*
constructHypergraphFromString(std::string const&,
                              IVocabularyPtr const& pVoc = Vocabulary::createDefaultVocab());

}}

#endif
