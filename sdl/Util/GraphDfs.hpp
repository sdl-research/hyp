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

    simple boost graph for Hypergraph::empty.
*/


#ifndef SDL_UTIL_GRAPH_DFS_HPP
#define SDL_UTIL_GRAPH_DFS_HPP
#pragma once

#include <boost/graph/graph_traits.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/properties.hpp>
#include <graehl/shared/property_factory.hpp>

namespace sdl {
namespace Util {


typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> Graph;
// typedef boost::graph_traits<Graph>::vertices_size_type GraphSize;
typedef Graph::vertices_size_type GraphSize;

typedef boost::default_color_type GraphColor;
inline GraphColor unvisitedColor() {
  return boost::color_traits<GraphColor>::white();
}
inline GraphColor visitedColor() {
  return boost::color_traits<GraphColor>::black();
}

typedef std::vector<boost::default_color_type> VertexColors;
typedef boost::iterator_property_map<VertexColors::iterator, boost::identity_property_map> VertexColorMap;


template <class Graph, class Colormap>
void dfsColorMap(Graph const& g, Colormap const& c, typename boost::graph_traits<Graph>::vertex_descriptor from) {
  boost::depth_first_visit(g, from, boost::default_dfs_visitor(), c);
}

inline void dfsColor(Graph const& g, VertexColors& c, std::size_t from) {
  c.clear();
  c.resize(boost::num_vertices(g), unvisitedColor());  // white: yet to be discovered
  VertexColorMap cm(c.begin());
  dfsColorMap(g, cm, from);
}


}}

#endif
