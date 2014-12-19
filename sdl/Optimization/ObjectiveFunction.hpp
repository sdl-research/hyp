











#include <cassert>

#include <vector>

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









template <class FloatT>



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

    Queue queue(50 * numProducers);










    GradientUpdate_Queue<FloatT, Queue> updateQueue(queue);








    }

    boost::atomic<bool> areProducersDone(false);

    // just one consumer; consumes gradient updates from queue and
    // writes them into gradient array
    ConsumeUpdates<Queue, FloatT> consumer(updates, queue, areProducersDone);



    areProducersDone = true;


    FloatT fctValDelta(0.0f);


































  FloatT fctVal_;

  bool doScale_;










    function_value_ = (FloatT)0.0;














 private:








