/** \file

    configurable contiguous range of feature ids.
*/

#ifndef FEATUREIDRANGE_JG2013130_HPP
#define FEATUREIDRANGE_JG2013130_HPP
#pragma once

#include <iostream>

#include <sdl/Hypergraph/Types.hpp>
#include <sdl/Util/LogHelper.hpp>

namespace sdl { namespace Hypergraph {

/**
   contiguous range of feature ids.
*/
struct FeatureIdRange {
  struct Unlimited {};
  FeatureId begin, end;

  FeatureIdRange()
      : begin(), end() {}

  FeatureIdRange(FeatureId begin, FeatureId end)
      : begin(begin), end(end) {}

  FeatureIdRange(Unlimited)
      : begin(), end(~(FeatureId)0) {}

  FeatureId offset(FeatureId id) const {
    assert(contains(id));
    return id-begin;
  }

  FeatureId id(FeatureId offset) const {
    assert(offset+begin<end);
    return offset+begin;
  }

  bool contains(FeatureId id) const {
    return id >= begin && id < end;
  }

  bool containsMaxOffset(FeatureId id, FeatureId maxOffset) const {
    return contains(id) && offset(id)<maxOffset;
  }

  bool enabled() const { return end > begin; }

  template <class Config>
  void configure(Config &config) {
    config.is("Feature Id Range");
    config("select features with half-open interval begin <= id < end; (end-begin) feature ids");
    config("begin", &begin)("minimum (inclusive) feature id").self_init();
    config("end", &end)("id one higher than the maximum (end-1). 0 disables the range completely").self_init();
  }

  FeatureId size() const { return end-begin; }

  void requireSize(double needed, char const* prefix="FeatureIdRange required");

  bool disjoint(FeatureIdRange const& o) const {
    return !o.enabled() || !enabled() || o.end <= begin || end <= o.begin;
  }

  friend
  std::ostream& operator<<(std::ostream &out,
                           FeatureIdRange const& x);

  FeatureId historyRequires(unsigned order) const {
    return (order - 1) * (size() + 1);
  }

  double tupleRequiresDouble(unsigned order);

  FeatureId tupleRequires(unsigned order);

  void setSize(FeatureId size) {
    end = begin+size;
  }
};

}}

#endif
