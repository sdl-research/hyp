
















namespace Hypergraph {
























































/**



 */




  PruneUnreachable(PruneOptions const& opt=PruneOptions()) : opt(opt) {}





















template<class Arc>
void pruneUnreachable(IMutableHypergraph<Arc>* pHg) {
  PruneUnreachable<Arc> pruneFct;
  inplace(*pHg, pruneFct);
}

template<class Arc>





void pruneUnreachable(IHypergraph<Arc> const& hgInput,
                      IMutableHypergraph<Arc>* pHgResult) {
  PruneUnreachable<Arc> pruneFct;
  inout(hgInput, pHgResult, pruneFct);
}





