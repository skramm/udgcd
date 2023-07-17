// Copyright Sebastien Kramm 2016-2020
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file read_graph.cpp
\brief Read a graph in a file, searches for cycles, and make sure these are correct.

Also generates a .dot file that can be graphically rendered with Graphviz
*/

#include <iostream>

//#define UDGCD_DEV_MODE
//#define UDGCD_PRINT_STEPS

std::string prog_id = "read_graph";
#include "common_sample.h"


//-------------------------------------------------------------------
/// see read_graph.cpp
int main( int argc, const char** argv )
{
	SHOW_INFO;

	using graph_t = boost::adjacency_list<
	boost::vecS,
	boost::vecS,
	boost::undirectedS,
	sample::NodeData
	>;

	using vertex_t = boost::graph_traits<graph_t>::vertex_descriptor;

	if( argc < 2 )
	{
		std::cout << "missing input filename, exit.\n";
		return 1;
	}
	auto vs = sample::splitString( argv[argc-1], '.' );
	if( vs.size() < 2 )
	{
		std::cerr << "Error, input file '" <<  argv[1] << "' has no extension\n";
		return 1;
	}

	graph_t gr;
	if( vs.back() == "dot" )
	{
		gr = sample::loadGraph_dot<graph_t>( argv[1] );
	}
	else
	{
		if( vs.back() == "txt" )
		{
			gr = sample::loadGraph_txt<graph_t>( argv[1] );
		}
		else
		{
			std::cerr << "Error, input file '" <<  argv[1] << "' extension invalid\n";
			return 1;
		}
	}


	auto vs1 = sample::splitString( argv[1], '/' );      // splits by '/', and keep the last one (filename)
	auto vs2 = sample::splitString( vs1.back(), '.' );     // splits by the point (if any)
	sample::renderGraph( gr, vs2[0] );

	bool noProcess(false);
	bool verbose(false);
	if( argc > 2 )
	{
		for( int i=1;i<argc-1;i++ )
		{
			std::string a( argv[i] );
			if( a == "-n" )
				noProcess = true;
			if( a == "-v" )
				verbose = true;
		}
	}
	if( noProcess )
		return 0;

	udgcd::RunTimeOptions rtOptions;
	auto result = sample::processGraph<graph_t,vertex_t>( gr, rtOptions );
	sample::renderGraph2( gr, result.second, vs2[0]+"_color" );
	return result.first;
}

//-------------------------------------------------------------------

