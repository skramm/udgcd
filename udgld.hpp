// Copyright Sebastien Kramm 2015
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file udgld.hpp
\brief UnDirected Graph Loop Detection. Finds all the loops inside an undirected graph.

See file README.md
*/

#ifndef HG_UDGLD_HPP
#define HG_UDGLD_HPP

#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/undirected_dfs.hpp>

/// A macro that you need to put in your code
/**
Needed to allocate memory for a static boolean
*/
#define UDGLD_INIT bool udgld::LoopDetector::cycleDetected = false;

/// all the codes is in this namespace
namespace udgld {

//-------------------------------------------------------------------------------------------
/// Private, don't use.
/**
Recursive function, explores edges connected to \c v1 until we find a loop

\warning Have to be sure there \b is a loop, else infinite recursion !
*/
template <class Vertex, class Graph>
bool explore(
	const Vertex& v1,                            ///< the starting vertex we want to explore
	const Graph&  g,
	std::vector<std::vector<Vertex>>& vv_paths,
	std::vector<std::vector<Vertex>>& vv_loops, ///< this is where we store the paths that have loops
	int depth = 0
) {
	++depth;
	static int max_depth = std::max( depth, max_depth );
	assert( vv_paths.size()>0 );

	typename boost::graph_traits<Graph>::out_edge_iterator ei, ei_end;
	boost::tie(ei, ei_end) = out_edges( v1, g );
//	COUTP << "nb of edges = " << ei_end - ei << "\n";
	size_t edge_idx = 0;

	std::vector<Vertex> src_path = vv_paths[vv_paths.size()-1];
//	COUTP << "src_path :"; for( const auto& vv:src_path )	cout << vv << "-"; cout << "\n";

	bool found = false;
//	int iter=0;
//	size_t n = ei_end -ei;
	for( ; ei != ei_end; ++ei, ++edge_idx )
	{
		bool b = false;
		Vertex v2a = source(*ei, g);
		Vertex v2b = target(*ei, g);
//		COUTP << ++iter << '/' <<  n << ": connected edges v2: v2a=" << v2a << " v2b=" << v2b << "\n";

		if( v2b == v1 && v2a == src_path[0] ) // we just found the edge that we started on, so no need to finish the current iteration, just move on.
			continue;

		std::vector<Vertex> newv(src_path);
		bool AddNode = true;
		if( newv.size() > 1 )
			if( newv[ newv.size()-2 ] == v2b )
				AddNode = false;

		if( AddNode )
		{
			if( std::find( newv.cbegin(), newv.cend(), v2b ) != newv.cend() )
			{
				newv.push_back( v2b );
//				COUTP << "*** FOUND LOOP!\n";
				vv_loops.push_back( newv );
				return true;
			}
			else
			{
				newv.push_back( v2b );         // else add'em and continue
//				COUTP << "  -adding vector ";  for( const auto& vv:newv )	cout << vv << "-"; cout << "\n";
				vv_paths.push_back( newv );
				b = explore( v2b, g, vv_paths, vv_loops, depth );
			}
		}
		if( b )
			found = true;
	}
	return found;
}

//-------------------------------------------------------------------------------------------
/// Additional helper function, can be used to print the loops found
template<typename T>
void PrintPaths( std::ostream& f, const std::vector<std::vector<T>>& v_paths, const char* msg=0 )
{
	if( msg )
		f << msg << ": ";
	f <<  "-Found " << v_paths.size() << " loops:\n";
	for( const auto& path: v_paths )
	{
		f << " - ";
		for( const auto& node : path )
			f << node << "-";
		f << "\n";
	}
}

//-------------------------------------------------------------------------------------------
/// Private, don't use.
/**
Extract loop segment from whole path.

If an input string is "ABCDEFCXY" then the output string will be "CDEFC"
*/
template<typename T>
std::vector<std::vector<T>> ExtractRelevantPart( const std::vector<std::vector<T>>& loops )
{
	std::vector<std::vector<T>> out;
	for( const auto& path: loops )
	{
		for( auto it = path.cbegin(); it != path.cend()-1; ++it )
		{
			auto it_second = std::find( it+1, path.cend(), *it );
			if( it_second != path.cend() )
			{
//				cout << "Found dupe: " << *it_second << "\nNew=";
				std::vector<T> newv( it_second - it + 1 );         // output vector will have the common element at both sides
				std::copy( it, it_second+1, newv.begin() );
				out.push_back( newv );
				break;
			}
		}
	}
	return out;
}

//-------------------------------------------------------------------------------------------
/// Private, don't use.
/**
 Remove twins : vector that are the same, but in reverse order
*/
template<typename T>
std::vector<std::vector<T>> RemoveOppositePairs( const std::vector<std::vector<T>>& loops )
{
	assert( loops.size() );
	std::vector<std::vector<T>> out;      // output vector
	std::vector<bool> flags( loops.size(), true ); // some flags to keep track of which elements are reversed

	for( size_t i=0; i<loops.size()-1; ++i )
		if( flags[i] )
		{
			std::vector<T> rev = loops[i];                       // step 1: build a reversed copy of the current vector
			std::reverse( rev.begin(), rev.end() );
			for( size_t j=i+1; j<loops.size(); ++j )                  // step 2: parse the rest of the list, and check
				if( flags[j] && rev == loops[j] )                     // if similar, then
				{
					out.push_back( loops[i] );                        //  1 - add current vector into output
					flags[j] = false;                                 //  2 -  invalidate the reversed one
				}
		}
	return out;
}

//-------------------------------------------------------------------------------------------
/// Private, don't use.
/**
Helper function for RemoveIdentical()

Given an input vector "DABCD", it will return "ABCD" (removal of duplicate element, and first element is the smallest)
*/
template<typename T>
std::vector<T> GetSortedTrimmed( const std::vector<T>& v_in )
{
	assert( v_in.front() == v_in.back() );
	assert( v_in.size() > 1 );

	std::vector<T> v_out( v_in.size() - 1 );
	std::copy( v_in.cbegin(), v_in.cend()-1, v_out.begin() );

	auto it = std::min_element( v_out.begin(), v_out.end() );
	std::rotate( v_out.begin(), it, v_out.end() );

	return v_out;
}

//-------------------------------------------------------------------------------------------
/// Private, don't use.
/**
Remove identical strings that are the same up to the starting point
It also sorts the paths by rotating them so that the node of smallest index is first
*/
template<typename T>
std::vector<std::vector<T>> RemoveIdentical( const std::vector<std::vector<T>>& loops )
{
	assert( loops.size() );
	std::vector<std::vector<T>> out;      // output vector

	if( loops.size() == 1 )                                  // if single, then we justs add it, after trimming/sorting
		out.push_back( GetSortedTrimmed( loops[0] ) );

	std::vector<bool> flags( loops.size(), true );     // flags to keep track of elements to consider

	for( size_t i=0; i<loops.size()-1; ++i )
		if( flags[i] )
		{
			std::vector<T> rot_i = GetSortedTrimmed( loops[i] ) ;               // step 1: build a rotated copy of the current vector, minus the last element (identical to the first)
			out.push_back( rot_i );

			for( size_t j=i+1; j<loops.size(); ++j )                                // step 2: parse the rest of the list, and check
				if( flags[j] )                                                      //  if not flagged, then
				{
					const std::vector<T>& rot_j = GetSortedTrimmed( loops[j] );     // 2.1 - build a rotated copy of second vector
					if( rot_i == rot_j )                                            // 2.2 - if rotated vectors are the same, then
						flags[j] = false;                                           //        invalidate the second one
				}
		}
	return out;
}

//-------------------------------------------------------------------------------------------
/// Loop detector for an undirected graph
/**
Passed by value as visitor to \c boost::undirected_dfs()

See http://www.boost.org/doc/libs/1_58_0/libs/graph/doc/undirected_dfs.html
*/
struct LoopDetector : public boost::dfs_visitor<>
{
	public:
		LoopDetector()
		{
			LoopDetector::cycleDetected = false;
		}

