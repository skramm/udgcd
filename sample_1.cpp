// Copyright Sebastien Kramm, 2016-2020
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file sample_1.cpp
\brief Sample code showing usage of udgcd.hpp
*/

#include <iostream>
#include "udgcd.hpp"

std::string prog_id = "s1";
#include "common_sample.h"


/*
  Modified example graph from Tarjei Knapstad.

                   H15
                   |
          H8       C2
            \     /  \
          H9-C0-C1    C3-O7-H14
            /   |     |
          H10   C6    C4
               /  \  /
              H11  C5---H13
                   |
                   H12
*/

/// the nodes
std::vector<std::string> names = {
	"C0", "C1", "C2", "C3", "C4", "C5", "C6", "O7",
	"H8", "H9", "H10", "H11", "H12", "H13", "H14", "H15"};

struct myVertex_t {
	std::string color_str;
	std::string node_name;
};

struct myEdge {
	int e1;
	std::string e2;
};

//-------------------------------------------------------------------------------------------
/// Some typedefs for readability... ;-)
typedef boost::adjacency_list<
	boost::vecS,                   // edge container
	boost::vecS,                   // vertex container
	boost::undirectedS,            // type of graph
	myVertex_t,                  // vertex properties
	myEdge
	> graph_t;

	typedef boost::graph_traits<graph_t>::vertex_descriptor vertex_t;
	typedef boost::graph_traits<graph_t>::edge_descriptor   edge_t;

//-------------------------------------------------------------------
template<typename graph_t>
void
AssignRandomColors( graph_t& g )
{
	auto it_pair = vertices( g );              // get iterator range
	for( ; it_pair.first != it_pair.second; ++it_pair.first ) // we iterate using the first iterator
	{
		std::ostringstream oss;
		oss << '#' << std::hex;
		for( int i=0; i<3; i++ )
			oss << static_cast<int>(255.0 * std::rand() / RAND_MAX );
		g[*it_pair.first].color_str = oss.str();
	}
}
//-------------------------------------------------------------------
template<typename graph_t>
void
AssignNodeNames( graph_t& g, const std::vector<std::string>& names )
{
	assert( boost::num_vertices( g ) == names.size() );

	auto it_str = names.cbegin();
	auto it_pair = boost::vertices( g );              // get iterator range
	for( ; it_pair.first != it_pair.second; ++it_pair.first ) // we iterate using the first iterator
		g[*it_pair.first].node_name = *it_str++;
}

//-------------------------------------------------------------------
int main()
{
	SHOW_INFO;

	graph_t g( names.size() );

	add_edge(0, 1, g);
	add_edge(0, 8, g);
	add_edge(0, 9, g);
	add_edge(0, 10, g);
	add_edge(1, 2, g);
	//  add_edge(1, 6, g); // remove the initial cycle
	add_edge(2, 15, g);
	add_edge(2, 3, g);
	add_edge(3, 7, g);
	add_edge(3, 4, g);
	add_edge(5, 13, g);
	add_edge(4, 5, g);
	add_edge(5, 12, g);
	add_edge(5, 6, g);
	add_edge(6, 11, g);
	add_edge(7, 14, g);

	AssignRandomColors( g );
	AssignNodeNames( g, names );
	RenderGraph( g, prog_id );

	//std::vector<std::vector<vertex_t>> cycles = udgcd::findCycles<graph_t,vertex_t>( g );      // no cycles at first
	auto cycles = udgcd::findCycles<graph_t,vertex_t>( g );      // no cycles at first
	udgcd::PrintPaths( std::cout, cycles, "final-0" );

	add_edge( 1, 6, g );                                 // cycle !
	cycles = udgcd::findCycles<graph_t,vertex_t>( g );

//	RenderGraph3<graph_t,myVertex_t>( g );
	RenderGraph( g, "s1" );

	udgcd::PrintPaths( std::cout, cycles, "final-1" );

	add_edge( 13, 14, g );                              // another cycle !
	RenderGraph( g, prog_id );
	cycles = udgcd::findCycles<graph_t,vertex_t>( g );
	udgcd::PrintPaths( std::cout, cycles, "final-2" );

	add_edge( 15, 8, g );                               // another cycle !
	RenderGraph( g, prog_id );
	cycles = udgcd::findCycles<graph_t,vertex_t>( g );
	udgcd::PrintPaths( std::cout, cycles, "final-3" );

	add_edge( 15, 8, g );                               // add a second arc between same vertices, does not add a path
	RenderGraph( g, prog_id );
	cycles = udgcd::findCycles<graph_t,vertex_t>( g );
	udgcd::PrintPaths( std::cout, cycles, "final-4" );

	return 0;
}

//-------------------------------------------------------------------

