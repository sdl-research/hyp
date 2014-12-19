/**








 */





#include <string>
#include <vector>
#include <map>
#include <stdexcept>



















namespace Optimization {



struct OptimizationProcedureOptions {

  OptimizationProcedureOptions() {
    Config::inits(this);
  }

  template <class Config>
  void configure(Config& config) {

    config.is("OptimizationProcedure");

    config("optimization-method", &optimizationMethod)("Either 'lbfgs' or 'online'. For each method you can specify options, e.g., lbfgs").init(kLbfgs);
    config("lbfgs", &lbfgsOptions)("Options for 'lbfgs' (use if optimization-method is 'lbfgs')");
    config("online", &onlineOptions)("Options for 'online' (use if optimization-method is 'online')");

    config("variance", &variance)("Variance for L2 regularization").init(floatCast(2.0));
    config("weights-path", &weightsPath)("Output path for learned weights ('-' is stdout)").init("-");
    config("test-mode", &testMode)("Test mode? In test mode, we don't optimize, but read the weights and get the best path").init(false);
    config("test-mode-output-hypergraph", &testModeOutputHypergraph)("Prints hypergraph output (test mode)").init(false);
    config("test-mode-detailed-output", &testModeDetailed)("Will produce detailed info for 1-best (alignment, features, scores) ").init(false);
    config("num-threads", &numThreads)("Number of threads for computing feature expectations on the aligned data").init(1);
  }

  OptimizationMethod optimizationMethod;
  LbfgsOptimizerOptions lbfgsOptions;
  OnlineOptimizerOptions onlineOptions;

  std::string weightsPath;
  double variance;
  bool testMode;
  bool testModeDetailed;
  bool testModeOutputHypergraph;
  std::size_t numThreads;
};

/*

 */
class OptimizationProcedure
{
 public:
  typedef Optimization::FloatT FloatT;
  typedef Optimization::Arc Arc;

  OptimizationProcedure(shared_ptr<ICreateSearchSpace<Arc> >& searchSpace
                        , OptimizationProcedureOptions const&);

  /**


   */
  void optimize();

  /**



   */
  void setCheckIntersectionNonEmpty(bool b = true) {
    checkIntersectionNonEmpty_ = b;
  }

  /**



   */
  void setCheckGradients(bool b = true) {
    checkGradients_ = b;
  }

  /**


   */
  void test();

 private:
  OptimizationProcedureOptions opts_;

  shared_ptr<ICreateSearchSpace<Arc> > pSearchSpace_;
  bool checkIntersectionNonEmpty_;
  bool checkGradients_;
  Weight::Map weightsmap_;
};




#endif
