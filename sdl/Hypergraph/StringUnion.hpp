



















/* aka dictionary



*/




















  bool push_weights;

  bool loop;  // if !loop, the ignoreTags is meaningless













  bool chars;
  std::string wordsep;


  typedef std::string IgnoreTag;
  typedef std::vector<IgnoreTag> IgnoreTags;


  IgnoreTags ignoreTags;
















































































































  typedef A Arc;












































  void buildTrie(WS const& ws) {











      // TODO: push_weights (postprocess for general (at least acyclic fsm) HG?)
    }
  }


    hg.addArcFsm(s, loopEndState, hg.addState(label), s == start ? times(wt, perTokenWeight) : wt);




















































  typedef std::vector<StateId> LastTrie;

















      hg.addArcFsm(start, trieStart, begin.labelState, Weight::one());

    LastTrie triep;
    if (endOfTokenSequenceWeight != Weight::one()) {




    }













    }
    buildTrie(ws);






















        StateId asciiCharSt = hg.addState(lexicalSymbol(s, *voc));












        StateId spaceState = hg.addState(deleteSpace);





























  typedef std::vector<LabelPair> Tags;
  Tags tags;  // input = open, output = close









    hg.clear(kFsm | kCanonicalLex | kStoreFirstTailOutArcs);




























