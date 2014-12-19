


    determinize(i, &o);

    where i is hypergraph that happens to be an unweighted fsa, with epsilons and phi only,
    and o is mutable hypergraph





    DFA < UFA < FNA <= PNA < NFA

    UFA: unambiguous
    FNA: finitely nondet.
    PNA: polynomially ambig.





    ambiguity of string: # of accepting runs ; max ambig of str of len n is nondecreasing with n
    UFA: ambig=0 or 1
    FNA or PNA: ambig(n) is O(k) or O(x^k) resp
    SEA: strictly exponentially ambig: ambig(n)=Theta(k^n) (i.e. ENA and not PNA)
    SEA iff exists useful state q and string w so that q->w->q ambiguously (>1 run)

    (from http://lrb.cs.uni-dortmund.de/~martens/data/icalp08.pdf )
    UFA: ptime subset, equivalence, but:

    NP-complete: min UFA L= UFA, min UFA L= DFA
    PSPACE-complete: min NFA L= DFA

    DFA L= NFA is expsize output but low polynomial (in size of output) construction

    DFA->X and X->X min is NP-complete for many classes between X DFA and NFA

    1. choose between >1 init states, then deterministic

    or 2. constant # of runs per string




    or 3b. thereforeeven DFA-with-2-start-states

    (are all NP-complete to minimize for any FA class including them)




*/

/* meanings:

   epsilon and phi don't advance the input pointer when taking the arc (rho and sigma do).

   epsilon and sigma match any symbol;
   phi and rho are "else" - they match any symbol not explicitly named on another outgoing arc
*/
#ifndef HYPERGRAPH_DETERMINIZE_HPP
#define HYPERGRAPH_DETERMINIZE_HPP



































#else

#endif


namespace Hypergraph {




struct IsDetState {  // unlike DupSymbols, allow epsilon to final only!

  StateId final;








  template <class A>




  }
};

template <class A>
bool isDeterminized(StateId s, IHypergraph<A> const& i, bool allow_epsilon = false) {
  if (i.prunedEmpty()) return true;



}

template <class A>
bool isDeterminized(IHypergraph<A> const& i, bool allow_epsilon = false) {
  if (i.prunedEmpty()) return true;

  for (StateId s = 0; s < ns; ++s)



  return true;
}

template <class A>
bool containsEmptyStringDet(IHypergraph<A> const& i) {
  assert(isDeterminized(i));

  if (st == f) return true;



  }
  return false;
}

typedef boost::uint32_t DeterminizeFlags;
const DeterminizeFlags DETERMINIZE_INPUT = 1;
const DeterminizeFlags DETERMINIZE_OUTPUT = 2;
const DeterminizeFlags DETERMINIZE_FST = 4;  // TODO
const DeterminizeFlags DETERMINIZE_EPSILON_NORMAL = 8;
const DeterminizeFlags DETERMINIZE_EPS_NORMAL = DETERMINIZE_EPSILON_NORMAL;
const DeterminizeFlags DETERMINIZE_RHO_NORMAL = 0x10;
const DeterminizeFlags DETERMINIZE_PHI_NORMAL = 0x20;
const DeterminizeFlags DETERMINIZE_SIGMA_NORMAL = 0x40;

#define DET_SPECIAL_SYMBOL(t, x) (!(t & DETERMINIZE_##x##_NORMAL))
#define IS_DET_SPECIAL_SYM(s, t, x) (s == x::ID && DET_SPECIAL_SYMBOL(t, x))


typedef std::vector<StateId> QSet;  // prevent duplicates via seenq
#else
typedef std::set<StateId> QSet;  // set instead of unordered_set for better hashing/equality
#endif
typedef QSet const* Qp;

DEFINE_PRINTRANGE(QSet)




#else

#endif
inline std::ostream& operator<<(std::ostream& o, Explored::value_type const& p) {
  o << '(' << *p.first << ',' << p.second << ')';
  return o;
}

