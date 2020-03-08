// Copyright Sebastien Kramm, 2016-2017
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file sample_3.cpp
\brief Sample code demonstrating when some nodes are NOT connected, and with custom properties in graph

*/

#include <iostream>
#include <algorithm>
#include "udgcd.hpp"

std::string prog_id = "s3";
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

struct vertex_data {
    std::string whatever;
    int othervalue;
};

typedef boost::adjacency_list<
	boost::vecS,
	boost::vecS,
	boost::undirectedS,
	vertex_data
	> graph_t;

	typedef boost::graph_traits<graph_t>::vertex_descriptor vertex_t;


//-------------------------------------------------------------------
void g1()
{
	graph_t g;

	add_edge(0, 1, g);
	add_edge(0, 2, g);
	add_edge(2, 1, g);

	add_edge(3, 1, g);
	add_edge(2, 3, g);

	RenderGraph( g, prog_id );
	{
		std::vector<std::vector<vertex_t>> cycles = udgcd::FindCycles<graph_t,vertex_t>( g );
		udgcd::PrintPaths( std::cout, cycles );
	}
	add_edge( 2, 4, g);
	add_edge( 3, 4, g);

	RenderGraph( g, prog_id );
	{
		std::vector<std::vector<vertex_t>> cycles = udgcd::FindCycles<graph_t,vertex_t>( g );
		udgcd::PrintPaths( std::cout, cycles );
	}
}

//-------------------------------------------------------------------
void g2()
{
	graph_t g;
//    std::vector<std::string> v_nodeName;

	add_edge( 0, 1, g );
	add_edge( 1, 2, g );
	add_edge( 2, 3, g );
	add_edge( 3, 4, g );
	add_edge( 4, 0, g );

	add_edge( 5, 1, g);
	add_edge( 5, 4, g);

/*    v_nodeName.resize( boost::num_vertices(g) );

	for( auto& s : v_nodeName )
	{
		static int i=0;
		s="N" + std::to_string(i++);
	};


	for( auto s: v_nodeName )
			std::cout << s << "-";
	std::cout << "\n";
*/

	{
        RenderGraph( g, "s3g2" );
//        RenderGraph2( g, v_nodeName );
        std::vector<std::vector<vertex_t>> cycles = udgcd::FindCycles<graph_t,vertex_t>( g );
        udgcd::PrintPaths( std::cout, cycles );
	}

/*	clear_vertex( 3, g );
	add_edge( 2, 4, g );
	{
    	RenderGraph( g );
        std::vector<std::vector<vertex_t>> cycles = udgcd::FindCycles<graph_t,vertex_t>( g );
        udgcd::PrintPaths( std::cout, cycles );
	}

	clear_vertex( 2, g );
	add_edge( 1, 4, g );
	{
    	RenderGraph( g );
        std::vector<std::vector<vertex_t>> cycles = udgcd::FindCycles<graph_t,vertex_t>( g );
        udgcd::PrintPaths( std::cout, cycles );
	}
*/

}

//-------------------------------------------------------------------
int main(int, char*[])
{
	SHOW_INFO;

    g1();
    g2();
	return 0;
}
//-------------------------------------------------------------------
