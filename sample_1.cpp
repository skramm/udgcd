/**
\file sample_1.cpp
\brief Finds all the loops inside an undirected graph
*/

#include <iostream>
#include <string>
#include "udgld.hpp"
#include <boost/version.hpp>

#include "boost/graph/graphviz.hpp"

/// memory allocation
UDGLD_INIT;

/*
  Example graph from Tarjei Knapstad.

                   H15
                   |
          H8       C2
            \     /  \
          H9-C0-C1    C3-O7-H14
            /   |     |
          H10   C6    C4
               /  \  /  \
              H11  C5    H13
                   |
                   H12
*/

std::vector<std::string> name = {
	"C0", "C1", "C2", "C3", "C4", "C5", "C6", "O7",
	"H8", "H9", "H10", "H11", "H12", "H13", "H14", "H15"};

//-------------------------------------------------------------------
/// Generates a dot file from grah \c g and calls the renderer to produce a png image of the graph
template<typename Graphtype>
void RenderGraph( const Graphtype& g, int idx )
{
	std::string idx_str( std::to_string(idx) );
	{
		std::ofstream f ( "sample_" + idx_str + ".dot" );
		assert( f.is_open() );
		boost::write_graphviz( f, g );
	}
	std::system( std::string( "dot -Tpng sample_" + idx_str + ".dot > sample_" + idx_str + ".png").c_str() );
}

//-------------------------------------------------------------------
int main(int, char*[])
{
	std::cout << "-built with Boost " << BOOST_LIB_VERSION << '\n';
	udgld::graph_t g( name.size() );

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
	add_edge(4, 13, g);
	add_edge(4, 5, g);
	add_edge(5, 12, g);
	add_edge(5, 6, g);
	add_edge(6, 11, g);
	add_edge(7, 14, g);

	int i=0;
	RenderGraph( g, i++ );

	std::vector<std::vector<udgld::vertex_t>> loops;

	loops = udgld::FindLoops( g );          // no cycles
	udgld::PrintPaths( std::cout, loops );

	add_edge( 1, 6, g );                     // cycle !
	RenderGraph( g, i++ );
	loops = udgld::FindLoops( g );
	udgld::PrintPaths( std::cout, loops );

	add_edge( 13, 14, g );                  // another cycle !
	RenderGraph( g, i++ );
	loops = udgld::FindLoops( g );
	udgld::PrintPaths( std::cout, loops );

	add_edge( 15, 8, g );                   // another cycle !
	RenderGraph( g, i++ );
	loops = udgld::FindLoops( g );
	udgld::PrintPaths( std::cout, loops );

	add_edge( 15, 8, g );                 // add a second arc between same vertices, does not add a path
	RenderGraph( g, i++ );
	loops = udgld::FindLoops( g );
	udgld::PrintPaths( std::cout, loops );

	return 0;
}
//-------------------------------------------------------------------
