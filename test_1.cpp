// Copyright Sebastien Kramm, 2016-2017
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file test_1.cpp
\brief this app will generate a random graph and count its cycles.
It also saves the generated graph in a file so it can be read afterwards with read_graph.cpp

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
/// see test_1.cpp
int main(int, char*[])
{
	SHOW_INFO;

	graph_t g;
	auto t = time(0);
	std::srand( t );
	int nb_egdes = 15;
	int nb_vertices = 8;

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

	PrintGraphInfo( g );
	RenderGraph( g );

	{
		std::string fname = "gen_graph_" + std::to_string(t) + ".txt";
		std::cout << "Graph is saved in file " << fname << '\n';
		std::ofstream f( fname );
		assert( f.is_open() );
		f << "v:" << boost::num_vertices( g ) << '\n';

		auto pei = boost::edges( g );
		for(
			auto pei = boost::edges( g );
			pei.first != pei.second;
			++pei.first
		)
			f << boost::source( *pei.first, g ) << "-" << boost::target( *pei.first, g ) << '\n';
	}

	std::vector<std::vector<vertex_t>> cycles = udgcd::FindCycles<graph_t,vertex_t>( g );
	udgcd::PrintPaths( std::cout, cycles, "final-0" );
}

//-------------------------------------------------------------------

