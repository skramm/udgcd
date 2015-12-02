// Copyright Sebastien Kramm 2015
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file sample_3.cpp
\brief Sample code demonstrating when some nodes are NOT connected

In some situations, we can have a loop but explore can fail if the starting node DOES NOT lead to that loop
*/

#include <iostream>
#include "udgld.hpp"

int prog_id = 3;
#include "common_sample.h"

/*
  test graph
        N3
       / \
      N1-N2
       \ /
        N0
*/


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

	int i=0;

	graph_t g;

	add_edge(0, 1, g);
	add_edge(0, 2, g);
	add_edge(2, 1, g);

	add_edge(3, 1, g);
	add_edge(2, 3, g);

	RenderGraph( g );
	{
		std::vector<std::vector<vertex_t>> loops = udgld::FindLoops<graph_t,vertex_t>( g );
		udgld::PrintPaths( std::cout, loops );
	}
	add_edge( 2, 4, g);
	add_edge( 3, 4, g);

	RenderGraph( g );
	{
		std::vector<std::vector<vertex_t>> loops = udgld::FindLoops<graph_t,vertex_t>( g );
		udgld::PrintPaths( std::cout, loops );
	}

	return 0;
}
//-------------------------------------------------------------------
