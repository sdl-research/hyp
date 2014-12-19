/** \file

    project: select only output labels or only input labels (result is an FSA or CFG).
*/

#ifndef HYPERGRAPH_PROJECT_HPP
#define HYPERGRAPH_PROJECT_HPP
#pragma once

#include <sdl/Hypergraph/IMutableHypergraph.hpp>
#include <sdl/Hypergraph/Transform.hpp>
#include <sdl/Util/Enum.hpp>

namespace sdl {
namespace Hypergraph {

/// for configuration "project-input" and "project-output"
SDL_ENUM(ProjectType, 2, (Project_Input, Project_Output))

template<class Arc>
void project(IMutableHypergraph<Arc>* hg,
             ProjectType projection) {
  switch(projection) {
    case kProject_Input:
      hg->projectInput(); return;
    case kProject_Output:
      hg->projectOutput(); return;
    default:
      SDL_ERROR(Hypergraph.Project, "unknown projection type: " << to_string_impl(projection));
  }
}

struct Project : SimpleTransform<Project, Transform::Inplace>, TransformOptionsBase
{
  Project() {}
  explicit Project(TransformOptionsBase const& base) {}

  ProjectType projection;

  template <class Config>
  void configure(Config &c) {
    c.is("Project");
    c("make an fsa from fst by taking either the input or output labels");
    c("projection", &projection)("use input or output labels").init(kProject_Output);
  }
  static char const* name() { return "Project"; }
  template <class Arc>
  void inplace(IMutableHypergraph<Arc> &hg) const {
    project(&hg, projection);
  }
};


}}

#endif
