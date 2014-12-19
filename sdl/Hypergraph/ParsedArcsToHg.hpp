























  typedef typename Arc::Weight Weight;

  // increments on each <xmt-block> symbol
  std::size_t numBlockStartSymsSeen = 0;




  // Figure out next available state ID (for states for which no state
  // ID is specified)

  StateId highestStateId = 0; // should be -1 but that is kNoState, so added workaround using hasStateIds
  bool hasStateIds = false;


    if (arc->head.hasId())
      hasStateIds = true;
    forall (ParserUtil::State s, arc->tails) {

        if (s.hasId())
          hasStateIds = true;
    }
  }

  if (!hasStateIds) {
    assert(highestStateId == 0);
    highestStateId = (StateId)-1; // so that next available ID is 0









               inFilename, linenum, &numBlockStartSymsSeen);
    }

             inFilename, linenum, &numBlockStartSymsSeen);












    }
    else {
      Arc* arc = new Arc();
      arc->setHead(wrappedArc->head.id);

        arc->addTail(t.id);
      }
      if (!wrappedArc->weightStr.empty()) {
        Weight w;
        try {
          parseWeightString(wrappedArc->weightStr, &w);
        }
        catch(std::exception& e) {



        }
        arc->setWeight(w);
      }










