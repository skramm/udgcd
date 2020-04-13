// Copyright Sebastien Kramm, 2016-2020
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file sample_2.cpp
\brief Sample code demonstrating when some nodes are NOT connected. Also uses a "no-property" graph.

*/

#include <iostream>
#include "udgcd.hpp"

std::string prog_id = "s2";
#include "common_sample.h"

/*
  test graph

 N0   N1-N2  N4-N5  N6-N7
       \ /           \ /
        N3            N8
*/


//-------------------------------------------------------------------------------------------
/// Some typedefs for readability... ;-)
typedef boost::adjacency_list<
	boost::vecS,
	boost::vecS,
	boost::undirectedS
	> graph_t;

	typedef boost::graph_traits<graph_t>::vertex_descriptor vertex_t;

//-------------------------------------------------------------------
int main(int, char*[])
{
	SHOW_INFO;

	graph_t g;

	add_edge(2, 1, g);
	add_edge(2, 3, g);
	add_edge(3, 1, g);

	add_edge(4, 5, g);

	add_edge(6, 7, g);
	add_edge(8, 7, g);
	add_edge(8, 6, g);

	add_edge(9, 4, g);

	RenderGraph( g, prog_id );

	auto cycles = udgcd::findCycles<graph_t,vertex_t>( g );
	udgcd::PrintPaths( std::cout, cycles );

	return 0;
}
//-------------------------------------------------------------------
