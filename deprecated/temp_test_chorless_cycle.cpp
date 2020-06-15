// temp_test_chorless_cycle.cpp

#include <vector>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <set>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/undirected_dfs.hpp>

//--------------------------------------------------------
template<typename T>
void
printVector( std::ostream& f, const std::vector<T>& vec )
{
	f << "* vector #=" << vec.size() << ": { ";
	for( const auto& elem : vec )
		f << elem << "-";
	f << " }\n";
}

#define COUT for(int i=0; i<depth; i++ ) std::cout << "  "; std::cout
//--------------------------------------------------------
template<typename vertex_t, typename graph_t>
void
rec_explore_cycle(
	const std::vector<vertex_t>&        cycle,   ///< the cycle we are investigating
	std::vector<std::vector<vertex_t>>& out,     ///< the set of output chordless cycles
	vertex_t                            current, ///< current vertex
	vertex_t                            previous, ///< current vertex
	const graph_t&                      gr,
	std::vector<vertex_t>&              temp_cycle
)
{
	static int depth;
//	static std::set<vertex_t> vset;
	depth++;
	COUT << __FUNCTION__ << "(): current=" << current << " previous=" << previous << "\n";

	if( depth == 10 )		std::exit(1);



	temp_cycle.push_back( current );
// step 1 : building a list of connected nodes
//	std::vector<vertex_t> candidates;
	for(
		auto p_it = boost::out_edges( current, gr );
		p_it.first != p_it.second;
		p_it.first++
	)
	{
		COUT << "L1: depth=" << depth << " current=" << current << ": edge " << *p_it.first << "\n";
		vertex_t vs = boost::source( *p_it.first, gr );
		vertex_t vt = boost::target( *p_it.first, gr );
		assert( vs == current );

		if( vt != previous )
		{
			// for each node, see if it is in the cycle
			// and if so, add it to the list of candidates to consider
			auto it = std::find( cycle.begin(), cycle.end(), vt );
			if( it != cycle.end() )         // if it is in the list
//			if( vset.find(vt) == vset.end() )  // and NOT already in the set
			{
				if( std::find(temp_cycle.begin(), temp_cycle.end(), vt ) != temp_cycle.end() ) // FOUND!: we have a chordless cycle
				{
					COUT << "Found " << vt << ", adding cycle to output "; printVector( std::cout, temp_cycle );
					out.push_back( temp_cycle );
					temp_cycle.clear();
					return;
				}
//				else
//					temp_cycle.push_back( vt );
			}
			COUT << "temp_cycle: "; printVector( std::cout, temp_cycle );

//		COUT << "candidates: "; printVector( std::cout, candidates );

	// step 2 : iterating on list of connected nodes and exploring them 1 by 1
//		for( const auto& cand: candidates )

//			vset.insert( cand );
			COUT << "L2: cand: " << vt << "\n";
			if( vt == cycle.back() )
			{
				COUT << "Found last!\n";
				depth--;
				return;
			}
			else
				rec_explore_cycle( cycle, out, vt, current, gr, temp_cycle );
		}
	}

//	COUT << "END: candidates: "; printVector( std::cout, candidates );
	COUT << "END\n";
	depth--;
}


template<typename vertex_t, typename graph_t>
std::vector<std::vector<vertex_t>>
extractChordlessCycles( const std::vector<vertex_t>& cycle, const graph_t& gr )
{
	std::vector<std::vector<vertex_t>> out;
	std::vector<vertex_t> empty;
	rec_explore_cycle( cycle, out, cycle[0], cycle[0], gr, empty );
	return out;
}

using  graph_t = boost::adjacency_list<
		boost::vecS,
		boost::vecS,
		boost::undirectedS
	>;


int main()
{
	graph_t g;
	boost::add_edge( 0, 1, g );
	boost::add_edge( 1, 2, g ) ;

	boost::add_edge( 0, 7, g );   //  7--0--1--2---3--4---5---6
	boost::add_edge( 2, 3, g );   //     |  |\    /   |   |
	boost::add_edge( 3, 4, g );   //     |  | \--/    /   |
	boost::add_edge( 4, 5, g );   //     |   \-------/    |
	boost::add_edge( 5, 0, g );   //     \----------------/
	boost::add_edge( 1, 3, g );
	boost::add_edge( 1, 4, g );
	boost::add_edge( 5, 6, g );

	std::vector<size_t> vec{ 0,1,2,3,4,5 };
	auto res = extractChordlessCycles( vec, g );

}