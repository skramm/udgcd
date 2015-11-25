
/**
sample_1.cpp

Finds all the loops inside an undirected graph
*/

#include <iostream>
#include "undir_graph.hpp"
#include <boost/version.hpp>

// memory allocation
UDGLD_INIT;

//bool LoopDetector::cycleDetected;

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


//-------------------------------------------------------------------------------------------
int main(int, char*[])
{
	std::cout << "-built with Boost " << BOOST_LIB_VERSION << '\n';
	udgld::my_graph g( name.size() );

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

	std::vector<std::vector<udgld::vertex_t>> loops;

	loops = udgld::Process( g ); // no cycles
	udgld::PrintPaths( std::cout, loops );

	add_edge(1, 6, g);            // cycle !
	loops = udgld::Process( g );
	udgld::PrintPaths( std::cout, loops );

	add_edge(13, 14, g);          // another cycle !
	loops = udgld::Process( g );
	udgld::PrintPaths( std::cout, loops );

	add_edge(15, 8, g);           // another cycle !
	loops = udgld::Process( g );
	udgld::PrintPaths( std::cout, loops );

	add_edge(15, 8, g);           // add a second arc between same vertices
	loops = udgld::Process( g );
	udgld::PrintPaths( std::cout, loops );

	return 0;
}

