// Copyright 2014-2015 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/** \file

    moses format lattice output from hg lattice.
*/

#ifndef HYP__HYPERGRAPH_HGTOLATTICE
#define HYP__HYPERGRAPH_HGTOLATTICE
#include <deque>
#include <boost/unordered_map.hpp>
#include <sdl/Util/LogHelper.hpp>
#include <sdl/Hypergraph/MutableHypergraph.hpp>
#include <sdl/Hypergraph/SortStates.hpp>
#pragma once


// output the lattice in plf format with arc weight

namespace sdl {
namespace Hypergraph {

template<typename Arc> inline
void printHgAsMosesLattice(IHypergraph<Arc> const& hg){
  if(!hg.isFsm()){
    SDL_THROW_LOG(Hypergraph.PrintHgAsLattice, std::runtime_error, "Input Hypergraph should be FSM");
  }
  SortStatesOptions opt(SortStatesOptions::kTopSort);
  MutableHypergraph<Arc>* hg_sort = dynamic_cast<MutableHypergraph<Arc>* >(hg.clone());
  if(hg_sort == NULL){
    SDL_THROW_LOG(Hypergraph.PrintHgAsLattice, std::runtime_error, "Input Hypergraph should be mutable");
  }

  sortStates(*hg_sort, opt);
  SDL_DEBUG(Hypergraph.PrintHgAsLattice, *hg_sort);
  IVocabularyPtr pVoc = hg_sort->getVocabulary();
  std::cout << '(';
  for(StateId sid = hg_sort->start(), endid = hg_sort->maxNotTerminalState(); sid != endid; ++ sid) {
    std::cout << '(' ;
    forall (ArcId aid, hg_sort->outArcIds(sid)){
      Arc* arc = hg_sort->outArc(sid, aid);
      StateId next_sid = arc->head();
      int step = next_sid - sid;
      if(step < 0){
        SDL_THROW_LOG(Hypergraph.PrintHgAsLattice, std::runtime_error, "Illegal lattice input that cant be ordered");
      }
      Sym arc_label = hg_sort->inputLabel(arc->getTail(1));
      typename Arc::Weight w = arc->weight();

      std::cout << "(\'" << pVoc->str(arc_label) << "\', " << w.getValue() << ", "<< step << "), " ;

    } // foreach
    std::cout << "), " ;
  } // end while
  std::cout << ")\n";;
}


}}

#endif
