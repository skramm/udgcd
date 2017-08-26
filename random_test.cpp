// Copyright Sebastien Kramm, 2016-2017
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file random_test.cpp
\brief this app will generate a random graph and count its cycles.
It also saves the generated graph in a file so it can be read afterwards with read_graph.cpp

Arguments can be given (default values provided):
\code
 random_test nb_vertices nb_edges
\endcode
*/

#include <iostream>
#include "udgcd.hpp"

std::string prog_id = "random";

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
/// saves grah \c g in a text file, in folder \c out
void
SaveGraph( graph_t g, std::string id )
{
	std::string fname = "out/gen_graph_" + id + ".txt";
	std::cout << "Graph is saved in file " << fname << '\n';
	std::ofstream f( fname );
	assert( f.is_open() );
	f << "v:" << boost::num_vertices( g ) << "\n\n";

	for(                                 // enumerate all the edges
		auto pei = boost::edges( g );
		pei.first != pei.second;
		++pei.first
	)
		f << boost::source( *pei.first, g ) << "-" << boost::target( *pei.first, g ) << '\n';
}
//-------------------------------------------------------------------
/// see random_test.cpp
int main( int argc, const char** argv )
{
	SHOW_INFO;

	graph_t g;
	auto t = time(0);
	std::srand( t );
	size_t nb_egdes = 15;
	size_t nb_vertices = 8;
	if( argc > 1 )
		nb_vertices = std::atoi( argv[1] );
	if( argc > 2 )
		nb_egdes = std::atoi( argv[2] );

	for( size_t i=0;i<nb_vertices; i++ )
		add_vertex(g);

	for( size_t i=0;i<nb_egdes; i++ )
	{
		int v1 = static_cast<int>(std::rand()*1.0/RAND_MAX*nb_vertices );
		int v2 = static_cast<int>(std::rand()*1.0/RAND_MAX*nb_vertices );
		if( v1 != v2 )
		{
			auto p = boost::edge( v1, v2, g );  // returns a pair<edge_descriptor, bool>
			if( !p.second )                     // add edge only if not already present
				boost::add_edge( v1, v2, g );
		}
	}

	auto expected = PrintGraphInfo( g );
	RenderGraph( g );
	SaveGraph( g, std::to_string(t) );

	std::vector<std::vector<vertex_t>> cycles = udgcd::FindCycles<graph_t,vertex_t>( g );
	udgcd::PrintPaths( std::cout, cycles, "final-0" );
	if( expected != cycles.size() )
	{
		std::cout << "ERROR: computed nb of cycles is not what expected...\n";
		return 1;
	}
}
//-------------------------------------------------------------------

