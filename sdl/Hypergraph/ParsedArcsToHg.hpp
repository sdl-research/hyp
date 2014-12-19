























  typedef typename Arc::Weight Weight;







  // Figure out next available state ID (for states for which no state
  // ID is specified)



























    }














    }
    else {
      Arc* arc = new Arc();
      arc->setHead(wrappedArc->head.id);

        arc->addTail(t.id);
      }
      if (!wrappedArc->weightStr.empty()) {
        Weight w;

          parseWeightString(wrappedArc->weightStr, &w);






        arc->setWeight(w);
      }










