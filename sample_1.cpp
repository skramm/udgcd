// Copyright Sebastien Kramm 2015
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file sample_1.cpp
\brief Sample code showing usage of udgld.hpp
*/

#include <iostream>
#include "udgld.hpp"

#include "common_sample.h"


/*
  Modified example graph from Tarjei Knapstad.

                   H15
                   |
          H8       C2
            \     /  \
          H9-C0-C1    C3-O7-H14
            /   |     |
          H10   C6    C4
               /  \  /
              H11  C5---H13
                   |
                   H12
*/

/// the nodes
std::vector<std::string> name = {
	"C0", "C1", "C2", "C3", "C4", "C5", "C6", "O7",
	"H8", "H9", "H10", "H11", "H12", "H13", "H14", "H15"};


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

	graph_t g( name.size() );

	add_edge(0, 1, g);
	add_edge(0, 8, g);
	add_edge(0, 9, g);
	add_edge(0, 10, g);
	add_edge(1, 2, g);
	//  add_edge(1, 6, g); // remove the initial cycle
	add_edge(2, 15, g);
	add_edge(2, 3, g);
	add_edge(3, 7, g);
	add_edge(3, 4, g);
	add_edge(5, 13, g);
	add_edge(4, 5, g);
	add_edge(5, 12, g);
	add_edge(5, 6, g);
	add_edge(6, 11, g);
	add_edge(7, 14, g);

	int i=0;
	RenderGraph( g, 1, i++ );

	std::vector<std::vector<vertex_t>> loops = udgld::FindLoops<graph_t,vertex_t>( g );      // no cycles at first
	udgld::PrintPaths( std::cout, loops, "final-0" );

	add_edge( 1, 6, g );                                 // cycle !
	loops = udgld::FindLoops<graph_t,vertex_t>( g );
	RenderGraph( g, 1, i++ );
	udgld::PrintPaths( std::cout, loops, "final-1" );

	add_edge( 13, 14, g );                              // another cycle !
	RenderGraph( g, 1, i++ );
	loops = udgld::FindLoops<graph_t,vertex_t>( g );
	udgld::PrintPaths( std::cout, loops, "final-2" );
/*
	add_edge( 15, 8, g );                               // another cycle !
	RenderGraph( g, 1, i++ );
	loops = udgld::FindLoops<graph_t,vertex_t>( g );
	udgld::PrintPaths( std::cout, loops, "final-3" );

	add_edge( 15, 8, g );                               // add a second arc between same vertices, does not add a path
	RenderGraph( g, 1, i++ );
	loops = udgld::FindLoops<graph_t,vertex_t>( g );
	udgld::PrintPaths( std::cout, loops, "final-4" );
*/
	return 0;
}

//-------------------------------------------------------------------

