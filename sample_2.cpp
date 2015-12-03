// Copyright Sebastien Kramm 2015
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file sample_2.cpp
\brief Sample code demonstrating when some nodes are NOT connected

In some situations, we can have a loop but explore can fail if the starting node DOES NOT lead to that loop
*/

#include <iostream>
#include "udgcd.hpp"

int prog_id = 2;
#include "common_sample.h"

/*
  test graph

 N0   N1-N2  N4-N5  N6-N7
       \ /           \ /
        N3            N8
*/

// the nodes
//std::vector<std::string> name = { "N0", "N1", "N2", "N3", "N4", "N5", "N6", "N6", "N7", "N8" };

//-------------------------------------------------------------------------------------------
/// Some typedefs for readability... ;-)
typedef boost::adjacency_list<
	boost::vecS,
	boost::vecS,
	boost::undirectedS,
	boost::no_property,
	boost::property<
		boost::edge_color_t,
		boost::default_color_type
		>
	> graph_t;

	typedef boost::graph_traits<graph_t>::vertex_descriptor vertex_t;
//	typedef boost::graph_traits<graph_t>::edge_descriptor   edge_t;

//-------------------------------------------------------------------
int main(int, char*[])
{
	SHOW_INFO;

//	graph_t g( name.size() );
	graph_t g;

	add_edge(2, 1, g);
	add_edge(2, 3, g);
	add_edge(3, 1, g);

	add_edge(4, 5, g);

	add_edge(6, 7, g);
	add_edge(8, 7, g);
	add_edge(8, 6, g);

	add_edge(9, 4, g);

	RenderGraph( g );

	std::vector<std::vector<vertex_t>> loops = udgcd::FindCycles<graph_t,vertex_t>( g );
	udgcd::PrintPaths( std::cout, loops );

	return 0;
}
//-------------------------------------------------------------------
