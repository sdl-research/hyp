















#include <fst/symbol-table.h>
#include <fst/vector-fst.h>
#include <fst/script/fst-class.h>









namespace Hypergraph {



// read-only access to an IVocabulary using openfst SymbolTable interface


  typedef fst::SymbolTable Base;
  typedef IVocabularySymbolTable self_type;
  IVocabularyPtr pVoc;


  IVocabularySymbolTable(self_type const& o) : Base("IVocabularySymbolTable"), pVoc(o.pVoc) {}
  typedef std::string string;


  virtual string Find(int64 key) const OVERRIDE {


  }

  virtual int64 AddSymbol(string const&) OVERRIDE {


  }
  virtual int64 AddSymbol(string const&, int64) OVERRIDE {


  }

  virtual string CheckSum() const OVERRIDE {


  }
  virtual string LabeledCheckSum() const OVERRIDE {


  }



  }
  virtual bool WriteText(std::ostream& strm) const OVERRIDE {






















  }
};





















template <class Arc, class FArc = fst::ArcTpl<typename Arc::Weight> >

  IVocabularySymbolTable syms;



  typedef typename Arc::Weight Weight;
  typedef FArc FstArc;
  typedef typename FstArc::Weight FWeight;
  IHypergraph<Arc> const& h;
  fst::VectorFst<FstArc> fst;





  }



    fst.DeleteStates();
    fst.SetInputSymbols(&syms);
    fst.SetOutputSymbols(&syms);

      StateId s = fst.AddState();
      assert(s == i);
    }




  }








};







