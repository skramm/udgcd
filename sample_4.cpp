// Copyright Sebastien Kramm 2016
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file sample_4.cpp
\brief Sample code
*/

#include <iostream>
#include "udgcd.hpp"

int prog_id = 4;
#include "common_sample.h"


/// the nodes


//-------------------------------------------------------------------------------------------
/// Some typedefs for readability... ;-)
typedef boost::adjacency_list<
	boost::vecS,
	boost::vecS,
	boost::undirectedS
	> graph_t;

	typedef boost::graph_traits<graph_t>::vertex_descriptor vertex_t;
	typedef boost::graph_traits<graph_t>::edge_descriptor   edge_t;

//-------------------------------------------------------------------
int main(int, char*[])
{
	SHOW_INFO;

	graph_t g;

	add_edge(0, 1, g);
add_edge(0, 2, g);
add_edge(0, 3, g);
add_edge(1, 2, g);
add_edge(2, 3, g);
add_edge(1, 5, g);
add_edge(3, 5, g);
add_edge(6, 7, g);
add_edge(7, 8, g);
add_edge(8, 6, g);

//	RenderGraph( g );

	std::vector<std::vector<vertex_t>> cycles = udgcd::FindCycles<graph_t,vertex_t>( g );      // no cycles at first
	udgcd::PrintPaths( std::cout, cycles, "final-0" );

//	add_edge( 1, 6, g );                                 // cycle !
	cycles = udgcd::FindCycles<graph_t,vertex_t>( g );
	RenderGraph( g );
	udgcd::PrintPaths( std::cout, cycles, "final-1" );

/*	add_edge( 13, 14, g );                              // another cycle !
	RenderGraph( g );
	cycles = udgcd::FindCycles<graph_t,vertex_t>( g );
	udgcd::PrintPaths( std::cout, cycles, "final-2" );
*/
/*	add_edge( 15, 8, g );                               // another cycle !
	RenderGraph( g );
	cycles = udgcd::FindCycles<graph_t,vertex_t>( g );
	udgcd::PrintPaths( std::cout, cycles, "final-3" );
*/

	return 0;
}

//-------------------------------------------------------------------

