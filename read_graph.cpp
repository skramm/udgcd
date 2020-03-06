// Copyright Sebastien Kramm 2016-2020
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file read_graph.cpp
\brief read a graph in simple text format, searches for cycles, and make sure these are correct

Also generates a .dot file that can be graphically rendered with Graphviz
*/

#include <iostream>

#define UDGCD_DEV_MODE
#define UDGCD_PRINT_STEPS
#include "udgcd.hpp"

std::string prog_id = "read_graph";
#include "common_sample.h"

//-------------------------------------------------------------------------------------------
/// Some typedefs for readability
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

	auto vs1 = split_string( argv[1], '/' );      // splits by '/', and keep the last one (filename)
	auto vs2 = split_string( vs1.back(), '.' );     // splits by the point (if any)
	RenderGraph( g, vs2[0] );

	bool noProcess(false);
	if( argc > 2 )
	{
		std::string a( argv[2] );
		if( a == "-n" )
			noProcess = true;
	}
	if( noProcess )
		return 0;
	return Process( g );
}

//-------------------------------------------------------------------

