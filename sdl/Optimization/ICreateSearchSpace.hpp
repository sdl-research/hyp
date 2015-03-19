/**
   \file

   Contains the ICreateSearchSpace interface.

   \author Markus Dreyer
 */

#ifndef SDL_OPTIMIZATION_ICREATESEARCHSPACE_HPP
#define SDL_OPTIMIZATION_ICREATESEARCHSPACE_HPP
#pragma once

#include <string>
#include <sdl/SharedPtr.hpp>

#include <sdl/Hypergraph/IHypergraph.hpp>
#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Optimization/FeatureHypergraphPairs.hpp>
#include <sdl/Optimization/IInput.hpp>

namespace sdl {
namespace Optimization {

enum { kIsXmtModule = false, kIsOptimize = true };

/**
   Users can implement this for their specific task (see, e.g.,
   TrainableCapitalizer) and the optimize their task-specific feature
   weights using the OptimizationProcedure.
 */
template<class Arc>
class ICreateSearchSpace {

 public:

  virtual ~ICreateSearchSpace() {}

  /**
      Creates a hypergraph that is not clamped to specific
       output; just to the observed input.
     *
      The hypergraph contains all possible outputs given the input (as
      well as all possible alignments between the input and these
      outputs). The hypergraph must contain features.
     *
      This function is called at decoding time. At training time, the
      function overload below is called, which takes both the observed
      input and output strings.

     featureWeights are added to the arc value (dotproduct values) - should not be
     provided for training; just test/decode.
   */
  virtual Hypergraph::IMutableHypergraph<Arc>*
  getUnclampedHypergraph(IInput const& observedInput) = 0;

  /**
      Creates the task-specific training data (will load files
      and create one unclamped/clamped hypergraph pair per training
      example).
   */
  virtual shared_ptr< IFeatureHypergraphPairs<Arc> >
  getFeatureHypergraphPairs() const = 0;

  virtual std::size_t getNumFeatures() = 0;

  /**
      In test mode, we don't map feature IDs to contiguous space
      and don't reset all values to zero. (Values must be set according
      to test weights before decoding.)
   */
  virtual void setTestMode() = 0;

  /**
      Reads training files, etc.

     featureWeights are added to the arc value (dotproduct values) - should not be
     provided for training; just test/decode.

   */
  virtual void prepareTraining() = 0;

  virtual std::string getName() const = 0;
};

}}

#endif
