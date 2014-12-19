




















#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/tuple/tuple_io.hpp>

#include <map>
#include <vector>
#include <stack>


namespace Hypergraph {

// TODO: could put everything into a class => less parameters to pass
// around

struct SubUnionOptions {
  bool requirePathOverlap;
  bool addStandardUnion;

  static inline std::string usage() {

           "additional paths that are not contained in standard union.";
  }
  template <class Config>
  void configure(Config& config) {
    config.is("SubUnionOptions");
    config("require-path-overlap", &requirePathOverlap)("require path overlap?");
    config("add-standard-union", &addStandardUnion)("add standard union?");
  }
};

namespace SubUnionUtil {

typedef boost::tuple<std::size_t, std::size_t> Span;

// TODO: boost (smart) pointer vector?

typedef std::map<StateId, Span> StateIdToSpan;



bool isValidSpan(Span const& sp) {






}

bool isSmallerSpan(Span const& s1, Span const& s2) {
  assert(s1 != undefSpan);

  using boost::get;


  return range1 < range2;
}

/**

*/
Span parseSpanStr(std::string const& str) {
  std::string::size_type hyphenPos = str.find('-');

  std::string i = str.substr(0, hyphenPos);
  std::string j = str.substr(hyphenPos + 1, str.length() - hyphenPos - 1);


}

/**


*/
template <class Arc>


  // e.g., "0-3"

  // we also have strings like "0-3.b", which is also span (0,3)
  // remove the ".b" part
  std::string::size_type dotPos = symStr.find(".");


}

template <class Key, class Value>

  typedef typename std::map<Key, Value>::const_iterator Iter;
  Iter it = aMap.begin();
  Iter best = it;
  std::advance(it, 1);
  for (; it != aMap.end(); ++it) {

  }
  return best;
}

/**


*/
template <class Arc>

  // Already done?
  StateIdToSpan::const_iterator found = stateIdToSpan->find(sid);

  if (hg.hasLexicalLabel(sid)) {



  }


  using boost::get;  // tuple fct


  // Look at the spans of children




    std::vector<Span> tailSpans;

    for (std::size_t i = 0, end = tailIds.size(); i < end; ++i) {
      Span sp = getSourceSpansBubbleUp(hg, tailIds[i], stateIdToSpan);
      tailSpans.push_back(sp);

    }




  }

  // Look at own span annotation if children are unreliable
  if (votesForLeft.size() != 1 || votesForRight.size() != 1) {

    ++votesForLeft[get<0>(sidSpan)];
    ++votesForRight[get<1>(sidSpan)];
  }



  (*stateIdToSpan)[sid] = resultingSpan;

  return resultingSpan;
}

/**

*/
template <class Arc>

                              StateIdToSpan* stateIdToSpan) {




    for (std::size_t i = 0, end = tailIds.size(); i < end; ++i) {
      StateIdToSpan::iterator found = stateIdToSpan->find(tailIds[i]);











    }
  }
}

/**


*/
template <class Arc>
struct InvalidArcSpansRemover {




    typename Arc::StateIdContainer const& tails = arc.tails();


    for (std::size_t i = 0, end = tails.size(); i < end; ++i) {
      StateIdToSpan::iterator found = m->find(tails[i]);
      const bool tailIsLexical = hg.hasLexicalLabel(tails[i]);

    }
  }


  IHypergraph<Arc> const& hg;
  StateIdToSpan* m;
};

/**


*/
template <class Arc>

  InvalidArcSpansRemover<Arc> fct(hg, stateIdToSpan);
  hg.forArcs(fct);






}

/**




*/
template <class Arc>

  // Run bubble-up first b/c the hg might contain better annotations
  // for smaller (i.e., lower) spans


  removeInvalidSpans(hg, stateIdToSpan);
}

template <class Arc>


                               SpanToStateIds const& resultSpanToStateIds,



  // Memoized result



  bool isUnion = false;
  const bool hasLexicalLabel = hg.hasLexicalLabel(head);

  StateIdToSpan::const_iterator foundSpan = hgStateIdToSpan.find(head);
  const bool didFindHeadSpan = foundSpan != hgStateIdToSpan.end();
  Span headSpan = didFindHeadSpan ? foundSpan->second : undefSpan;
  if (headSpan != undefSpan && !isSmallerSpan(headSpan, parentSpan) && !hasLexicalLabel) {
    headSpan = undefSpan;
  }




    std::size_t allTailsAreUnion = true;



                                         resultArcs, newStateInfos, opts);
      bool isUnion = p->first;

      newStatesForTails.push_back(p->second);
    }

    newStatesForTailsPerArc.push_back(newStatesForTails);
  }


  if (didFindHeadSpan && headSpan != undefSpan) {


    if (foundStates != resultSpanToStateIds.end()) {


        if (result->hasLexicalLabel(s)) {

            isUnion = true;
            newStates.push_back(s);

          }


          newStates.push_back(s);
        }
      }
    }

    NewStateInfo* info = new NewStateInfo(false, newStates);
    // return std::make_pair(false, newStates);
    (*newStateInfos)[head] = info;
    return info;
  }

  if (newStates.empty() || (opts.requirePathOverlap && !isUnion)) {


      std::stringstream ss;


    }
    StateId newState = result->addState(label, label);
    newStates.clear();
    newStates.push_back(newState);
  }

  // Add all resulting arcs to the result machine

    for (std::size_t a = 0; a < newStatesForTailsPerArc.size(); ++a) {

      for (std::size_t i = 0, end = newStatesForTails.size(); i < end; ++i) {

      }

      Util::cartesianProduct(newStatesForTails, &cartProd);
      for (std::size_t i = 0; i < cartProd.size(); ++i) {
        // avoid duplicate arcs

        arcStates.push_back(newHead);
        if (resultArcs->find(arcStates) == resultArcs->end()) {


          result->addArc(arc);
          resultArcs->insert(arcStates);
        }
      }
    }
  }




  // return std::make_pair(isUnion, newStates);
  NewStateInfo* info = new NewStateInfo(isUnion, newStates);
  (*newStateInfos)[head] = info;
  return info;
}

template <class Arc>


               SubUnionOptions& opts) {






      resultArcs.insert(v);
    }
  }

  // To memoize results
  std::map<StateId, NewStateInfo*> newStateInfos;






    delete it->second;
  }
}


/**



*/
template <class Arc>


  using namespace SubUnionUtil;








  StateIdToSpan stateIdToSpan1;
  getSourceSpans(hg1, &stateIdToSpan1);

  SpanToStateIds spanToStateIds1;

    spanToStateIds1[it->second].push_back(it->first);

  }


  StateIdToSpan stateIdToSpan2;
  getSourceSpans(hg2, &stateIdToSpan2);


  }

  copyHypergraph(hg1, result);
  addStates(hg2, stateIdToSpan2, result, spanToStateIds1, opts);

  // In addition, do normal union

}




#endif
