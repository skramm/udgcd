// Copyright Sebastien Kramm 2016
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file test_1.cpp
\brief Sample code
*/

#include <iostream>
#include "udgcd.hpp"

int prog_id = 10;
#include "common_sample.h"



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
	std::srand(time(0));
	int nb_egdes = 50;
	int max_vertex = 30;

	for( size_t i=0;i<nb_egdes; i++ )
	{
		int v1 = static_cast<int>(std::rand()*1.0/RAND_MAX*max_vertex );
		int v2 = static_cast<int>(std::rand()*1.0/RAND_MAX*max_vertex );
		if( v1 != v2 )
			/// \todo: add checking if the edge is not already there
			boost::add_edge( v1, v2, g );
	}

	PrintGraphInfo( g );
	RenderGraph( g );

	std::vector<std::vector<vertex_t>> cycles = udgcd::FindCycles<graph_t,vertex_t>( g );
	udgcd::PrintPaths( std::cout, cycles, "final-0" );
}

//-------------------------------------------------------------------

