































#include <utility>
#include <queue>
#include <set>


namespace Hypergraph {

/**


*/
template<class FstArc, class HgArc>
fst::ReplaceFst<FstArc>*
toReplaceFst(IHypergraph<HgArc> const& hg, fst::SymbolTable* fsyms) {


                  "Needs incoming arcs");
  }
  std::queue<StateId> queue;
  std::set<StateId> onQueue;


  queue.push(finalState);
  onQueue.insert(finalState);

  typedef typename FstArc::Label FLabel;
  typedef typename FstArc::StateId FStateId;
  typedef typename FstArc::Weight FWeight;
  typedef std::pair<FLabel, const fst::Fst<FstArc>*> LabelFstPair;
  std::vector<LabelFstPair> fsts;

  fsyms->AddSymbol(EPSILON::TOKEN); // in OpenFst, eps must be first (=0)
  std::string root =

  FLabel fRootLabel = fsyms->AddSymbol(root);

  while (!queue.empty()) {
    StateId sid = queue.front();

              "Converting HG state " << sid << " to FST");
    queue.pop();

    // Each HG state gets an FST "nonterminal"

    FLabel nonterm = fsyms->AddSymbol(token);
    fst::MutableFst<FstArc>* fst = new fst::VectorFst<FstArc>();
    FStateId startState = fst->AddState();
    fst->SetStart(startState);

    // Each HG arc has tails, which are converted into a sequence in an FST


      FStateId prevState = startState;
      for (std::size_t t = 0, end = arc->getNumTails(); t < end; ++t) {
        StateId tailId = arc->getTail(t);
        FLabel flabel;
        std::string token;
        if (hg.hasLexicalLabel(tailId)) {

        }
        else {
          // Nonterminal: Gets its own Fst

          if (onQueue.find(tailId) == onQueue.end()) {
            queue.push(tailId);
            onQueue.insert(tailId);
          }
        }
        flabel = fsyms->AddSymbol(token);
        FStateId nextState = fst->AddState();
        FWeight fweight =

        fst->AddArc(prevState, FstArc(flabel, flabel, fweight, nextState));
        prevState = nextState;
      }
      fst->SetFinal(prevState, FWeight::One());
    }

    fsts.push_back(LabelFstPair(nonterm, fst));
  }


  fst::ReplaceFst<FstArc>* result =
      new fst::ReplaceFst<FstArc>(fsts, fRootLabel);
  // return std::make_pair(result, fsyms);
  return result;
}

template<class FstArc, class HgArc>
std::pair<fst::ReplaceFst<FstArc>*, fst::SymbolTable*>
toReplaceFst(IHypergraph<HgArc> const& hg) {
  fst::SymbolTable* syms = new fst::SymbolTable("");
  fst::ReplaceFst<FstArc>* result = toReplaceFst<FstArc>(hg, syms);
  return std::make_pair(result, syms);
}






#endif
