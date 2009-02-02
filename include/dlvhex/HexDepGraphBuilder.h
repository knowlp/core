/* dlvhex -- Answer-Set Programming with external interfaces.
 * Copyright (C) 2005, 2006, 2007 Roman Schindlauer
 * Copyright (C) 2008, 2009 Thomas Krennwallner, DAO Tran Minh
 * 
 * This file is part of dlvhex.
 *
 * dlvhex is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * dlvhex is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with dlvhex; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


/**
 * @file HexDepGraphBuilder.h
 * @author Thomas Krennwallner
 * @author DAO Tran Minh
 * @date Mon Feb 02 10:54:24 CET 2009
 *
 * @brief Builder for HEX dependency graphs
 *
 *
 */


#if !defined(_DLVHEX_HEXDEPGRAPH_BUILDER_H)
#define _DLVHEX_HEXDEPGRAPH_BUILDER_H

#include "dlvhex/PlatformDefinitions.h"
#include "dlvhex/HexDepGraph.h"
#include "dlvhex/DepGraphBuilder.tcc"

#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>

DLVHEX_NAMESPACE_BEGIN

// forward declarations
class PluginContainer;
class Program;


class DLVHEX_EXPORT HexDepGraphBuilder 
: public DepGraphBuilder<HexDepGraph, HexDepGraphType::Vertex, HexDepGraphType::Edge, HexDepGraphType::VertexProperty, HexDepGraphType::EdgeProperty>
{
 protected:
  boost::shared_ptr<HexDepGraph> dg;
  
 public:

 HexDepGraphBuilder() : dg(new HexDepGraph)
    { }
  
  
  virtual boost::shared_ptr<HexDepGraph>
    getDepGraph() const
    {
      return dg;
    }
  
  
  virtual HexDepGraphType::Vertex
    buildVertex(HexDepGraphType::VertexProperty& vp)
    {
      return boost::add_vertex(vp, *dg);
    }
  
  
  virtual HexDepGraphType::Edge
    buildEdge(HexDepGraphType::Vertex u, HexDepGraphType::Vertex v, HexDepGraphType::EdgeProperty& ep)
    {
      std::pair<HexDepGraphType::Edge, bool> eret = boost::add_edge(u, v, ep, *dg);
      return eret.first;
    }
};

DLVHEX_NAMESPACE_END

#endif /* _DLVHEX_HEXDEPGRAPH_BUILDER_H */


// Local Variables:
// mode: C++
// End: