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
/**
   \file

   Defines general objective functions for optimization.

   \author Markus Dreyer
 */

#ifndef SDL_OPTIMIZATION_OBJECTIVEFUNCTION_HPP
#define SDL_OPTIMIZATION_OBJECTIVEFUNCTION_HPP
#pragma once

#include <cassert>
#include <list>
#include <vector>
#include <utility>
#include <algorithm>

#include <mutex>
#include <thread>
#include <sdl/SharedPtr.hpp>
#include <sdl/Util/Unordered.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/lockfree/queue.hpp>
#include <functional>
#include <boost/bind.hpp>

#include <atomic>
#include <sdl/Exception.hpp>
#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Util/Equal.hpp>
#include <sdl/Util/IsDebugBuild.hpp>
#include <sdl/Util/Delete.hpp>
#include <sdl/Optimization/IOriginalFeatureIds.hpp>
#include <functional>
#include <sdl/Optimization/Types.hpp>
#include <sdl/Util/Sleep.hpp>
#include <graehl/shared/thread_group.hpp>

namespace sdl {
namespace Optimization {

// fwd decl:
template <class FloatT>
class IObjectiveFunctionVisitor;

template <class X, class Y>
struct PodPair {
  X first;
  Y second;
};

template <class X, class Y>
PodPair<X, Y> makePodPair(X x, Y y) {
  PodPair<X, Y> p;
  p.first = x;
  p.second = y;
  return p;
}

/*
Adds regularization gradient to gradients and return delta for objective
 */
template <class FloatT>
struct IRegularizeFct {
  virtual FloatT operator()(FloatT const* params, FloatT* gradients, FeatureId numParams) const = 0;
  virtual ~IRegularizeFct(){};
};

/*
Adds L2 regularizer to gradients and function value.
 */
template <class FloatT>
class L2RegularizeFct : public IRegularizeFct<FloatT> {
 public:
  explicit L2RegularizeFct(FloatT v) : inverseVariance_(1 / (double)v) {}

  FloatT operator()(FloatT const* params, FloatT* gradients, FeatureId numParams) const override {
    FloatT norm(0.0);
    for (FeatureId i = 0; i < numParams; ++i) {
      FloatT const x = params[i];
      norm += x * x;
      gradients[i] += x * inverseVariance_;
    }
    norm /= 2;
    norm *= inverseVariance_;
    return norm;
  }

 private:
  FloatT const inverseVariance_;
};

/**
   API for an objective function, which can be optimized using
   L-BFGS, (stochastic) gradient descent or other methods. Objective
   functions have a function value and gradients (or alternatively,
   just updates resembling gradients).

   Examples for objective functions: A CRF objective function uses as
   (gradient) updates the difference A-B, where A=(feature
   expectations of the observed input-output pair) and B=(feature
   expectations of all possible outputs given the observed input). A
   perceptron is similar but B=(feature expectations of the current
   one-best output).

   Objective functions can also compute updates based on a loss
   function like 1-BLEU, mean squared error, etc.
 */
template <class FloatT>
class IObjectiveFunction {
 protected:
  shared_ptr<IRegularizeFct<FloatT>> regularize_;

 public:
  virtual void setRegularizeFct(IRegularizeFct<FloatT>* f) { regularize_.reset(f); }

  IRegularizeFct<FloatT>* getRegularizeFct() { return regularize_.get(); }

  /// \return part of objective fn due to params
  FloatT regularize(FloatT const* params, FloatT* gradients, FeatureId numParams) {
    return regularize_ ? (*regularize_)(params, gradients, numParams) : 0;
  }

  virtual ~IObjectiveFunction() {}

  /**
      Takes new parameters and updates the gradients and
      function value accordingly.
     *

      \param newParams The new parameters based on which the gradients
      and the function value are recomputed. (The type is FloatT*
      instead of std::vector<FloatT> because the L-BFGS uses FloatT*.)
     *

      \param resultingGradients Array must be of same size as the
      parameters and allocated outside.
   */
  virtual FloatT update(FloatT const* newParams, FloatT* resultingGradients, FeatureId numParams) = 0;

  /**
      Returns current function value.
   */
  virtual FloatT getFunctionValue() const = 0;

  /**
      accepts visitor. Good for printing current parameter
      values, etc.
   */
  virtual void accept(IObjectiveFunctionVisitor<FloatT>*) = 0;
};

/**
   visitor for IObjectiveFunction.
 */
template <class FloatT>
class IObjectiveFunctionVisitor {
 public:
  virtual ~IObjectiveFunctionVisitor() {}
  virtual void visit(IObjectiveFunction<FloatT>*) = 0;
};

template <class FloatT>
struct IUpdate {
  virtual ~IUpdate() {}
  virtual void update(FeatureId index, FloatT value) = 0;
};

/// Queue is typically boost::lockfree::queue<T>
template <class FloatT, class Queue>
struct GradientUpdate_Queue : public IUpdate<FloatT> {
  GradientUpdate_Queue(Queue& queue) : queue_(queue) {}

  void update(FeatureId index, FloatT value) override {
    while (!queue_.bounded_push(makePodPair(index, value)))
      ;
  }

