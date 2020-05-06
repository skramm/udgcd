// Copyright Sebastien Kramm 2016-2020
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
/// Saves graph \c g in a text file, in folder \c out
/**
- fails is folder does not exist
- data is saved as ASCII, with the following format
 - first line: number of vertices
 - one line per edge
*/
void
SaveGraph( graph_t g, std::string id )
{
	std::string fname = "out/gen_graph_" + id + ".txt";
	std::cout << "udgcd: graph is saved in file " << fname << '\n';
	std::ofstream f( fname );
	if( !f.is_open() )
	THROW_ERROR( "unable to open file " + fname  );
/*	{
		std::cerr << "udgcd: " << __FILE << '-' << __FUNCTION__ << ": unable to open file " << fname << std::endl;
		throw;
	}*/
	f << "v:" << boost::num_vertices( g ) << "\n\n";

	for(                                 // enumerate all the edges
		auto pei = boost::edges( g );
		pei.first != pei.second;
		++pei.first
	)
		f << boost::source( *pei.first, g ) << "-" << boost::target( *pei.first, g ) << '\n';
}
//-------------------------------------------------------------------
graph_t
GenerateRandomGraph( size_t nb_vertices, size_t nb_egdes )
{
	graph_t g;

	for( size_t i=0;i<nb_vertices; i++ )
		boost::add_vertex(g);

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
	return g;
}
//-------------------------------------------------------------------
/// See random_test.cpp
int main( int argc, const char** argv )
{
	SHOW_INFO;

	size_t nb_egdes = 15;
	size_t nb_vertices = 8;
	if( argc > 1 )
		nb_vertices = std::atoi( argv[1] );
	if( argc > 2 )
		nb_egdes = std::atoi( argv[2] );

	auto current_time = time(0);
	std::srand( current_time );

	graph_t g = GenerateRandomGraph( nb_vertices, nb_egdes );

	SaveGraph( g, std::to_string(current_time) );
	sample::renderGraph( g, "gen_" + std::to_string(current_time) );

	auto result = sample::processGraph<graph_t,vertex_t>( g );
	return result.first;
}
//-------------------------------------------------------------------

