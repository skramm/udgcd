// Copyright Sebastien Kramm 2015
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file sample_1.cpp
\brief Sample code
*/

#include <iostream>
#include "udgcd.hpp"

int prog_id = 4;
#include "common_sample.h"


/// the nodes


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
	typedef boost::graph_traits<graph_t>::edge_descriptor   edge_t;

//-------------------------------------------------------------------
int main(int, char*[])
{
	SHOW_INFO;

	graph_t g; //( name.size() );

	add_edge(0, 1, g);
	add_edge(0, 8, g);

	add_edge(1, 2, g);
	//  add_edge(1, 6, g); // remove the initial cycle

//	add_edge(2, 15, g);
	add_edge(2, 8, g);

	add_edge(2, 3, g);
	add_edge(3, 7, g);
	add_edge(3, 4, g);
	add_edge(5, 9, g);
	add_edge(4, 5, g);
//	add_edge(5, 12, g);
	add_edge(5, 6, g);
//	add_edge(6, 11, g);
	add_edge(7, 9, g);

	RenderGraph( g );

	std::vector<std::vector<vertex_t>> loops = udgcd::FindCycles<graph_t,vertex_t>( g );      // no cycles at first
	udgcd::PrintPaths( std::cout, loops, "final-0" );

	add_edge( 1, 6, g );                                 // cycle !
	loops = udgcd::FindCycles<graph_t,vertex_t>( g );
	RenderGraph( g );
	udgcd::PrintPaths( std::cout, loops, "final-1" );

/*	add_edge( 13, 14, g );                              // another cycle !
	RenderGraph( g );
	loops = udgcd::FindCycles<graph_t,vertex_t>( g );
	udgcd::PrintPaths( std::cout, loops, "final-2" );
*/
/*	add_edge( 15, 8, g );                               // another cycle !
	RenderGraph( g );
	loops = udgcd::FindCycles<graph_t,vertex_t>( g );
	udgcd::PrintPaths( std::cout, loops, "final-3" );
*/

	return 0;
}

//-------------------------------------------------------------------

