








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






    return f;
  }











};



#endif
