








#include <istream>












namespace Hypergraph {






// fwd decls


/**




*/
template<class A>
class IHypergraphsIteratorTpl {

 public:
  typedef A Arc;

  inline
  virtual ~IHypergraphsIteratorTpl() {}

  /**

  */
  virtual void next() = 0;

  /**

  */
  virtual IHypergraph<Arc>* value() = 0;

  /**

  */
  virtual bool done() const = 0;

  /**

  */
  virtual void setHgProperties(Properties prop) = 0;

  /**


  */
  static IHypergraphsIteratorTpl<Arc>* create(std::istream&,

                                              shared_ptr<IPerThreadVocabulary> const& perThreadVocab,
                                              shared_ptr<IFeaturesPerInputPosition> feats = shared_ptr<IFeaturesPerInputPosition>());
};



#endif
