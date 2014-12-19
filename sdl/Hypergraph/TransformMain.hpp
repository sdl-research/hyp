
































*/
































namespace Hypergraph {

struct TransformMainBase : HypergraphMainBase {











  typedef ExpectationWeight expectationSemiring;
  typedef LogWeightTpl<float> logSemiring;
  typedef ViterbiWeightTpl<float> viterbiSemiring;
  typedef FeatureWeight featureSemiring;

  std::string arcType;
  graehl::hex_int<Properties> default_properties;














      , allowedSemirings(semirings)






  semirings_type allowedSemirings;

  // these control the appearance of cmdline options that activate
  // alternative input options and output options (lines = paths, or
  // multiple input hgs)
  bool bestOutputs;





  static std::string semiringsList(semirings_type x) {
    std::ostringstream o;
    graehl::word_spacer sp('|');
    if (x & viterbi) o << sp << "viterbi";
    if (x & log) o << sp << "log";
    if (x & expectation) o << sp << "expectation";

    return o.str();
  }

  static semirings_type semiringFor(std::string const& s) {
    if (s == "viterbi") return viterbi;
    if (s == "log") return log;
    if (s == "expectation") return expectation;
    if (s == "feature") return feature;





  }





































  typedef ArcTpl<featureSemiring> featureArc;




  void configure(Config& c) {


















  void finish_configure_more() OVERRIDE {


      this->configurable(&optBestOutputs);






  }


    semirings_type sr = semiringFor(arcType);
    if (!(sr & allowedSemirings))


  }
};

template <class CRTP>  // google CRTP if you're confused
struct TransformMain : TransformMainBase {



























































    std::string const& w = this->arcType;
    bool r;


    else if (w == "log")
      r = runWeight<logSemiring>();


    else if (w == "expectation")
      r = runWeight<expectationSemiring>();

    else if (w == "feature")
      r = runWeight<featureSemiring>();
    else


      warn("Aborted early (transform returned false).");



  }



  // if has_*input_transform (can override directly):
  template <class Arc>




    MutableHypergraph<Arc>* m = new MutableHypergraph<Arc>(impl().properties(n));
    h.reset(m);
    return impl().inputTransform((IHypergraph<Arc> const&)*i, m, n);
  }
  template <class Arc>



    return true;
  }
  template <class Arc>




    return true;
  }

  // if has_*transform1, override either this or one of below
  template <class Arc>



    MutableHypergraph<Arc>* m = new MutableHypergraph<Arc>(impl().properties(0));
    h.reset(m);
    return impl().transform1((IHypergraph<Arc> const&)*i, m);
  }
  // if has_inplace_transform1
  template <class Arc>
  bool transform1InPlace(IMutableHypergraph<Arc>& h) {

  }
  // else just has_transform1
  // most unary transform will ONLY override this
  template <class Arc>
  bool transform1(IHypergraph<Arc> const& i, IMutableHypergraph<Arc>* o) {

    return true;
  }

  // if has2().
  // First ptr is taken as result for reduction/fold of N Hgs to
  // 1. please don't meaningfully modify i2.
  template <class Arc>



    io.reset(new MutableHypergraph<Arc>(impl().properties(0)));

    return impl().transform2pp(i, i2, io.get());
  }

  // if has_inplace_transform2
  template <class Arc>
  bool transform2InPlace(IMutableHypergraph<Arc>& io, IMutableHypergraph<Arc>& i2) {
    return impl().transform2InPlacec(io, i2);
  }

  template <class Arc>
  bool transform2InPlacec(IMutableHypergraph<Arc>& io, IHypergraph<Arc> const& i2) {


    return true;
  }

  // else:

  template <class Arc>




    return impl().transform2mm(*i, *i2, o);
  }

  template <class Arc>

    return impl().transform2mc(i, (IHypergraph<Arc> const&)i2, o);
  }

  template <class Arc>

    return impl().transform2((IHypergraph<Arc> const&)i, i2, o);
  }

  // most will override this:
  template <class Arc>



    return true;
  }

  bool has2() const { return impl().has_inplace_transform2 || impl().has_transform2; }
  bool has1() const { return impl().has_inplace_transform1 || impl().has_transform1; }



  bool outEvery() const {
    assert(!impl().out_every_default());
    return false;
  }














  struct Cascade {

    typedef MutableHypergraph<Arc> H;
















    bool transformInput(unsigned inputLine, bool reload, bool free) {

      std::string olast_name;



      std::ostringstream o_name;

      Util::Sep oname_sep = main.impl().transform2sep();













          parseText(*in, in.name, h.get());


            bool inputOk = impl().inputTransformInPlaceP(h, input);


          }







          o_name << oname_sep << in.name;





        }


      }
      if (impl().has1()) {


      }

      return true;
    }


  /// Override if desired: it's guaranteed that impl().prepare will be
  /// called for exactly one Arc type - the same as in all the
  /// transform* calls.  this may e.g. create a TransformHolder or
  /// other shared_ptr<void> that's used across all inputs.

  void prepare() {}








    }


























};




#endif