  Queue& queue_;
};

template <class FloatT>
struct GradientUpdate : public IUpdate<FloatT> {
  GradientUpdate(FloatT* gradients) : gradients_(gradients) {}

  void update(FeatureId index, FloatT value) override { gradients_[index] += value; }

  FloatT* gradients_;
};

template <class FloatT>
struct ScaledGradientUpdate : public IUpdate<FloatT> {
  ScaledGradientUpdate(FloatT* gradients, FloatT scale = (FloatT)1.0)
      : gradients_(gradients), scaleTimes_(1 / scale) {}

  void update(FeatureId index, FloatT value) override { gradients_[index] += value * scaleTimes_; }

  FloatT* gradients_;
  FloatT scaleTimes_;
};

/**
   Consume updates of the form (index, gradient_delta) from the
   producer threads that compute feature expectations
   (gradients). Forward them to the IUpdate that adds them to the one
   gradient vector.
 */
template <class Queue, class FloatT>
struct ConsumeUpdates {

  ConsumeUpdates(IUpdate<FloatT>& updates_, Queue& queue_, std::atomic<bool>& areProducersDone_)
      : updates(updates_), queue(queue_), areProducersDone(areProducersDone_) {}

  typedef PodPair<FeatureId, FloatT> Pair;

  void update(Pair p) { updates.update(p.first, p.second); }

  enum { kSleepAfterFails = 1000 };  // avoid burning CPU when producers aren't keeping up
  enum { kSleepForMicroSeconds = 100 };
  void operator()() {
    unsigned fails = 0;
    Pair p;
    while (!areProducersDone) {
      while (queue.pop(p)) {
        update(p);
        fails = 0;
      }
      if (fails >= kSleepAfterFails)
        Util::usSleep(kSleepForMicroSeconds);
      else
        ++fails;
    }
    while (queue.pop(p)) update(p);
  }

#if SDL_DEBUG
  std::size_t n_;
#endif
  IUpdate<FloatT>& updates;
  Queue& queue;
  std::atomic<bool>& areProducersDone;
};

/**
   An objective function that's based on (training)
   data. Applies to most objective functions in NLP and machine
   learning.

   \tparam Updates The type for a list of updates to the parameters
   and/or gradients.
 */
template <class FloatT>
class DataObjectiveFunction : public IObjectiveFunction<FloatT> {

 public:
  DataObjectiveFunction() : numThreads_(1), doScale_(false) {}

  virtual void setNumThreads(std::size_t n) {
    SDL_INFO(Optimization, "Number of threads: " << n);
    numThreads_ = n;
  }

  virtual void doScale(bool b) {
    SDL_INFO(Optimization.DataObjectiveFunction, "Scale: " << (b ? "Y" : "N"));
    doScale_ = b;
  }

  virtual ~DataObjectiveFunction() {}

  virtual FloatT update(FloatT const* newParams, FloatT* resultingGradients, FeatureId numParams) {
    SDL_TRACE(Optimization.DataObjectiveFunction, "Updating " << numParams << " parameters on "
                                                              << getNumExamples() << " examples.");
    setFeatureWeights(newParams, numParams);
    zeroGradients(resultingGradients, numParams);
    initFunctionValue();

    // Add regularization terms to function value and gradients
    fctVal_ += this->regularize(newParams, resultingGradients, numParams);

    typedef IUpdate<FloatT> Update;
    unique_ptr<Update> update(
        doScale_ ? (Update*)new ScaledGradientUpdate<FloatT>(resultingGradients, (FloatT)getNumExamples())
                 : (Update*)new GradientUpdate<FloatT>(resultingGradients));

    // Get updates from all training examples:
    FloatT fctValUpdate = (FloatT)0.0;
    if (numThreads_ == 1)
      fctValUpdate = getUpdates(0, getNumExamples(), *update);
    else
      fctValUpdate = getUpdatesParallel(0, getNumExamples(), *update);

    if (doScale_) fctValUpdate /= getNumExamples();

    fctVal_ += fctValUpdate;

    if (Util::isDebugBuild()) {
      for (FeatureId i = 0; i < numParams; ++i)
        SDL_TRACE(Optimization.DataObjectiveFunction, "x[" << i << "] = " << std::setprecision(8)
                                                           << newParams[i]);
      for (FeatureId i = 0; i < numParams; ++i)
        SDL_TRACE(Optimization.DataObjectiveFunction, "g[" << i << "] = " << std::setprecision(8)
                                                           << resultingGradients[i]);
    }
    SDL_TRACE(Optimization.DataObjectiveFunction, "f = " << std::setprecision(8) << fctVal_);

    return fctVal_;
  }

  /**
      Computes the gradient updates for the specified training data range [begin, end)
   */
  virtual FloatT getUpdates(TrainingDataIndex begin, TrainingDataIndex end, IUpdate<FloatT>& updates) = 0;

  virtual FloatT getUpdate(TrainingDataIndex i, IUpdate<FloatT>& updates) const {
    SDL_THROW_LOG(Optimization, UnimplementedException, "unimplemented");
  }