struct ToDo {
  Qp qs;  // states
  StateId q;  // result-state
  inline friend std::ostream& operator<<(std::ostream& o, ToDo const& t) {
    return o << "ToDo:" << t.q << "=" << *t.qs;
  }
};
typedef std::vector<ToDo> Agenda;
DEFINE_PRINTRANGE(Agenda)












template <class A>
struct DeterminizeFsa {
  IHypergraph<A> const& i;  // input
  IMutableHypergraph<A>* o;  // output
  DeterminizeFlags flags;
  Graph eps;
  StateSet expanded;

  StateSet seenq;  // SINGLE THREADED
  template <class I>
  void addqs(QSet& qs, I i, I const& end, bool& anyadded) {
    for (; i != end; ++i)


        anyadded = true;
      }
  }
  void clearseen(QSet const& qs) {

  }

  bool checkseen(QSet const& qs) const {
    for (QSet::const_iterator i = qs.begin(), e = qs.end(); i != e; ++i)

    return true;
  }
  void setseen(QSet const& qs) const {

  }
  bool checkseeniff(QSet const& qs) {
    for (QSet::const_iterator i = qs.begin(), e = qs.end(); i != e; ++i)
      if (test(seenq, *i))
        reset(seenq, *i);
      else {
        for (QSet::const_iterator j = qs.begin(); j != i; ++j)  // undo resets

        return false;
      }
    bool ret = true;

    for (QSet::const_iterator i = qs.begin(), e = qs.end(); i != e; ++i)  // undo resets

    return ret;
  }
  QSet& uniq(QSet& qs) {  // doesn't clear qseen! result to be passed to subsetId

    removeShrink(qs, removeDup(seenq));
    return qs;
  }
#endif
  template <class C>
  void addqs(QSet& qs, C const& c, bool& anyadded) {

    addqs(qs, c.begin(), c.end(), anyadded);
#else

#endif
  }





  StateId qfinali;

  typedef std::vector<Arc> Arcs;


  typedef std::vector<StateId> States;
  struct Outi {
    Letters normals;
    Arcs arcs;  // no specials.
    States qrhos;  // "else" dest states
  };
  struct has_rhos {
    bool operator()(Outi const& p) { return !p.qrhos.empty(); }
  };

  Explored explored;



  Rhos rhos;
  Agenda agenda;

  std::vector<StateId> qfinals;
  bool specialEps;
  DeterminizeFsa(IHypergraph<A> const& i, IMutableHypergraph<A>* o, DeterminizeFlags flags)







#else

#endif
  {
    assert(i.isFsm());

    specialEps = DET_SPECIAL_SYMBOL(flags, EPSILON) && i.anyInputLabel(isEps());
    if (specialEps) {
      eps.addFsmArcs(i, isEps());
      eps.transitiveClose();
    }
    initRhos();
    QSet* s = qsp.construct();


    finish_agenda();




      StateId f;
      if (nf == 1)
        f = qfinals[0];
      else {
        f = o->addState();

      }

    }
  }
  bool isFinal(QSet const& qs) {

  }


  // in all cases, post: seenq==0



  StateId subsetId(QSet& qs) {  // memoized before and after e-closure!







#else
#define FORCE_CLEAR(qs)
#define FORCE_SET(qs)
#endif
// because of uniq leaving qs' bits set:


#else
    FORCE_CLEAR(qs);
#endif
    StateId* id;

      FORCE_CLEAR(qs);

      return *id;
    }
    QSet* pcqs = &qs, *newpcqs = 0;
    bool anyadded = false;
    if (specialEps) {
      newpcqs = qsp.construct(qs);
      FORCE_SET(qs);

        // if using BitSet, use |= op
        addqs(*newpcqs, eps.outs[s], anyadded);


      }




      }
    }
    // POST: anyadded iff need to destroy pcqs if it's not newly added to explored








      ToDo t;
      t.qs = pcqs;









    }

    FORCE_CLEAR(*pcqs);

  }




#define DEREF_DELTA
#else
  typedef QSet* Qmutp;


    Qmutp* pp;

    return **pp;
  }
