/** \file

    Hypergraph-specific exceptions.
*/

#ifndef HYP__HYPERGRAPH_EXCEPTION_HPP
#define HYP__HYPERGRAPH_EXCEPTION_HPP
#pragma once

#include <sdl/Exception.hpp>

namespace sdl {
namespace Hypergraph {

VERBOSE_EXCEPTION_DECLARE(MultipleDerivationsException)
VERBOSE_EXCEPTION_DECLARE(LazyHypergraphException)
VERBOSE_EXCEPTION_DECLARE(FsmNoStartException)
VERBOSE_EXCEPTION_DECLARE(NonFsmHypergraphException)
VERBOSE_EXCEPTION_DECLARE(ImmutableHypergraphException)
VERBOSE_EXCEPTION_DECLARE(HypergraphPropertiesException)


}}

#endif
