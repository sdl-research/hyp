/**





 */





#include <string>








namespace Optimization {



/**



 */
template<class Arc>
class ICreateSearchSpace {

 public:

  virtual ~ICreateSearchSpace() {}

  /**


     *



     *






   */
  virtual Hypergraph::IMutableHypergraph<Arc>*
  getUnclampedHypergraph(IInput const& observedInput) = 0;

  /**



   */

  getFeatureHypergraphPairs() const = 0;

  virtual std::size_t getNumFeatures() = 0;

  /**



   */
  virtual void setTestMode() = 0;

  /**





   */
  virtual void prepareTraining() = 0;

  virtual std::string getName() const = 0;
};



#endif