  struct CallGetUpdates {
    CallGetUpdates(CallGetUpdates const&) = delete;
    CallGetUpdates(DataObjectiveFunction<FloatT>* obj, TrainingDataIndex begin, TrainingDataIndex end,
                   IUpdate<FloatT>& updates)
        : obj_(obj), begin_(begin), end_(end), updates_(updates) {}

    void operator()() {
      SDL_DEBUG(Optimization, "Computing updates " << begin_ << " to " << end_);
      result_ = obj_->getUpdates(begin_, end_, updates_);
      SDL_DEBUG(Optimization, "Computing updates " << begin_ << " to " << end_ << " result: " << result_);
    }

    DataObjectiveFunction<FloatT>* obj_;
    TrainingDataIndex begin_, end_;
    IUpdate<FloatT>& updates_;
    FloatT result_;
  };

  /**
      Multithreading implemented after:
      http://www.boost.org/doc/libs/1_55_0/doc/html/lockfree/examples.html
   */
  virtual FloatT getUpdatesParallel(TrainingDataIndex begin, TrainingDataIndex end, IUpdate<FloatT>& updates) {
    SDL_DEBUG(Optimization, "getUpdatesParallel from " << begin << " to " << end);
    assert(end > begin);

    typedef PodPair<FeatureId, FloatT> Pair;  // Windows compiler didn't like lockfree::queue<std::pair>
    typedef boost::lockfree::queue<Pair> Queue;
    assert(numThreads_ > 1);
    TrainingDataIndex numProducers = numThreads_-1;  // one of the threads is the consumer
    Queue queue(50 * numProducers);

    graehl::thread_group producerThreads, consumerThreads;

    TrainingDataIndex size = end - begin;
    TrainingDataIndex blockSize = size / numProducers;
    TrainingDataIndex rest = size % numProducers;

    typedef Util::AutoDeleteAll<CallGetUpdates> Producers;
    Producers producers;
    producers.reserve(numProducers);
    GradientUpdate_Queue<FloatT, Queue> updateQueue(queue);
    CallGetUpdates* update;
    TrainingDataIndex prevBlockEnd = begin;
    for (TrainingDataIndex i = 0; i < numProducers; ++i) {
      TrainingDataIndex const blockEnd = prevBlockEnd + blockSize + (i < rest);
      // process one more in the first 'rest' threads so no training examples are left out
      producers.push_back(update = new CallGetUpdates(this, prevBlockEnd, blockEnd, updateQueue));
      prevBlockEnd = blockEnd;
      producerThreads.create_thread(std::ref(*update));
    }

    std::atomic<bool> areProducersDone(false);

    // just one consumer; consumes gradient updates from queue and
    // writes them into gradient array
    ConsumeUpdates<Queue, FloatT> consumer(updates, queue, areProducersDone);
    consumerThreads.create_thread(consumer);

    producerThreads.join_all();
    areProducersDone = true;
    consumerThreads.join_all();

    FloatT fctValDelta(0.0f);

    for (typename Producers::const_iterator i = producers.begin(), e = producers.end(); i != e; ++i)
      fctValDelta += (*i)->result_;

    return fctValDelta;
  }

  virtual FloatT getFunctionValue() const { return fctVal_; }

  virtual void setFunctionValue(FloatT const& f) { fctVal_ = f; }

  virtual void initFunctionValue() { fctVal_ = (FloatT)0.0; }

  virtual void increaseFunctionValue(FloatT const& f) { fctVal_ += f; }

  void zeroGradients(FloatT* gradients, FeatureId size) { std::fill_n(gradients, size, (FloatT)0.0); }

  /**
      Returns the number of training examples.
   */
  virtual std::size_t getNumExamples() = 0;

  /**
      Inserts feature weights into all training examples
   */
  virtual void setFeatureWeights(FloatT const* params, FeatureId numParams) = 0;

  /**
      Inserts feature weights into training examples [begin,
      end).
   */
  virtual void setFeatureWeights(TrainingDataIndex begin, TrainingDataIndex end, FloatT const* params,
                                 FeatureId numParams)
      = 0;

  FloatT fctVal_;
  TrainingDataIndex numThreads_;
  bool doScale_;
};

/**
   A toy objective function from http://www.chokkan.org/software/liblbfgs.
 */
template <class FloatT>
class MockObjectiveFunction : public IObjectiveFunction<FloatT> {

 public:
  FloatT update(FloatT const* x, FloatT* g, FeatureId num_params) {
    function_value_ = (FloatT)0.0;
    for (int i = 0; i < num_params; i += 2) {
      FloatT t1 = (FloatT)1.0-x[i];
      FloatT t2 = (FloatT)10.0 * (x[i + 1]-x[i] * x[i]);
      g[i + 1] = (FloatT)20.0 * t2;
      g[i] = (FloatT)-2.0 * (x[i] * g[i + 1] + t1);
      function_value_ += t1 * t1 + t2 * t2;
    }
    return function_value_;
  }

  FloatT getFunctionValue() const { return function_value_; }

  void accept(IObjectiveFunctionVisitor<FloatT>* visitor) { visitor->visit(this); }

 private:
  FloatT function_value_;
};


}}

#endif
