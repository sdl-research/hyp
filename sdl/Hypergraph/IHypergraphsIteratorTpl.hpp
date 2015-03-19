/** \file

    sequence of hypergraph (e.g. from input file(s)).
*/

#ifndef HYP__HYPERGRAPH_HYPERGRAPHSITERATOR_HPP
#define HYP__HYPERGRAPH_HYPERGRAPHSITERATOR_HPP
#pragma once

#include <istream>
#include <sdl/SharedPtr.hpp>
#include <sdl/Hypergraph/Properties.hpp>
#include <sdl/IVocabulary.hpp>
#include <sdl/Hypergraph/ParsedArcs.hpp>
#include <sdl/Hypergraph/FeaturesPerInputPosition.hpp>
#include <sdl/Util/Enum.hpp>

namespace sdl {


struct IVocabulary;

namespace Hypergraph {

SDL_ENUM(InputHgType, 2, (FlatStringsHg, DashesSeparatedHg));

/// pass arcs to #include <sdl/Hypergraph/ArcParserFct.hpp> parseText
void readArcsUntil(std::istream & in, ParsedArcs &arcs, bool requireNfc = true);

// fwd decls
template<class Arc> struct IHypergraph;

/**
   \author Markus Dreyer

   Iterates over hypergraphs, which are constructed from the
   input stream (which may be in various formats).
*/
template<class A>
class IHypergraphsIteratorTpl {

 public:
  typedef A Arc;

  inline
  virtual ~IHypergraphsIteratorTpl() {}

  /**
     Prepares the next hypergraph.
  */
  virtual void next() = 0;

  /**
     Returns the current hypergraph.
  */
  virtual IHypergraph<Arc>* value() = 0;

  /**
     Returns true if no further hypergraph found in the stream.
  */
  virtual bool done() const = 0;

  /**
     Set the properties of the read in hypergraph, for instance kStoreInArcs
  */
  virtual void setHgProperties(Properties prop) = 0;

  /**
     Factory method; constructs a hypergraph iterator from some
     input stream, which may be of various formats.
  */
  static IHypergraphsIteratorTpl<Arc>* create(std::istream&,
                                              InputHgType inputType,
                                              shared_ptr<IPerThreadVocabulary> const& perThreadVocab,
                                              shared_ptr<IFeaturesPerInputPosition> feats = shared_ptr<IFeaturesPerInputPosition>());
};

}}

#endif
