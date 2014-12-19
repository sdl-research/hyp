








//could dump DeterminizeFlags and use this instead.





namespace Hypergraph {

struct DeterminizeOptions {
  DeterminizeFlags default_flags;
  bool EPSILON_NORMAL;
  bool RHO_NORMAL;
  bool PHI_NORMAL;
  bool SIGMA_NORMAL;

  DeterminizeOptions(DeterminizeFlags default_flags = DETERMINIZE_INPUT) : default_flags(default_flags), EPSILON_NORMAL(), RHO_NORMAL(), PHI_NORMAL(), SIGMA_NORMAL() {}
  DeterminizeFlags getFlags() const {
    DeterminizeFlags f = default_flags;

    HYP__DETOPTSETMASK(EPSILON_NORMAL);
    HYP__DETOPTSETMASK(RHO_NORMAL);
    HYP__DETOPTSETMASK(PHI_NORMAL);
    HYP__DETOPTSETMASK(SIGMA_NORMAL);
#undef HYP__DETOPTSETMASK
    return f;
  }





    HYP__DETOPT('E', epsilon, EPSILON);
    HYP__DETOPT('R', rho, RHO);
    HYP__DETOPT('P', phi, PHI);
    HYP__DETOPT('S', sigma, SIGMA);
#undef HYP__DETOPT

};



#endif
