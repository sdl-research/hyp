typedef ViterbiWeight Weight;
typedef ArcTpl<Weight> Arc;

shared_ptr<IVocabulary> voc(createDefaultVocab());
SymId john = voc->addSymbol("John", kTerminal);
SymId loves = voc->addSymbol("loves", kTerminal);
SymId likes = voc->addSymbol("likes", kTerminal);
SymId mary = voc->addSymbol("Mary", kTerminal);
SymId vp = voc->addSymbol("VP", kNonTerminal);

MutableHypergraph<Arc> hyp;
hyp.setVocabulary(voc);

StateId s0 = hyp.addState(vp);
StateId s1 = hyp.addState();

hyp.addArc(new Arc(Head(s0),
                   Tails(hyp.addState(likes), hyp.addState(mary)),
                   Weight(2.0f)));
hyp.addArc(new Arc(Head(s0),
                   Tails(hyp.addState(loves), hyp.addState(mary)),
                   Weight(4.0f)));
hyp.addArc(new Arc(Head(s1), Tails(hyp.addState(john), s0),
                   Weight(8.0f)));

hyp.setFinal(s1);
