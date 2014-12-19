#include <cmath>





namespace Hypergraph {

void FeatureIdRange::requireSize(double needed, char const* prefix) {




                  prefix << " " << needed << " ids; you allowed only " << n << " in " << *this);
  end = begin + (FeatureId)(needed + epsilon);
}

std::ostream& operator<<(std::ostream &out,











double FeatureIdRange::tupleRequiresDouble(unsigned order) {







FeatureId FeatureIdRange::tupleRequires(unsigned order) {




                  "tuple order " << order << " for context "<<*this << " would result in too many features (" << r<<")");





