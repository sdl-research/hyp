using namespace sdl;
using namespace sdl::Hypergraph;
typedef ViterbiWeight Weight;
typedef ArcTpl<Weight> Arc;
MutableHypergraph<Arc> hyp;
IVocabularyPtr voc(createDefaultVocab());
Sym john = voc->add("John", kTerminal);
Sym loves = voc->add("loves", kTerminal);
Sym likes = voc->add("likes", kTerminal);
Sym mary = voc->add("Mary", kTerminal);
Sym vp = voc->add("VP", kNonTerminal);

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

hyp.setVocabulary(voc);
hyp.setFinal(s1);