		template <class Edge, class Graph>
		void back_edge( Edge e, const Graph& g )     // is invoked on the back edges in the graph.
		{
	#if 0
			typedef typename boost::graph_traits < Graph >::vertex_descriptor Vertex;
			Vertex v1 = source(e, g);
			Vertex v2 = target(e, g);
			std::cout << " => CYCLE DETECTED! v1=" << v1 << " v2=" << v2 << "\n";
	#endif
			LoopDetector::cycleDetected = true;
		}
//	private:
		static bool cycleDetected;
};

//-------------------------------------------------------------------------------------------
/// Main user interface: just call this function to get the loops inside your graph
/**
Returns a vector of loops that have been found in the graph
*/
template<typename Graph, typename vertex_t>
std::vector<std::vector<vertex_t>>
FindLoops( Graph& g )
{
	LoopDetector ld;
	boost::undirected_dfs( g, boost::root_vertex( vertex_t(0) ).visitor(ld).edge_color_map( get(boost::edge_color, g) ) );

	if( !ld.cycleDetected )
		return std::vector<std::vector<vertex_t>>(); // return empty vector, no loops found

// else, get the loops.
	{
		std::vector<std::vector<vertex_t>> v_loops;
		std::vector<std::vector<vertex_t>> v_paths;
		std::vector<vertex_t> newv(1, vertex_t(0) ); // start by first vertex (index 0)
		v_paths.push_back( newv );
		explore( vertex_t(0), g, v_paths, v_loops );    // call of recursive function

	#ifdef UDGLD_PRINT_STEPS
		PrintPaths( std::cout, v_loops, "Raw loops" );
	#endif

	// post process 1: extract loop segments from whole path
		std::vector<std::vector<vertex_t>> loops2 = ExtractRelevantPart( v_loops );
	#ifdef UDGLD_PRINT_STEPS
		PrintPaths( std::cout, loops2, "loops2" );
	#endif

	// post process 2: remove the paths that are identical but reversed
		std::vector<std::vector<vertex_t>> loops3 = RemoveOppositePairs( loops2 );
	#ifdef UDGLD_PRINT_STEPS
		PrintPaths( std::cout, loops3, "loops3" );
	#endif

	// post process 3: remove twin pathes
		std::vector<std::vector<vertex_t>> loops4 = RemoveIdentical( loops3 );
	#ifdef UDGLD_PRINT_STEPS
		PrintPaths( std::cout, loops4, "loops4" );
	#endif
		return loops4;
	}
}

//-------------------------------------------------------------------------------------------

} // namespace end

#endif // HG_UDGLD_HPP
