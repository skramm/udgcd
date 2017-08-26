// Copyright Sebastien Kramm 2016-2017
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file read_graph.cpp
\brief read a graph in simple text format
*/

#include <iostream>
#include "udgcd.hpp"

std::string prog_id = "read_graph";
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
/// see read_graph.cpp
int main( int argc, const char** argv )
{
	SHOW_INFO;
	if( argc < 2 )
	{
		std::cout << "missing input filename, exit.\n";
		return 1;
	}
	graph_t g = LoadGraph<graph_t>( argv[1] );
	PrintGraphInfo( g );
	RenderGraph( g );

	std::vector<std::vector<vertex_t>> cycles = udgcd::FindCycles<graph_t,vertex_t>( g );
	udgcd::PrintPaths( std::cout, cycles, "final" );
}

//-------------------------------------------------------------------

