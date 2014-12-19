



















#define TRANSFORM_TO_STR(x) #x
#define TRANSFORM_NAME(x) TRANSFORM_TO_STR(x)














namespace Hypergraph {















































  bool initlogger;





















  }











    if (initlogger) {




    }
  }

  virtual void validate_parameters_extra() {
    initlog();
    validate_parameters_more();
  }

  virtual void validate_parameters_more() {}
};




#endif
