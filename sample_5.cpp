// Copyright Sebastien Kramm 2016
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file sample_5.cpp
\brief Sample code
*/

#include <iostream>
#include "udgcd.hpp"

int prog_id = 5;
#include "common_sample.h"


/// the nodes


//-------------------------------------------------------------------
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
	add_edge(0, 3, g);
	add_edge(0, 6, g);
	add_edge(0, 8, g);

	add_edge(1, 2, g);
	add_edge(3, 2, g);

	add_edge(3, 4, g);
	add_edge(5, 4, g);
	add_edge(5, 6, g);

	add_edge(9, 8, g);
	add_edge(9, 7, g);
	add_edge(1, 7, g);

	add_edge(10, 5, g);
	add_edge(10, 11, g);
	add_edge(11, 9, g);

	RenderGraph( g );

	std::vector<std::vector<vertex_t>> cycles = udgcd::FindCycles<graph_t,vertex_t>( g );      // no cycles at first
	udgcd::PrintPaths( std::cout, cycles, "final-5" );
}
//-------------------------------------------------------------------
