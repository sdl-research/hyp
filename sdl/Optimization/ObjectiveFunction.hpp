/**





 */





#include <cassert>
#include <list>
#include <vector>
#include <utility>
#include <algorithm>

#include <boost/thread/mutex.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread/thread.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>












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

 */
template <class FloatT>
struct IRegularizeFct {

  virtual ~IRegularizeFct(){};
};

/*

 */
template <class FloatT>

 public:



    FloatT norm(0.0);




    }



  }

 private:

};

/**














 */
template <class FloatT>
class IObjectiveFunction {













  virtual ~IObjectiveFunction() {}

  /**


     *




     *



   */


  /**

   */
  virtual FloatT getFunctionValue() const = 0;

  /**


   */
  virtual void accept(IObjectiveFunctionVisitor<FloatT>*) = 0;
};

/**

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

};

/// Queue is typically boost::lockfree::queue<T>
template <class FloatT, class Queue>
struct GradientUpdate_Queue : public IUpdate<FloatT> {





  }

  Queue& queue_;
};

template <class FloatT>
struct GradientUpdate : public IUpdate<FloatT> {




  FloatT* gradients_;
};

// template<class FloatT>
// struct GradientUpdate_Map : public IUpdate<FloatT> {

//    aMap[index] += value;
//  }

//  Map aMap;
//};

template <class FloatT>
struct ScaledGradientUpdate : public IUpdate<FloatT> {





  FloatT* gradients_;

};

/**




 */
template <class Queue, class FloatT>
struct ConsumeUpdates {










  void operator()() {

    Pair p;
    while (!areProducersDone) {
      while (queue.pop(p)) {


      }




    }

  }




  IUpdate<FloatT>& updates;
  Queue& queue;
  boost::atomic<bool>& areProducersDone;
};

/**






 */
template <class FloatT>


 public:
  DataObjectiveFunction() : numThreads_(1), doScale_(false) {}

  virtual void setNumThreads(std::size_t n) {

    numThreads_ = n;
  }

  virtual void doScale(bool b) {

    doScale_ = b;
  }

  virtual ~DataObjectiveFunction() {}



                                                              << getNumExamples() << " examples.");
    setFeatureWeights(newParams, numParams);

    initFunctionValue();

    // Add regularization terms to function value and gradients







    // Get updates from all training examples:
    FloatT fctValUpdate = (FloatT)0.0;

      fctValUpdate = getUpdates(0, getNumExamples(), *update);

      fctValUpdate = getUpdatesParallel(0, getNumExamples(), *update);



    fctVal_ += fctValUpdate;

    if (Util::isDebugBuild()) {






    }


    return fctVal_;
  }

  /**

   */




  }

  struct CallGetUpdates : boost::noncopyable {





    void operator()() {

      result_ = obj_->getUpdates(begin_, end_, updates_);

    }

    DataObjectiveFunction<FloatT>* obj_;

    IUpdate<FloatT>& updates_;
    FloatT result_;
  };

  /**


   */


    assert(end > begin);


    typedef boost::lockfree::queue<Pair> Queue;
    assert(numThreads_ > 1);
    TrainingDataIndex numProducers = numThreads_ - 1;  // one of the threads is the consumer
    Queue queue(50 * numProducers);

    boost::thread_group producerThreads, consumerThreads;



    TrainingDataIndex rest = size % numProducers;



    producers.reserve(numProducers);
    GradientUpdate_Queue<FloatT, Queue> updateQueue(queue);

    TrainingDataIndex prevBlockEnd = begin;
    for (TrainingDataIndex i = 0; i < numProducers; ++i) {
      TrainingDataIndex const blockEnd = prevBlockEnd + blockSize + (i < rest);
      // process one more in the first 'rest' threads so no training examples are left out

      prevBlockEnd = blockEnd;
      producerThreads.create_thread(boost::ref(*update));  // boost::ref prevents copy
    }

    boost::atomic<bool> areProducersDone(false);

    // just one consumer; consumes gradient updates from queue and
    // writes them into gradient array
    ConsumeUpdates<Queue, FloatT> consumer(updates, queue, areProducersDone);
    consumerThreads.create_thread(consumer);

    producerThreads.join_all();
    areProducersDone = true;
    consumerThreads.join_all();

    FloatT fctValDelta(0.0f);

















  /**

   */
  virtual std::size_t getNumExamples() = 0;

  /**

   */


  /**


   */



  FloatT fctVal_;

  bool doScale_;
};

/**

 */
template <class FloatT>
class MockObjectiveFunction : public IObjectiveFunction<FloatT> {

 public:

    function_value_ = (FloatT)0.0;
    for (int i = 0; i < num_params; i += 2) {




      function_value_ += t1 * t1 + t2 * t2;
    }
    return function_value_;
  }





 private:
  FloatT function_value_;

};  // end class




#endif
