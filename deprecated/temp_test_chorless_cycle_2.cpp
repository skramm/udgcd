// temp_test_chorless_cycle_2.cpp
/**
Attempt using a tree and a DFS
*/

#include <vector>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <set>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/undirected_dfs.hpp>
#include <boost/graph/depth_first_search.hpp>


#include "udgcd.hpp"

/// Type of vertices in the tree. Hold an index on the source vertex in undirected graph
template<typename T>
struct TreeNode
{
	T src_vertex;
};

/// The tree definition


template<typename vertex_t>
using tree_t = boost::adjacency_list<
		boost::vecS,
		boost::vecS,
		boost::undirectedS,
		TreeNode<vertex_t>
	>;

template<typename vertex_t>
using tree_vertex_t = typename boost::graph_traits<tree_t<vertex_t>>::vertex_descriptor;

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
/// Convert undirected graph to tree. Recursive
template<typename vertex_t, typename graph_t>
void
fill_tree(
	tree_t<vertex_t>&             tree,       ///< output tree
	const std::vector<vertex_t>&  cycle,      ///< the cycle we are investigating
	tree_vertex_t<vertex_t>       t_current,  ///< current vertex
	const graph_t&                gr,         ///< input graph (undirected)
	std::vector<vertex_t>         cvec,       ///< current cycle we are exploring
	std::vector<std::vector<vertex_t>>& out   ///< output set of cycles
)
{
	static int depth;

	auto current = tree[t_current].src_vertex;
	COUT << __FUNCTION__ << "(): current=" << current << "\n";

	if( depth == 15 )		std::exit(1);

	cvec.push_back( current );

	for(
		auto p_it = boost::out_edges( current, gr );
		p_it.first != p_it.second;
		p_it.first++
	)
	{
		vertex_t next   = boost::target( *p_it.first, gr );
		COUT << "L1: depth=" << depth << " current=" << current << ": edge " << *p_it.first << " target=" << next << "\n";

		auto it = std::find( cvec.begin(), cvec.end(), next );
		if( it != cvec.end() )     // found in the current path
		{
//			COUT << "next=" << next << " found, *it=" << *it << "\n";
			if( cvec.size() > 1 )
			{
				auto it_previous = std::end(cvec) - 2;
				if( *it != *it_previous )            // and not the previous node
				{
					COUT << "-FOUND in PATH, stop: vec:"; printVector( std::cout, cvec );

					std::vector<vertex_t> newvec( cvec.end()-it );                 // copy from the location of found item
					COUT << "newvec size=" << newvec.size() << "\n";
					std::copy( it, cvec.end(), newvec.begin() );
					udgcd::priv::normalizeCycle( newvec );
					if( std::find( out.begin(), out.end(), newvec ) == out.end() )  // if cycle not already in set,
						if( udgcd::priv::chords::isChordless( newvec, gr ) )                // AND is chordless,
							out.push_back( newvec );                                // then add it to the output set
					return;
				}
			}
		}
		else                       // NOT found in the current path
		{
			if( next == cycle.back() )   // if last element of cycle
			{
				cvec.push_back( next );
				COUT << "-Found last: " << next << ", return, vec: "; printVector( std::cout, cvec );
				if( cvec != cycle && cvec.size()>2 )   // if NOT the same as input cycle,
					out.push_back( cvec );            // we add it to the output set
				return;
			}
			else
			{
				COUT << "-create edge " << current << "-" << next << "\n";
				auto t_next = boost::add_vertex( tree );
				tree[t_next].src_vertex = next;
				boost::add_edge( t_current, t_next, tree );   // add edge in tree
				depth++;
				fill_tree( tree, cycle, t_next, gr, cvec, out );
				depth--;
			}
		}
	}
	COUT << "END\n";
}

//--------------------------------------------------------
/// Remove in the set of vector \c in the longest one
/**
\todo 20200418: This could probably be optimized by swapping the value of the
longest one with the last one, so the erase operation ha a minimal cost.
But is it worth it ?
*/
template<typename T>
void
removeLongest( std::vector<T>& in )
{
	auto it = std::max_element(
		in.begin(),
		in.end(),
		[]                           // lambda
		(const T& a, const T& b)
		{
			return a.size()<b.size();
		}
	);
//	std::cout << "longest is "; printVector( std::cout, *it );
	in.erase( it );
}

//--------------------------------------------------------
template<typename vertex_t, typename graph_t>
std::vector<std::vector<vertex_t>>
extractChordlessCycles( const std::vector<vertex_t>& cycle, const graph_t& gr )
{
	std::vector<std::vector<vertex_t>> out;
	std::cout << __FUNCTION__ << "()\n";

// build tree (call to recursive function)
	tree_t<vertex_t> tree;
	std::vector<vertex_t> empty;
	auto tfirst = boost::add_vertex( tree );
	tree[tfirst].src_vertex = cycle[0];

	fill_tree( tree, cycle, tfirst, gr, empty, out );
	removeLongest( out );

	udgcd::priv::checkCycles( out, gr );

	std::cout << "\nOUTPUT SET:\n";
	for( const auto& cycle: out)
	{
		printVector( std::cout, cycle );
	}
// DFS
/*	myDFSVisitor vis;
	auto indexmap = boost::get( boost::vertex_index, tree );
	auto colormap = boost::make_vector_property_map<boost::default_color_type>(indexmap);

	boost::depth_first_search( gr, vis, colormap, cycle[0] );
*/

	return out;
}

//--------------------------------------------------------
using  graph_t = boost::adjacency_list<
		boost::vecS,
		boost::vecS,
		boost::undirectedS
	>;

//--------------------------------------------------------
int main()
{
	graph_t g;
	boost::add_edge( 0, 1, g );   //           8--9
	boost::add_edge( 1, 2, g );   //           |
	boost::add_edge( 0, 7, g );   //  7--0--1--2---3--4---5---6
	boost::add_edge( 2, 3, g );   //     |  |\    /   |   |
	boost::add_edge( 3, 4, g );   //     |  | \--/    /   |
	boost::add_edge( 4, 5, g );   //     |   \-------/    |
	boost::add_edge( 5, 0, g );   //     \----------------/
	boost::add_edge( 1, 3, g );
	boost::add_edge( 1, 4, g );
	boost::add_edge( 5, 6, g );

	boost::add_edge( 2, 8, g );
	boost::add_edge( 9, 9, g );

	std::vector<size_t> vec{ 0,1,2,3,4,5 };
	auto res = extractChordlessCycles( vec, g );

	boost::add_edge( 3, 5, g );

	auto res2 = extractChordlessCycles( vec, g );

}