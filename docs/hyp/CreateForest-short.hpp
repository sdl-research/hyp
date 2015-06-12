typedef ViterbiWeight Weight;
typedef ArcTpl<Weight> Arc;
MutableHypergraph<Arc> hyp;
StateId s = hyp.addState(S);
hyp.setFinal(s);
hyp.addArc(new Arc(Head(s),
                   Tails(hyp.addState(he),
                         hyp.addState(eats),
                         hyp.addState(rice)),
                   Weight(0.693)));