#define DEREF_DELTA *
#endif


    StateId i = subsetId(dqs);
    o->addArcFsa(q, i, s);
  }
  // happens once per qs, because new things go on agenda only once (atomic w/ marking it 'explored')
  void genOut(QSet const& qs, StateId q) {

    Delta d;
    std::vector<StateId> qrho;
    QSet& qallrho = *qsp.construct();
    bool addrhodummy;

      Outi const& p = rhos[s];
      if (!p.qrhos.empty()) {
        qrho.push_back(s);  // process later once full alphabet for qs is known
        addqs(qallrho, p.qrhos, addrhodummy);
      }


      }
    }
    if (qrho.empty())

    else
      addArc(q, qallrho, RHO::ID);  // handles all symbols not mentioned in any outgoing arc of qs.


      QSet& dqs = DEREF_DELTA v.second;

        Outi const& p = rhos[q];



      }

// will uniq subsets, then add later
#else
      addArc(q, dqs, i);
#endif
    }


#endif
  }

  void finish_agenda() {
    while (!agenda.empty()) {


      genOut(*t.qs, t.q);
    }
  }

  struct addLetter {
    DeterminizeFlags t;
    Outi& l;
    addLetter(DeterminizeFlags t, Outi& l) : t(t), l(l) {}


      if (IS_DET_SPECIAL_SYM(s, t, RHO))

      else if (IS_DET_SPECIAL_SYM(s, t, EPSILON)) {
      } else if (IS_DET_SPECIAL_SYM(s, t, PHI) || IS_DET_SPECIAL_SYM(s, t, SIGMA)) {



      } else {


      }
    }
  };
  void initRhos() {


    for (StateId s = 0; s < N; ++s) {
      i.forArcsFsa(s, addLetter(flags, rhos[s]));


    }
  }
};

struct NotDeterminized {
  DeterminizeFlags flags;

    return IS_DET_SPECIAL_SYM(i, flags, PHI) || IS_DET_SPECIAL_SYM(i, flags, SIGMA);
    // epsilon (I handle subset-e-closure automatically), and rho (an "else, consuming") are both ok!
  }
};

template <class Arc>
void determinize_always(IHypergraph<Arc> const& i,  // input
                        IMutableHypergraph<Arc>* o,  // already has desired props


  typedef MutableHypergraph<Arc> H;

  HP fsap;
  IHypergraph<Arc> const* ip = &i;  // copy only if needed
  o->setVocabulary(i.getVocabulary());

  bool dinput = flags & DETERMINIZE_INPUT;
  bool doutput = flags & DETERMINIZE_OUTPUT;
  bool dfst = flags & DETERMINIZE_FST;
  if (dinput && i.hasOutputLabels() || doutput) {
    H* f = new H();
    fsap = HP(f);
    copyHypergraph(i, f);


    ip = f;
  }
  if (doutput && dinput || dfst)




  DeterminizeFsa<Arc> det(*ip, o, flags);
  assert(isDeterminized(*o));
}

template <class Arc>

  NotDeterminized nd;
  nd.flags = flags;
  if (i.anyInputLabel(nd)) {



  }
  if (!i.unweighted())


  if (!i.isFsm())


}

template <class Arc>
void determinize(IHypergraph<Arc> const& i,  // input
                 IMutableHypergraph<Arc>* o,  // output







  assertCanDeterminize(i, flags);
  if (isDeterminized(i, !DET_SPECIAL_SYMBOL(flags, EPSILON))) {
    copyHypergraph(i, o);
    DWARN("determinize: already determinized - simply copying.");
    return;
  }
  determinize_always(i, o, flags);
}


  DeterminizeFlags flags;
  Properties prop_on, prop_off;




  Properties outAddProps() const { return prop_on; }
  Properties outSubProps() const { return prop_off; }
  template <class H>
  bool needs(H const& h) const {

    assertCanDeterminize(h, flags);
    return true;
  }






    determinize_always(i, o, flags);
  }
};

template <class A>




  Determinize d(flags, newOn, newOff);
  inplace(p, d);
  return *p;
}

template <class A>





  Determinize d(flags, newOn, newOff);
  inplace(p, d);
  return p;
}














