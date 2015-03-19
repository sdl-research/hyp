/** \file

    simple boost graph for Hypergraph::empty.
*/


#ifndef SDL_UTIL_GRAPH_DFS_HPP
#define SDL_UTIL_GRAPH_DFS_HPP
#pragma once

#include <boost/graph/graph_traits.hpp>
#include <sdl/Util/Forall.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/properties.hpp>
#include <graehl/shared/property_factory.hpp>

namespace sdl {
namespace Util {


typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> Graph;
//typedef boost::graph_traits<Graph>::vertices_size_type GraphSize;
typedef Graph::vertices_size_type GraphSize;

typedef boost::default_color_type GraphColor;
inline GraphColor unvisitedColor()
{
  return boost::color_traits<GraphColor>::white();
}
inline GraphColor visitedColor()
{
  return boost::color_traits<GraphColor>::black();
}

typedef std::vector<boost::default_color_type> VertexColors;
typedef boost::iterator_property_map<VertexColors::iterator, boost::identity_property_map> VertexColorMap;


template <class Graph, class Colormap>
void dfsColorMap(Graph const&g, Colormap const& c, typename boost::graph_traits<Graph>::vertex_descriptor from)
{
  boost::depth_first_visit(g, from, boost::default_dfs_visitor(), c);
}

inline void dfsColor(Graph const&g, VertexColors &c, std::size_t from)
{
  c.clear();
  c.resize(boost::num_vertices(g), unvisitedColor()); // white: yet to be discovered
  VertexColorMap cm(c.begin());
  dfsColorMap(g, cm, from);
}


}}

#endif
