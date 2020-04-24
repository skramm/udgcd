// Copyright Sebastien Kramm 2016-2020
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file read_graph.cpp
\brief Read a graph in a file, searches for cycles, and make sure these are correct.

Also generates a .dot file that can be graphically rendered with Graphviz

\todo 20200422: Add DOT as input file format !
*/

#include <iostream>

//#define UDGCD_DEV_MODE
//#define UDGCD_PRINT_STEPS

std::string prog_id = "read_graph";
#include "common_sample.h"


//-------------------------------------------------------------------------------------------
/// Some typedefs for readability
typedef boost::adjacency_list<
	boost::vecS,
	boost::vecS,
	boost::undirectedS,
	NodePos
	> graph_t;

typedef boost::graph_traits<graph_t>::vertex_descriptor vertex_t;
typedef boost::graph_traits<graph_t>::edge_descriptor   edge_t;

//-------------------------------------------------------------------
/// see read_graph.cpp
int main( int argc, const char** argv )
{
	SHOW_INFO;

#ifdef UDGCD_USE_M4RI
	std::cout << "Build using libM4ri\n";
#endif

	if( argc < 2 )
	{
		std::cout << "missing input filename, exit.\n";
		return 1;
	}
	auto vs = splitString( argv[1], '.' );
	if( vs.size() < 2 )
	{
		std::cerr << "Error, input file '" <<  argv[1] << "' has no extension\n";
		return 1;
	}

	graph_t g;
	if( vs.back() == "dot" )
	{
		g = loadGraph_dot<graph_t>( argv[1] );
	}
	else
	{
		if( vs.back() == "txt" )
		{
			g = loadGraph_txt<graph_t>( argv[1] );
		}
		else
		{
			std::cerr << "Error, input file '" <<  argv[1] << "' extension invalid\n";
			return 1;
		}
	}


	auto vs1 = splitString( argv[1], '/' );      // splits by '/', and keep the last one (filename)
	auto vs2 = splitString( vs1.back(), '.' );     // splits by the point (if any)
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
	auto result = processGraph<graph_t,vertex_t>( g );
	RenderGraph2( g, result.second, vs2[0]+"_color" );

	return result.first;
}

//-------------------------------------------------------------------

