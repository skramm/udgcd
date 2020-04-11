// Copyright Sebastien Kramm 2016-2020
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file udgcd.hpp
\brief UnDirected Graph Cycle Detection. Finds all the cycles inside an undirected graph.

Home page: https://github.com/skramm/udgcd

Inspired from http://www.boost.org/doc/libs/1_58_0/libs/graph/example/undirected_dfs.cpp

\todo 2020-03-09: add a data structure that can measure the run-time depth of the recursive functions

See file README.md
*/

#ifndef HG_UDGCD_HPP
#define HG_UDGCD_HPP

#include <vector>
#include <chrono>
#include <iomanip>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/undirected_dfs.hpp>
#include <boost/dynamic_bitset.hpp>       // needed ! Allows bitwise operations on dynamic size boolean vectors

// TEMP
#include "wrapper_m4ri.hpp"
//#include "wrapper_m4ri_convert.hpp"

#ifdef UDGCD_DEV_MODE
	#include <iostream>
	#define COUT if(1) std::cout << std::setw(4) << __LINE__ << ": "
	#define PRINT_FUNCTION std::cout << "*** start function " <<  __FUNCTION__ << "()\n"
	#define PRINT_FUNCTION_2 if(1) std::cout << "*** start function " <<  __FUNCTION__ << "()"
	#define UDGCD_PRINT_STEPS
	#define PRINT_DIFF( step, v_after, v_before ) std::cout << step << ": REMOVAL OF " << v_before.size() - v_after.size() << " cycles\n"
#else
	#define PRINT_FUNCTION
	#define PRINT_FUNCTION_2 if(0) std::cout
	#define COUT if(0) std::cout
	#define PRINT_DIFF(a,b,c) ;
#endif

/// All the provided code is in this namespace
namespace udgcd {

//-------------------------------------------------------------------------------------------
template<typename T>
void
PrintVector( std::ostream& f, const std::vector<T>& vec )
{
	for( const auto& elem : vec )
		f << elem << "-";
	f << "\n";
}

/// Additional helper function, can be used to print the cycles found
template<typename T>
void
PrintPaths( std::ostream& f, const std::vector<std::vector<T>>& v_paths, const char* msg=0 )
{
	static int iter=0;
	f << "Paths (" << iter++ << "): nb=" << v_paths.size();
	if( msg )
		f << ": " << msg;
	f << "\n";

	for( size_t i=0; i<v_paths.size(); i++ )
	{
		f << " - " << i << ": ";
		PrintVector( f, v_paths[i] );
	}
}


//-------------------------------------------------------------------------------------------
/// holds private types and functions, unneeded to use this library
namespace priv {


//-------------------------------------------------------------------------------------------
/// Print vector of bits
template<typename T>
void
printBitVector( std::ostream& f, const T& vec )
{
	for( size_t i=0; i<vec.size(); i++ )
	{
		f << vec[i];
		if( !((i+1)%4) && i != vec.size()-1 )
			f << '.';
	}
	f << ": #=" << vec.count() << "\n";   // works with boost::dynamic_bitset << "\n";
}


template<typename T>
void
printBitMatrix( std::ostream& f, const T& mat, std::string msg )
{
	f << "Matrix " << msg << ", nbLines=" << mat.size() << " nbCols=" << mat[0].size() << "\n";
    for( auto line: mat )
    {
		f << " | ";
		for( size_t i=0; i<line.size(); i++ )
		{
			f << line[i];
			if( !((i+1)%4) && i != line.size()-1 )
				f << '.';
		}
		f << " |\n";
	}
}


/// Holds a path as a binary vector
/**
Based on https://www.boost.org/doc/libs/release/libs/dynamic_bitset/dynamic_bitset.html

For a graph of \f$n\f$ vertices, its size needs to be \f$ n.(n-1)/2 \f$

Example: for the path 1-3-4 on a graph of 5 vertices (0 - 4), the vector will have a size of 10 elements:

\verbatim
edge:    0  0  0  0  1  1  1  2  2  3
         1  2  3  4  2  3  4  3  4  4
--------------------------------------
vector:  0  0  0  0  0  1  1  0  0  1
\endverbatim
*/
typedef boost::dynamic_bitset<> BinaryVec;


//-------------------------------------------------------------------------------------------
struct BinaryMatInfo
{
    size_t nbLines  = 0;
    size_t nbCols   = 0;
    size_t nbOnes   = 0;
    size_t nb0Cols  = 0;   ///< nb of columns with only 0 values
    size_t nb0Lines = 0;  ///< nb of lines with only 0 values

    void print( std::ostream& f ) const
    {
		f << "BinaryMatInfo:"
			<< "\n-nbLines ="  << nbLines
			<< "\n-nbCols ="   << nbCols
			<< "\n-nbOnes ="   << nbOnes
			<< "\n-nb0Lines =" << nb0Lines
			<< "\n-nb0Cols ="  << nb0Cols
			<< '\n';
    }
};

//-------------------------------------------------------------------------------------------
/// A binary matrix, implemented as a vector of BinaryVec
/**
This type will allow to fetch some relevant information on what the matrix holds
*/
struct BinaryMatrix
{
	std::vector<BinaryVec> _data;

	BinaryMatrix( size_t nbLines, size_t nbCols )
	{
		assert( nbLines>0 );
		assert( nbCols>0 );
		_data.resize( nbLines );
		std::for_each( _data.begin(), _data.end(), [nbCols](BinaryVec& bv){ bv.resize(nbCols); } ); // lambda
//		std::cout << __FUNCTION__ << "(): nb cols=" << _data[0].size() << '\n';
	}
	BinaryMatrix( size_t nbLines )
	{
		assert( nbLines>0 );
		_data.resize( nbLines );
	}
	BinaryMatrix()
	{}

	size_t nbLines() const { return _data.size(); }
	size_t nbCols()  const
	{
		if( 0==nbLines() )
			return 0;
		return _data.at(0).size();
	}

	auto begin() -> decltype(_data.begin()) { return _data.begin(); }
	auto end()   -> decltype(_data.end())   { return _data.end();   }
	const auto begin() const -> decltype(_data.begin()) { return _data.begin(); }
	const auto end()   const -> decltype(_data.end())   { return _data.end();   }

	void addLine( const BinaryVec& bvec )
	{
		_data.push_back(bvec);
	}

	void addCol( const BinaryVec& vin )
	{
        assert( vin.size() == nbLines() );
        for( size_t i=0; i<vin.size(); i++ )
			_data[i].push_back( vin[i] );
	}

	BinaryVec getCol( size_t col ) const
	{
		assert( col<nbCols() );
		BinaryVec out( nbLines() );
		for( size_t i=0; i<nbLines(); i++ )
			out[i] = line(i)[col];
		return out;
	}

	const BinaryVec& line( size_t idx ) const
	{
		assert( idx<nbLines() );
		return _data[idx];
	}
	BinaryVec& line( size_t idx )
	{
		assert( idx<nbLines() );
		return _data[idx];
	}
    BinaryMatInfo getInfo() const
    {
		BinaryMatInfo info;
		info.nbLines = nbLines();
		assert( _data.size() );
		info.nbCols = nbCols();
		std::for_each( _data.begin(), _data.end(), [&info](const BinaryVec& v){ info.nbOnes+= v.count();} ); // count 1

		for( size_t i=0; i<nbCols(); i++ )
		{
			bool foundOne=false;
			for( size_t j=0; j<nbLines(); j++ )
			{
				if( _data[j][i] == 1 )
				{
					foundOne = true;
					break;
				}
			}
			if( !foundOne )
				info.nb0Cols++;
		}
		return info;
    }

	std::vector<size_t> getNonEmptyCols() const
	{
		std::vector<size_t> out;
		for( size_t col=0; col<nbCols(); col++ )
		{
			bool foundOne=false;
			for( size_t row=0; row<nbLines(); row++ )
			{
				if( _data[row][col] == 1 )
				{
					foundOne = true;
					break;
				}
			}
			if( foundOne )
				out.push_back(col);
		}
		return out;
	}

	void print( std::ostream& f, std::string msg=std::string() ) const
	{
		size_t i=0;
		f << "BinaryMatrix: " << msg << ", nbLines=" << nbLines() << " nbCols=" << nbCols() << "\n";
		for( auto line: *this )
		{
			f << std::setw(4) << i++ << ": | ";

			for( size_t i=0; i<line.size(); i++ )
			{
				f << line[i];
				if( !((i+1)%4) && i != line.size()-1 )
					f << '.';
			}
			f << " | #" << line.count() << "\n";
		}
	}

/// Returns a vector having as size the number of columns and holding the number of "1" the column has
	std::vector<size_t> getColumnCount() const
	{
		std::vector<size_t> out( nbCols(), 0 );
		for( size_t col=0; col<nbCols(); col++ )
		{
			size_t n=0;
			for( size_t row=0; row<nbLines(); row++ )
			{
				if( _data[row][col] == 1 )
					n++;
			}
			out[col] = n;
		}
		return out;
	}
};

// TEMP
} // namespace priv
} // namespace udgcd

#include "wrapper_m4ri_convert.hpp"
namespace udgcd {
namespace priv {

//-------------------------------------------------------------------------------------------
/// Print vector of vectors of bits
#if 0
template<typename T>
void
printBitVectors( std::ostream& f, const T& vec )
{
	f << "Binary vectors for each paths, #="<< vec.size() << '\n';
	for( size_t i=0; i<vec.size(); i++ )
	{
		f << i << ": ";
		printBitVector( f, vec[i] );
	}
//	f << "\n";
}
#endif
//-------------------------------------------------------------------------------------------
/// Recursive function, explores edges connected to \c v1 until we find a cycle
/**
\warning Have to be sure there \b is a cycle, else infinite recursion !
*/
template <class Vertex, class Graph>
bool
explore(
	const Vertex& v1,                            ///< the starting vertex we want to explore
	const Graph&  g,
	std::vector<std::vector<Vertex>>& vv_paths,
	std::vector<std::vector<Vertex>>& v_cycles, ///< this is where we store the paths that have cycles
	int depth = 0
)
{
//	PRINT_FUNCTION_2 << " depth=" << depth << " v1=" << v1 << "\n";

	++depth;
	static int max_depth = std::max( depth, max_depth );
	assert( vv_paths.size()>0 );

//	typename boost::graph_traits<Graph>::out_edge_iterator ei, ei_end;
//	boost::tie(ei, ei_end) = out_edges( v1, g );

//	COUTP << "nb of edges = " << ei_end - ei << "\n";
//	size_t edge_idx = 0;

//	std::vector<Vertex> src_path = vv_paths[vv_paths.size()-1];
	std::vector<Vertex> src_path = vv_paths.back();

#ifdef UDGCD_DEV_MODE
//	COUT << "src_path: "; PrintVector( std::cout, src_path );
//	int iter=0;
//	auto tmp = boost::out_edges( v1, g );
//	size_t nbedges = tmp.second - tmp.first;
#endif

	bool found = false;
	for( auto oei = boost::out_edges( v1, g ); oei.first != oei.second; ++oei.first ) //, ++edge_idx )         // iterating on all the output edges
	{
		bool b = false;
		Vertex v2a = boost::source(*oei.first, g);
		Vertex v2b = boost::target(*oei.first, g);
#ifdef UDGCD_DEV_MODE
//		COUT << ++iter << '/' << nbedges << " - v1=" << v1 << ": connected edges v2a=" << v2a << " v2b=" << v2b << "\n";
#endif
		if( v2b == v1 && v2a == src_path[0] ) // we just found the edge that we started on, so no need to finish the current iteration, just move on.
			continue;

		std::vector<Vertex> newv(src_path);  // create new path from source
		bool AddNode = true;
		if( newv.size() > 1 )
			if( newv[ newv.size()-2 ] == v2b )
				AddNode = false;

		if( AddNode )
		{
			if( std::find( newv.cbegin(), newv.cend(), v2b ) != newv.cend() )
			{
				newv.push_back( v2b );
//				COUT << "*** FOUND CYCLE: "; PrintVector( std::cout, newv );
				v_cycles.push_back( newv );
				return true;
			}
			else
			{
				newv.push_back( v2b );         // else add'em and continue
//				COUT << "  -adding vector ";  for( const auto& vv:newv )	COUT << vv << "-"; COUT << "\n";
				vv_paths.push_back( newv );
				b = explore( v2b, g, vv_paths, v_cycles, depth );
			}
		}
		if( b )
			found = true;
	}
	return found;
}
//-------------------------------------------------------------------------------------------
#if 0
/// Private, don't use.
/**
 Remove twins : vector that are the same, but in reverse order
*/
template<typename T>
std::vector<std::vector<T>>
removeOppositePairs( const std::vector<std::vector<T>>& v_cycles )
{
	PRINT_FUNCTION;
	assert( v_cycles.size() );

	std::vector<std::vector<T>> out;      // output vector
	out.reserve( v_cycles.size() );       // preallocate memory

	std::vector<bool> flags( v_cycles.size(), true ); // some flags to keep track of which elements are reversed

	for( size_t i=0; i<v_cycles.size()-1; ++i )
	{
		if( flags[i] )
		{
			out.push_back( v_cycles[i] );                        //  1 - add current vector into output

#ifdef UDGCD_DEV_MODE
			COUT << "-Considering path " << i << ":  "; PrintVector( std::cout, v_cycles[i] );
#endif
			std::vector<T> rev = v_cycles[i];                       // step 1: build a reversed copy of the current vector
			std::reverse( rev.begin(), rev.end() );
			for( size_t j=i+1; j<v_cycles.size(); ++j )                  // step 2: parse the rest of the list, and check
				if( flags[j] && rev == v_cycles[j] )                     // if similar, then
				{
//					out.push_back( v_cycles[i] );                        //  1 - add current vector into output
					flags[j] = false;                                 //  2 -  invalidate the reversed one
#ifdef UDGCD_DEV_MODE
					COUT << " -> discarding path " << j << ":  "; PrintVector( std::cout, v_cycles[j] );
#endif
				}
		}
	}
// adding last one, if not discarded

	if( flags.back() )
		out.push_back( v_cycles.back() );

	return out;
}
#endif
//-------------------------------------------------------------------------------------------
template<typename T>
void
putSmallestElemFirst( std::vector<T>& vec )
{
	auto it = std::min_element( vec.begin(), vec.end() );     // rotate so that smallest is first
	std::rotate( vec.begin(), it, vec.end() );
}
//-------------------------------------------------------------------------------------------
#if 0
/// Private, don't use.
/**
Helper function for RemoveIdentical()

Given an input vector "DABCD", it will return "ABCD" (removal of duplicate element, and first element is the smallest)
*/
template<typename T>
std::vector<T>
GetSortedTrimmed( const std::vector<T>& v_in )
{
	assert( v_in.front() == v_in.back() ); // check that this is a cycle
	assert( v_in.size() > 2 );             // a (complete) cycle needs to be at least 3 vertices long

	std::vector<T> v_out( v_in.size() - 1 );                      // Trim: remove
	std::copy( v_in.cbegin(), v_in.cend()-1, v_out.begin() );     //  last element

	putSmallestElemFirst( v_out );

	if( v_out.back() < v_out[1] )                     // if we have 1-4-3-2, then
	{
		std::reverse( v_out.begin(), v_out.end() );   // we transform it into 2-3-4-1
		putSmallestElemFirst( v_out );                // and put smallest first: 1-2-3-4
	}
	return v_out;
}
#endif
//-------------------------------------------------------------------------------------------
/// Removes the parts that are not part of the cycle.
/**
Example:
- in: 1-2-3-4-5-3
- out: 3-4-5
*/
template<typename T>
std::vector<T>
findTrueCycle( const std::vector<T>& cycle )
{
//	PRINT_FUNCTION;
//	COUT << "in: "; PrintVector( std::cout, cycle );
	assert( cycle.size() > 2 ); // 3 or more nodes
	if( cycle.size() == 3 )     // if 3 nodes, just return the input path
		return cycle;

	std::vector<T> out;
	out.reserve( cycle.size() );

	bool done = false;
	for( size_t i=0; i<cycle.size()-1 && !done; ++i )
	{
		const T& n1 = cycle[i];
		for( size_t j=i+2; j<cycle.size() && !done; ++j )
		{
			const T& n2 = cycle[j];
			if( n1 == n2 )
			{
				out.resize( j-i );
				std::copy( std::begin(cycle) + i, std::begin(cycle) + j, std::begin(out) );
				done = true;
			}
		}
	}
	putSmallestElemFirst( out );   // if we have 4-3-2-1, then transform into 1-4-3-2

	if( out.back() < out[1] )                     // if we have 1-4-3-2, then
	{
		std::reverse( out.begin(), out.end() );   // we transform it into 2-3-4-1
		putSmallestElemFirst( out );                // and put smallest first: 1-2-3-4
	}

//	COUT << "out: "; PrintVector( std::cout, out );
	return out;
}
//-------------------------------------------------------------------------------------------
/// Removes for each cycle the parts that are not part of the cycle.
/**
Example:
- in: 1-2-3-4-5-3
- out: 3-4-5

\todo We make a preallocation of the output vector, using the size of the input vector.
However, the output vector has a size usually less than 10 times less the size of the input vector.
Thus there might be some memory saving to do here.
*/
template<typename T>
std::vector<std::vector<T>>
cleanCycles( const std::vector<std::vector<T>>& v_cycles )
{
	PRINT_FUNCTION;
	assert( v_cycles.size() );

	std::vector<std::vector<T>> out;
	out.reserve( v_cycles.size() );

	for( const auto& cycle: v_cycles )
	{
		auto newcy = findTrueCycle( cycle );
		if( std::find( std::begin(out), std::end(out), newcy ) == std::end(out) )     // add to output vector only if not already present
			out.push_back( newcy );
	}
	return out;
}
//-------------------------------------------------------------------------------------------
#if 0
/// Private, don't use.
/**
Remove identical strings that are the same up to the starting point
It also sorts the paths by rotating them so that the node of smallest index is first
*/
template<typename T>
std::vector<std::vector<T>>
RemoveIdentical( const std::vector<std::vector<T>>& v_cycles )
{
	PRINT_FUNCTION;
	assert( v_cycles.size() );

	if( v_cycles.size() == 1 )                                   // if single path in input, then we justs add it, after trimming/sorting
	{
		std::vector<std::vector<T>> out( 1, GetSortedTrimmed( v_cycles[0] ) );
		return out;
	}

	std::vector<std::vector<T>> out( v_cycles.size() );
	for( size_t i=0; i<v_cycles.size(); i++ )            // 1 - fill output vector with sorted/trimmed paths
		out[i] = GetSortedTrimmed( v_cycles[i] );

	std::sort( out.begin(), out.end() );                 // 2 - sort
	out.erase(                                           // 3 - erase the ones that are
		std::unique( out.begin(), out.end() ),           //  consecutive duplicates
		out.end()
	);

	return out;
}
#endif

//-------------------------------------------------------------------------------------------
#ifdef UDGCD_REMOVE_NONCHORDLESS
/// Returns true if vertices \c v1 and \c v2 are connected by an edge
/**
http://www.boost.org/doc/libs/1_59_0/libs/graph/doc/IncidenceGraph.html#sec:out-edges
*/
template<typename vertex_t, typename graph_t>
bool
areConnected( const vertex_t& v1, const vertex_t& v2, const graph_t& g )
{
	auto pair_edge = boost::out_edges( v1, g );                      // get iterator range on edges
	for( auto it = pair_edge.first; it != pair_edge.second; ++it )
		if( v2 == boost::target( *it, g ) )
			return true;
	return false;
}
//-------------------------------------------------------------------------------------------
/// Return true if cycle is chordless
/**
See
- https://en.wikipedia.org/wiki/Cycle_(graph_theory)#Chordless_cycles

Quote:
<BLOCKQUOTE>
"A chordless cycle in a graph, also called a hole or an induced cycle, is a cycle such that
no two vertices of the cycle are connected by an edge that does not itself belong to the cycle."
</BLOCKQUOTE>
*/
template<typename vertex_t, typename graph_t>
bool
isChordless( const std::vector<vertex_t>& path, const graph_t& g )
{
	if( path.size() < 4 ) // no need to test if less than 4 vertices
		return true;

	for( size_t i=0; i<path.size()-3; ++i )
	{
		for( size_t j=i+2; j<path.size()-1; ++j )

		if( areConnected( path[i], path[j], g ) )
			return false;
	}
	return true;
}
//-------------------------------------------------------------------------------------------
/// Third step, remove non-chordless cycles
template<typename vertex_t, typename graph_t>
std::vector<std::vector<vertex_t>>
removeNonChordless( const std::vector<std::vector<vertex_t>>& v_in, const graph_t& g )
{
	PRINT_FUNCTION;

	std::vector<std::vector<vertex_t>> v_out;
	v_out.reserve( v_in.size() ); // to avoid unnecessary memory reallocations and copies
    for( const auto& cycle: v_in )
		if( isChordless( cycle, g ) )
			v_out.push_back( cycle );
	return v_out;
}
#endif // UDGCD_REMOVE_NONCHORDLESS
//-------------------------------------------------------------------------------------------
/// holds two vertices, used in RemoveRedundant()
/// \deprecated ???
template <typename vertex_t>
struct VertexPair
{
	vertex_t v1,v2;
	VertexPair() {}
	VertexPair( vertex_t va, vertex_t vb ): v1(va), v2(vb)
	{
		if( v2<v1 )
			std::swap( v1, v2 );
	}
	// 2-3 is smaller than 2-4
	friend bool operator < ( const VertexPair& vp_a, const VertexPair& vp_b )
	{
		if( vp_a.v1 < vp_b.v1 )
			return true;
		if( vp_a.v1 == vp_b.v1 )
			return ( vp_a.v2 < vp_b.v2 );
		return false;
	}

#ifdef UDGCD_DEV_MODE
	friend std::ostream& operator << ( std::ostream& s, const VertexPair& vp )
	{
		s << '(' << vp.v1 << '-' << vp.v2 << ')';
		return s;
	}
#endif
};
//-------------------------------------------------------------------------------------------
#if 0
template<typename vertex_t>
void
PrintSet( const std::set<VertexPair<vertex_t>>& set_edges, std::string msg )
{
	std::cout << "set: " << msg << '\n';
	for( const auto& e: set_edges )
		std::cout << e << '-';
	std::cout << '\n';
}
#endif
//-------------------------------------------------------------------------------------------
/// Builds the binary vector \c binvect associated to the cycle \c cycle.
/// The index vector \c idx_vec is used to fetch the index in binary vector from the two vertices indexes
/**
- sample input: 1-3-5
*/
template<typename vertex_t>
void
buildBinaryVector(
	const std::vector<vertex_t>& cycle,    ///< input cycle
	BinaryVec&                  binvect,  ///< output binary vector (must be allocated)
	const std::vector<size_t>&   idx_vec ) ///< reference index vector, see how it is build in \ref buildBinaryMatrix() and \ref buildBinaryIndexVec(()
{
	PRINT_FUNCTION;
	assert( binvect.size()>0 );
	for( size_t i=0; i<cycle.size(); ++i )
	{
		VertexPair<vertex_t> vp( (i==0 ? cycle[cycle.size()-1] : cycle[i-1]), cycle[i] );
		size_t idx = idx_vec[vp.v1] + vp.v2 - 1;
		assert( idx < binvect.size() );
		binvect[idx] = 1;
	}
//	printBitVector( std::cout, binvect );
}
//-------------------------------------------------------------------------------------------
/// Build table of series $y_n = y_{n-1}+N-n-1$
/**
This is needed to build the binary vector associated with a path, see \ref buildBinaryVector()

For example, if 6 vertices (indexes 0 to 5), then there are 6*(6-1)/2 = 15 possible edges:
\verbatim
        0  1  2  3  4  5  6  7  8  9  10 11 12 13 14
edges:  01-02-03-04-05-12-13-14-15-23-24-25-34-35-45
\endverbatim

This function builds a vector of 6-1=5 elements:
\verbatim
0-4-7-9-10
\endverbatim
With those values, we get the index of a given edge in the binary vector with the formulae:
\f$ idx = idx_vec[v1] + v2 - 1 \f$
with \f$ v1 \f$ being the smallest vertex and \f$ v2 \f$ the other one.

\todo Maybe we can remove the first value, as it is always 0.
*/
std::vector<size_t>
buildBinaryIndexVec( size_t nbVertices )
{
	PRINT_FUNCTION;

	std::vector<size_t> idx_map( nbVertices-1 );
	idx_map[0] = 0;
	for( size_t i=1;i<nbVertices-1; i++ )
		idx_map[i] = idx_map[i-1] + nbVertices - i - 1;
	return idx_map;
}
//-------------------------------------------------------------------------------------------
/// Builds all the binary vectors for all the cycles
template<typename vertex_t>
BinaryMatrix
buildBinaryMatrix(
	const std::vector<std::vector<vertex_t>>& v_cycles,     ///< input cycles
	size_t                                    nbVertices    ///< nb of vertices of the graph
)
{
	PRINT_FUNCTION;

	size_t nbCombinations = nbVertices * (nbVertices-1) / 2;
	BinaryMatrix out( v_cycles.size(), nbCombinations );  // lines x cols

//	assert( v_cycles.size() == v_binvect.size() );


	std::vector<size_t> idx_vec = buildBinaryIndexVec( nbVertices );
//	COUT << "idx_vec: "; PrintVector( std::cout, idx_vec );

	for( size_t i=0; i<v_cycles.size(); i++ )
		buildBinaryVector( v_cycles[i], out.line(i), idx_vec );

	return out;
}
//-------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
/// See buildReverseBinaryMap()
using RevBinMap = std::vector<std::pair<size_t,size_t>>;

//-------------------------------------------------------------------------------------------
/// Builds an table giving from an index in the binary vector the indexes of the two vertices
/// that are connected. See \ref convertBC2VC()
/**
Size: \f$ n*(n-1)/2 \f$

Example for n=5 (=> size=10)
\verbatim
0 | 0 1
1 | 0 2
2 | 0 3
3 | 0 4
4 | 1 2
5 | 1 3
6 | 1 4
7 | 2 3
8 | 2 4
9 | 3 4
\endverbatim
*/
RevBinMap
buildReverseBinaryMap( size_t nb_vertices )
{
	PRINT_FUNCTION;
	size_t nb_combinations = nb_vertices*(nb_vertices-1)/2;
	COUT << "nb_vertices=" << nb_vertices << " nb_combinations=" << nb_combinations << '\n';
	RevBinMap out( nb_combinations );
	size_t v1 = 0;
	size_t v2 = 1;
	for( size_t i=0; i<nb_combinations; ++i )
	{
		if( v2 == nb_vertices )
		{
			v1++;
			v2 = v1 + 1;
		}
		out[i].first  = v1;
		out[i].second = v2++;
//		COUT << i << ": (" << out[i].first << " - " << out[i].second << ")\n";
	}
	return out;
}
//-------------------------------------------------------------------------------------------
/// Returns false if a given vertex appears more than once in the vector \c vp
template<typename vertex_t>
bool
checkVertexPairSet( const std::vector<std::pair<vertex_t,vertex_t>>& vp, bool print=true )
{
	std::map<vertex_t,int> vmap;
	bool correct = true;
    for( auto p: vp )
    {
		vmap[p.first]++;
		vmap[p.second]++;
		if( vmap[p.first] > 2 )
		{
			if( print )
				std::cout << __FUNCTION__ << "(): Error, vertex " << p.first << " appears " << vmap[p.first] << " times in set\n";
			correct = false;
		}
		if( vmap[p.second] > 2 )
		{
			if( print )
				std::cout << __FUNCTION__ << "(): Error, vertex " << p.second << " appears " << vmap[p.second] << " times in set\n";
			correct = false;
		}
	}
	return correct;
}
//-------------------------------------------------------------------------------------------
template<typename vertex_t>
std::vector<std::pair<vertex_t,vertex_t>>
buildPairSetFromBinaryVec_v2(
	const BinaryVec& v_in,          ///< A binary vector holding 1 at each position where there is an edge
	const RevBinMap& rev_map,       ///< Reverse map, see buildReverseBinaryMap()
	const std::vector<size_t>& nec  ///< non-empty columns of the original matrix
)
{
	std::vector<std::pair<vertex_t,vertex_t>> v_out;
	for( size_t i=0; i<v_in.size(); ++i )
	{
		if( v_in[i] == 1 ) // if we find a '1', then we have found a connection
		{
			vertex_t v1 = rev_map[nec[i]].first;   // the two nodes
			vertex_t v2 = rev_map[nec[i]].second;  // that are connected
			v_out.push_back( std::make_pair(v1,v2) );
		}
	}
	return v_out;
}

//-------------------------------------------------------------------------------------------
/// Fill map with adjacent nodes
/**
The idea here is to avoid doing an extensive search each time to see if node is already present, which
can be costly for large cycles
*/
template<typename vertex_t>
std::vector<std::pair<vertex_t,vertex_t>>
buildPairSetFromBinaryVec(
	const BinaryVec& v_in,          ///< A binary vector holding 1 at each position where there is an edge
	const RevBinMap& rev_map        ///< Reverse map, see buildReverseBinaryMap()
)
{
	std::vector<std::pair<vertex_t,vertex_t>> v_out;
	for( size_t i=0; i<v_in.size(); ++i )
	{
		if( v_in[i] == 1 ) // if we find a '1', then we have found a connection
		{
			vertex_t v1 = rev_map[i].first;   // the two nodes
			vertex_t v2 = rev_map[i].second;  // that are connected
			v_out.push_back( std::make_pair(v1,v2) );
		}
	}
	return v_out;
}


//-------------------------------------------------------------------------------------------
/// Builds the final cycle
/**
Takes as input a set of pairs:
\verbatim
12-18
12-22
9-18
9-4
4-22
\endverbatim
and will return the cycle: 4-9-18-12-22
*/
template<typename vertex_t>
std::vector<vertex_t>
buildFinalCycle( const std::vector<std::pair<vertex_t,vertex_t>>& v_pvertex )
{
	PRINT_FUNCTION;

	assert( !v_pvertex.empty() );
	std::vector<vertex_t> v_out(2);
	v_out[0] = v_pvertex[0].first;
	v_out[1] = v_pvertex[0].second;
	size_t curr_idx = 0;
	size_t curr_v   = v_out[1];
	size_t iter = 0;
	do
	{
		++iter;
//		COUT << "\n* iter " << iter << " curr_idx=" << curr_idx << " curr_v=" << curr_v << '\n';
		bool stop = false;
		for( size_t i=1; i<v_pvertex.size(); i++ )       // search for next one
		{
			if( i != curr_idx )
			{
				auto p = v_pvertex[i];
				if( curr_v == p.first )
				{
					v_out.push_back( p.second );
					curr_v   = p.second;
					curr_idx = i;
					stop     = true;
				}
				else
				{
					if( curr_v == p.second )
					{
						v_out.push_back( p.first );
						curr_v   = p.first;
						curr_idx = i;
						stop     = true;
					}
				}
			}
			if( stop )
				break;
		}
	}
	while( curr_v != v_out[0] );      // while we don't cycle

//	PrintVector( std::cout, v_out );
	v_out.pop_back();
//	PrintVector( std::cout, v_out );

	return v_out;
}

//-------------------------------------------------------------------------------------------
/// TEMP !!!
template<typename vertex_t>
std::vector<vertex_t>
buildFinalCycle_TEMP( const std::vector<std::pair<vertex_t,vertex_t>>& v_pvertex )
{
	assert( !v_pvertex.empty() );
	std::vector<vertex_t> v_out(2);
	v_out[0] = v_pvertex[0].first;
	v_out[1] = v_pvertex[0].second;
	size_t curr_idx = 0;
	size_t curr_v   = v_out[1];
	size_t iter = 0;
	do
	{
		++iter;
//		COUT << "\n* iter " << iter << " curr_idx=" << curr_idx << " curr_v=" << curr_v << '\n';
		bool stop = false;
		for( size_t i=1; i<v_pvertex.size(); i++ )       // search for next one
		{
			if( i != curr_idx )
			{
				auto p = v_pvertex[i];
				if( curr_v == p.first )
				{
					v_out.push_back( p.second );
					curr_v   = p.second;
					curr_idx = i;
					stop     = true;
				}
				else
				{
					if( curr_v == p.second )
					{
						v_out.push_back( p.first );
						curr_v   = p.first;
						curr_idx = i;
						stop     = true;
					}
				}
			}
			if( stop )
				break;
		}
	}
	while( curr_v != v_out[0] && iter<20 );      // while we don't cycle

//	PrintVector( std::cout, v_out );
	v_out.pop_back();
//	PrintVector( std::cout, v_out );

	return v_out;
}

//-------------------------------------------------------------------------------------------
/// Similar to convertBC2VC() but to be used in case we do the "matrix reduction" trick
template<typename vertex_t>
std::vector<vertex_t>
convertBC2VC_v2(
	const BinaryVec& v_in,          ///< input binary path
	const RevBinMap&  rev_map,      ///< required map, has to be build before, see buildReverseBinaryMap()
	const std::vector<size_t>& nec, ///< non-empty columns
	size_t&           iter
)
{
	PRINT_FUNCTION;
// step 1: build set of pairs from binary vector
	auto v_pvertex = buildPairSetFromBinaryVec_v2<vertex_t>( v_in, rev_map, nec );
	assert( v_pvertex.size()>0 );

#if 1
	std::cout << "VERTEX MAP v2: size=" << v_pvertex.size() << "\n";
	size_t i = 0;
	for( const auto& vp: v_pvertex )
		std::cout << i++ << ":" << vp.first << "-" << vp.second << "\n";
#endif

	if( false == checkVertexPairSet( v_pvertex ) )
	{
		std::cout << "Fatal error: invalid set of pairs\n";
		std::exit(1);
	}

// step 2: build cycle from set of pairs
	return buildFinalCycle( v_pvertex );
}

//-------------------------------------------------------------------------------------------
/// Convert, for a given graph, a Binary Cycle (BC) \c v_in to a Vertex Cycle (VC)
/**
Algorithm:
-using rev_map, that gives for a given index in the binary vector the two corresponding vertices

* step 1: build a map, giving for each vertex, the next one

* step 2: parse the map, and add the vertices to the output vector

\todo The downside of this approach is that we need to build before the \c rev_map, that can be pretty big...
Maybe we can find a better way ?

\bug discovered on 2020-03-09. To trigger: bin/read_graph samples2/graph_1583824532.txt.
*/
template<typename vertex_t>
std::vector<vertex_t>
convertBC2VC(
	const BinaryVec& v_in,         ///< input binary path
	const RevBinMap&  rev_map,      ///< required map, has to be build before, see buildReverseBinaryMap()
	size_t&           iter
)
{
	assert( v_in.size() == rev_map.size() );

	PRINT_FUNCTION;
//	printBitVector( std::cout, v_in );

// step 1: build set of pairs from binary vector
	auto v_pvertex = buildPairSetFromBinaryVec<vertex_t>( v_in, rev_map );
	assert( v_pvertex.size()>0 );

#if 0
	std::cout << "VERTEX MAP: size=" << v_pvertex.size() << "\n";
	size_t i = 0;
	for( const auto& vp: v_pvertex )
		std::cout << i++ << ":" << vp.first << "-" << vp.second << "\n";
#endif

	if( false == checkVertexPairSet( v_pvertex ) )
	{
		std::cout << "Fatal error: invalid set of pairs\n";
		std::exit(1);
	}

// step 2: extract vertices from map
	return buildFinalCycle( v_pvertex );
}
//-------------------------------------------------------------------------------------------
/// Gaussian binary elimination
/**
- Input: a binary matrix
- Output: a reduced matrix

Assumes no identical rows
*/
//template<typename vertex_t> // TEMP
inline
BinaryMatrix
gaussianElim( BinaryMatrix& m_in, size_t& nbIter ) // /* TEMP */, size_t nbVertices, const std::vector<size_t>& nec )
{
	PRINT_FUNCTION;
	size_t col = 0;
	size_t nb_rows = m_in.nbLines();
	size_t nb_cols = m_in.nbCols();
	assert( nb_rows > 1 );

	BinaryMatrix m_out;

	nbIter = 0;
	bool done = false;

//	m_in.print( std::cout, "m_in INITIAL" );

// TEMP
//	auto rev_map = buildReverseBinaryMap( nbVertices );

	std::vector<bool> tag(nb_rows,false);
	do
	{
		++nbIter;
		COUT << "\n* start iter " << nbIter << ", current col=" << col
			<< " #tagged lines = " << std::count( tag.begin(),tag.end(), true ) << "\n";

		for( size_t row=0; row<nb_rows; row++ )                // search for first row with a 1 in current column
		{
//			COUT << "considering line " << row << "\n";
			if( tag[row] == false && m_in.line(row)[col] == 1 )    // AND not tagged
			{
				COUT << "row: " << row << ": found 1 in col " << col << "\n"; // found pivot
//				printBitVector( std::cout, m_out.line(row) );
				m_out.addLine( m_in.line(row) );
				COUT << "Adding line " << row << " to OUTMAT at line " << m_out.nbLines()-1 << '\n';

//				printBitVector( std::cout, m_in.line(row) );
//				printBitVector( std::cout, m_out.line(m_out.nbLines()-1) );
				tag[row] = true;
				if( row < nb_rows-1 )
				{
//					for( size_t i=0; i<nb_rows; i++ )      // search for all following rows that have a 1 in that column
//						if( i != row )
					for( size_t i=row+1; i<nb_rows; i++ )      // search for all following rows that have a 1 in that column
					{
						if( tag[i] == false )                  // AND that are not tagged.
							if( m_in.line(i)[col] == 1 )            // it there is, we XOR them with initial line
							{
//								std::cout << " -row " << i << " changes:\nwas: "; printBitVector( std::cout, m_in.line(i) );
								m_in.line(i) = m_in.line(i) ^ m_in.line(row);
//								std::cout << "now: "; printBitVector( std::cout, m_in.line(i) );
#if 0
								auto v_pvertex = buildPairSetFromBinaryVec_v2<vertex_t>( m_in.line(i), rev_map, nec );
								if( false == checkVertexPairSet( v_pvertex, false ) )
									COUT << "Invalid vector!\n";
#endif
							}
					}
				}
				COUT << "BREAK loop\n";
				break;
			}
		}
		COUT << "switch to next col\n";
		col++;
		if( col == nb_cols )
		{
			COUT << "All columns done, end\n";
			done = true;
		}
		if( std::find(tag.begin(),tag.end(), false ) == tag.end() )
		{
			COUT << "All lines tagged, end\n";
			done = true;
		}
#if 0
	COUT << "-m_in: " << m_in.nbLines() << "x" << m_in.nbCols() << '\n';
		m_in.print( std::cout,  "m_in" );
	COUT << "-m_out: " << m_out.nbLines() << "x" << m_out.nbCols() << '\n';
		m_out.print( std::cout, "m_out" );
#endif
	}
	while( !done );
	return m_out;
}

//-------------------------------------------------------------------------------------------
/// Similar tp convertBinary2Vertex_v2() but only returns a set of vertex pairs. ONLY for CHECKING !
template<typename vertex_t,typename graph_t /* TEMP */>
std::vector<std::pair<vertex_t,vertex_t>>
convertBinary2Vertex_v3
(
	const BinaryMatrix& binmat,
	size_t              nbVertices,
	const std::vector<size_t>& nec,      ///< Non Empty Columns
	const graph_t& g       ///< TEMP
)
{
	PRINT_FUNCTION;
	std::vector<std::vector<vertex_t>> v_out;

	v_out.reserve( binmat.nbCols() ); // to avoid unnecessary memory reallocations and copies

	auto rev_map = buildReverseBinaryMap( nbVertices );
	COUT << "revmap size=" << rev_map.size() << '\n';

	int cnc=0;
	int i=0;
	for( auto bcycle: binmat )
	{
		std::cout << "* converting line " << i++ << "\n";
		auto pset = buildPairSetFromBinaryVec_v2<vertex_t>( bcycle, rev_map, nec );
		for( const auto& p: pset )
			std::cout << p.first << "-" << p.second << "\n";
		std::cout << "Cycle: ";
		auto cycle = buildFinalCycle_TEMP( pset );
		PrintVector( std::cout, cycle );
//		if( !isChordless(cycle, g) )
//			std::cout << "is NOT chordless: " << ++cnc << "\n";

	}
	std::exit(1);
}


//-------------------------------------------------------------------------------------------
/// Convert vector of cycles expressed as binary vectors to vector of cycles expressed as a vector of vertices.
/// Similar to convertBinary2Vertex() but to be called when using the "binary matrix reduction" trick
template<typename vertex_t>
std::vector<std::vector<vertex_t>>
convertBinary2Vertex_v2
(
	const BinaryMatrix& binmat,
	size_t              nbVertices,
	const std::vector<size_t>& nec      ///< Non Empty Columns
)
{
	PRINT_FUNCTION;
	std::vector<std::vector<vertex_t>> v_out;

	v_out.reserve( binmat.nbCols() ); // to avoid unnecessary memory reallocations and copies

	auto rev_map = buildReverseBinaryMap( nbVertices );
	COUT << "revmap size=" << rev_map.size() << '\n';

	size_t i=0;
	for( auto bcycle: binmat )
	{
		size_t nbIter2 = 0;
		++i;
		COUT << i << ": **** calling convertBC2VC_v2()\nInput:"; printBitVector( std::cout, bcycle );
		auto cycle = convertBC2VC_v2<vertex_t>( bcycle, rev_map, nec, nbIter2 );
		COUT << i << ": **** result: nbIter=" << nbIter2 << '\n';
		PrintVector( std::cout, cycle );
		v_out.push_back( cycle );
	}
	return v_out;
}
//-------------------------------------------------------------------------------------------
/// Convert vector of cycles expressed as binary vectors to vector of cycles expressed as a vector of vertices
template<typename vertex_t>
std::vector<std::vector<vertex_t>>
convertBinary2Vertex
(
	const BinaryMatrix& binmat,
	size_t              nbVertices
)
{
	PRINT_FUNCTION;
	std::vector<std::vector<vertex_t>> v_out;

	v_out.reserve( binmat.nbCols() ); // to avoid unnecessary memory reallocations and copies

	auto rev_map = buildReverseBinaryMap( nbVertices );
	COUT << "heckVertexPairSet" << rev_map.size() << '\n';

	size_t i=0;
	for( auto bcycle: binmat )
	{
		size_t nbIter2 = 0;
		++i;
//		std::cout << i << ": **** calling convertBC2VC()\nInput:"; printBitVector( std::cout, bcycle );
		auto cycle = convertBC2VC<vertex_t>( bcycle, rev_map, nbIter2 );
//		std::cout << i << ": **** result: nbIter=" << nbIter2 << '\n';
		v_out.push_back( cycle );
	}
	return v_out;
}
//-------------------------------------------------------------------------------------------
/// Returns the same matrix but with empty cols removed
BinaryMatrix
reduceMatrix( const BinaryMatrix& m_in, const std::vector<size_t>& nonEmptyCols )
{
	BinaryMatrix out( m_in.nbLines() );
	for( auto idx: nonEmptyCols )
		out.addCol( m_in.getCol(idx) );
	return out;
}

//-------------------------------------------------------------------------------------------
/// Post-process step: removes cycles based on Gaussian Elimination
/**
arg is not const, because it gets sorted here.
*/
template<typename vertex_t,typename graph_t /* TEMP */>
std::vector<std::vector<vertex_t>>
removeRedundant( std::vector<std::vector<vertex_t>>& v_in, size_t nbVertices,
const graph_t& g /* TEMP */ )
{
	PRINT_FUNCTION;

// IMPORTANT: the code below assumes we have at least 3 cycles, so lets exit right away if not !
	if( v_in.size() < 3 )
		return v_in;

// build for each cycle its associated binary vector
	auto binMat_in = buildBinaryMatrix( v_in, nbVertices );

// just for checking purposes
	convertBinary2Vertex<vertex_t>( binMat_in, nbVertices );

//	printBitMatrix( std::cout, binMat_in, "removeRedundant(): input binary matrix" );

	size_t nbIter1 = 0;

//	binMat_in.print( std::cout, "removeRedundant(): input binary matrix" );
	binMat_in.getInfo().print( std::cout );//, "removeRedundant(): input binary matrix" );

	auto cc_in = binMat_in.getColumnCount();

#ifdef UDGCD_REDUCE_MATRIX
	auto nec = binMat_in.getNonEmptyCols();
	auto bm_in2 = reduceMatrix( binMat_in, nec );

	COUT << "CHECK bm_in2\n";
convertBinary2Vertex_v2<vertex_t>( bm_in2, nbVertices, nec ); // for checking

//	COUT << "bm_in2:\n"; bm_in2.print( std::cout );
//	auto bm_out2 = gaussianElim<vertex_t>( bm_in2, nbIter1, nbVertices, nec );

#if 0
	auto bm_out2 = gaussianElim( bm_in2, nbIter1 );
#else
	MatM4ri m4rmi = convertToM4ri( bm_in2 );
	mzd_echelonize_naive( m4rmi._data, 1 );
	auto bm_out2 = convertFromM4ri( m4rmi );
#endif

#if 1
	COUT << "bm_out2:\n"; bm_out2.print( std::cout );
#endif

	COUT << "CHECK bm_out2\n";
//	convertBinary2Vertex_v3<vertex_t>( bm_out2, nbVertices, nec, g ); // for checking

// convert back binary cycles to vertex-based cycles,
	return convertBinary2Vertex_v2<vertex_t>( bm_out2, nbVertices, nec );

#else // UDGCD_REDUCE_MATRIX
	auto binMat_out = gaussianElim( binMat_in, nbIter1 );

	COUT << "gaussianElim: nbIter=" << nbIter1 << '\n';

	binMat_out.print( std::cout, "removeRedundant(): output binary matrix" );
	binMat_out.getInfo().print( std::cout );//, "removeRedundant(): output binary matrix" );
	auto cc_out = binMat_out.getColumnCount();

#if 0
	RevBinMap revbinmap = buildReverseBinaryMap( nbVertices );

	std::cout << "col | in | out\n";
	for( size_t i=0; i<cc_in.size(); i++ )
	{
		if( cc_in[i] != 0 || cc_out[i] != 0 )
			std::cout << i <<  ": " << revbinmap[i].first << "-" << revbinmap[i].second <<  " | " <<  cc_in[i] << " | " <<  cc_out[i] << '\n';
	}
#endif

//	std::cout << "v_bpaths size=" << v_bpaths.size() << '\n';

// convert back binary cycles to vertex-based cycles,

	return convertBinary2Vertex<vertex_t>( binMat_out, nbVertices );

#endif
}

//-------------------------------------------------------------------------------------------
/// Recursive function, will iterate in graph and return true if cycle is correct
/**
End condition
 - we found the initial node as \c next
 - we cannot find in all the linked nodes the next node (the one that is after \c idx_curr in \c cycle)
*/
template<typename vertex_t, typename graph_t>
bool
checkNextNode
(
	const std::vector<vertex_t>& cycle,  ///< the cycle we are exploring
	size_t idx_curr,                     ///< current vertex index in \c cycle
	const graph_t& g                     ///< graph
)
{
	vertex_t start = cycle[0];
	vertex_t curr  = cycle[idx_curr];
	vertex_t next  = cycle[idx_curr==cycle.size()-1 ? 0 : idx_curr+1];
	assert( cycle.size() > 2 );

	for(                                             // iterate over edges
		auto it_pair = boost::out_edges( curr, g );  // of current vertex
		it_pair.first != it_pair.second;
		++it_pair.first
	)
	{
		auto vt = boost::target( *it_pair.first, g );
		if( idx_curr > 1 )
		{
			if( vt == start )    // if we meet the initial vertex
				return true;     // then, it is indeed a cycle!
		}
		if( vt == next )      // if we meet the next node, then we re-enter function
		{
			bool b = checkNextNode( cycle, ++idx_curr, g );
			if( b )                        // stop condition
				return true;
			break; // if if have found "next", we must not iterate on following nodes!
		}
	}
	return false;
}

//-------------------------------------------------------------------------------------------
/// Returns true if \c cycle is correct
/**
This function is only there for checking purposes
*/
template<typename vertex_t, typename graph_t>
bool
isACycle( const std::vector<vertex_t>& cycle, const graph_t& g )
{
//	PRINT_FUNCTION;
	if( cycle.size() > boost::num_vertices(g) )
		return false;

	return checkNextNode( cycle, 0, g );
}

//-------------------------------------------------------------------------------------------
/// Returns the number of cycles that are incorrect
/**
This function is only there for checking purposes
*/
template<typename vertex_t, typename graph_t>
size_t
checkCycles( const std::vector<std::vector<vertex_t>>& v_in, const graph_t& g )
{
	PRINT_FUNCTION;
	size_t c = 0;
	for( auto cycle: v_in )
	{
#if 0
		c += (size_t)isACycle( cycle, g );
#else
		bool b = isACycle( cycle, g );
		if( !b )
		{
			std::cout << "Error, computed cycle not a cycle:\n";
			PrintVector( std::cout, cycle );
			c++;
		}
#endif
	}
	return c;
}
//-------------------------------------------------------------------------------------------

} // namespace priv

//-------------------------------------------------------------------------------------------
/// Holds information on the cycle detection process
/// (nb of cycles at each step and timing information)
struct UdgcdInfo
{
	size_t nbRawCycles = 0;
	size_t nbCleanedCycles = 0;
	size_t nbNonChordlessCycles = 0;
	size_t nbFinalCycles = 0;

	size_t step = 0;
	constexpr static int nbSteps = 6;
	std::array<std::chrono::time_point<std::chrono::high_resolution_clock>,nbSteps> tp;

	void startTiming()
	{
		step = 0;
		for( auto& e: tp )
			e = std::chrono::high_resolution_clock::now();
	}
    void setTimeStamp()
    {
		step++;
		assert( step < nbSteps );
		tp[step] = std::chrono::high_resolution_clock::now();
    }

	void print( std::ostream& f ) const
	{
		f << "UdgcdInfo:"
			<< "\n - nbRawCycles=" << nbRawCycles
			<< "\n - nbCleanedCycles=" << nbCleanedCycles
			<< "\n - nbNonChordlessCycles=" << nbNonChordlessCycles
			<< "\n - nbFinalCycles=" << nbFinalCycles
			<< "\n";
	}

	void printCSV( std::ostream& f ) const
	{
		char sep=';';
		f << nbRawCycles << sep
			<< nbCleanedCycles << sep
			<< nbNonChordlessCycles << sep
			<< nbFinalCycles << sep;
		for( size_t i=0; i<nbSteps-1; i++ )
		{
			auto d = tp[i+1] - tp[i];
			f << std::chrono::duration_cast<std::chrono::milliseconds>(d).count();
			if( i != nbSteps-2 )
				f << sep;
		}
		f << "\n";
	}
};
//-------------------------------------------------------------------------------------------
/// Cycle detector for an undirected graph
/**
Passed by value as visitor to \c boost::undirected_dfs()

See http://www.boost.org/doc/libs/1_58_0/libs/graph/doc/undirected_dfs.html
*/
template <typename vertex_t>
struct CycleDetector : public boost::dfs_visitor<>
{
	template<typename T1, typename T2>
	friend std::vector<std::vector<T2>> findCycles( T1& );
	template<typename T1, typename T2>
	friend std::vector<std::vector<T2>> findCycles( T1&, UdgcdInfo& );

	public:
		CycleDetector()
		{
			v_source_vertex.clear();
		}
		bool cycleDetected() const { return !v_source_vertex.empty(); }
		template <class Edge, class Graph>
		void back_edge( Edge e, const Graph& g )     // is invoked on the back edges in the graph.
		{
			vertex_t vs = boost::source(e, g);
			vertex_t vt = boost::target(e, g);
	#ifdef UDGCD_PRINT_STEPS
			std::cout << " => CYCLE DETECTED! vs=" << vs << " vt=" << vt << "\n";
	#endif
			if(                                                                                                // add vertex to
				std::find( v_source_vertex.cbegin(), v_source_vertex.cend(), vs ) == v_source_vertex.cend()    // the starting point list
				&&                                                                                             // only if both are
				std::find( v_source_vertex.cbegin(), v_source_vertex.cend(), vt ) == v_source_vertex.cend()    // not already inside
			)
				v_source_vertex.push_back( vs );
		}
	private:
		static std::vector<vertex_t> v_source_vertex;
};


/// static var instanciation
template<class T>
std::vector<T> CycleDetector<T>::v_source_vertex;


//-------------------------------------------------------------------------------------------
/// Main user interface: just call this function to get the cycles inside your graph
/**
Returns a vector of cycles that have been found in the graph
*/
template<typename graph_t, typename vertex_t>
std::vector<std::vector<vertex_t>>
findCycles( graph_t& g, UdgcdInfo& info )
{
	info.startTiming();

	if( boost::num_vertices(g) < 3 || boost::num_edges(g) < 3 )
		return std::vector<std::vector<vertex_t>>();

	CycleDetector<vertex_t> cycleDetector;

// vertex color map
	std::vector<boost::default_color_type> vertex_color( boost::num_vertices(g) );
	auto idmap = boost::get( boost::vertex_index, g );
	auto vcmap = make_iterator_property_map( vertex_color.begin(), idmap );

// edge color map
	std::map<typename graph_t::edge_descriptor, boost::default_color_type> edge_color;
	auto ecmap = boost::make_assoc_property_map( edge_color );

// step 1: do a DFS
	boost::undirected_dfs( g, cycleDetector, vcmap, ecmap, 0 );

	info.setTimeStamp();

	if( !cycleDetector.cycleDetected() )             // if no detection,
		return std::vector<std::vector<vertex_t>>(); //  return empty vector, no cycles found

	std::cout << "-nbSourceVertices=" << cycleDetector.v_source_vertex.size() << '\n';
	std::vector<std::vector<vertex_t>> v_cycles;     // else, get the cycles.

// search paths only starting from vertices that were registered as source vertex
	for( const auto& vi: cycleDetector.v_source_vertex )
	{
		COUT << "\n * Start exploring from source vertex " << vi << "\n";
		std::vector<std::vector<vertex_t>> v_paths;
		std::vector<vertex_t> newv(1, vi ); // start by one of the filed source vertex
		v_paths.push_back( newv );
		priv::explore( vi, g, v_paths, v_cycles );    // call of recursive function

#ifdef UDGCD_PRINT_STEPS
//	std::cout << "considering vertex " << vi << ": v_cycle size=" << v_cycles.size() << '\n';
//	PrintPaths( std::cout, v_cycles, "temp" );
#endif

	}

	info.setTimeStamp();
	info.nbRawCycles = v_cycles.size();
	std::cout << "-Nb initial cycles: " << info.nbRawCycles << '\n';

#ifdef UDGCD_PRINT_STEPS
//	PrintPaths( std::cout, v_cycles, "Raw cycles" );
#endif

// post process 0: cleanout the cycles by removing steps that are not part of the cycle
	auto v_cycles0 = priv::cleanCycles( v_cycles );
#ifdef UDGCD_PRINT_STEPS
//	PrintPaths( std::cout, v_cycles0, "After cleanCycles()" );
#endif

	info.setTimeStamp();
	info.nbCleanedCycles = v_cycles0.size();
	std::cout << "-Nb cleaned cycles: " << info.nbCleanedCycles << '\n';

#ifdef UDGCD_DO_CYCLE_CHECKING
	if( 0 != priv::checkCycles( v_cycles0, g ) )
	{
		std::cerr << "udgcd: ERROR: INVALID CYCLE DETECTED, line " << __LINE__ << "\n";
		exit(1);
	}
#endif

#if 0
// post process 1: remove the paths that are identical but reversed
	auto v_cycles2 = priv::removeOppositePairs( v_cycles0 );
#ifdef UDGCD_PRINT_STEPS
	PrintPaths( std::cout, v_cycles2, "After removal of symmetrical cycles" );
#endif
#endif

#ifdef UDGCD_REMOVE_NONCHORDLESS
// post process 3: remove non-chordless cycles
	std::vector<std::vector<vertex_t>> v_cycles1 = priv::removeNonChordless( v_cycles0, g );
	std::cout << "-After removal of non-chordless cycles: " << v_cycles1.size() << " cycles\n";

#ifdef UDGCD_PRINT_STEPS
//	PrintPaths( std::cout, v_cycles1, "After removal of non-chordless cycles" );
#endif

	info.setTimeStamp();
	info.nbNonChordlessCycles = v_cycles1.size();

	#ifdef UDGCD_DO_CYCLE_CHECKING
	if( 0 != priv::checkCycles( v_cycles1, g ) )
	{
		std::cerr << "udgcd: ERROR: INVALID CYCLE DETECTED, line " << __LINE__ << "\n";
		exit(1);
	}
	#endif

	auto v_cycles2 = priv::removeRedundant( v_cycles1, boost::num_vertices(g), g );


#else // UDGCD_REMOVE_NONCHORDLESS

	auto v_cycles2 = priv::removeRedundant( v_cycles0, boost::num_vertices(g), g);
#ifdef UDGCD_PRINT_STEPS
	PrintPaths( std::cout, v_cycles2, "After removeRedundant()" );
#endif

#endif

#ifdef UDGCD_DO_CYCLE_CHECKING
	if( 0 != priv::checkCycles( v_cycles2, g ) )
	{
		std::cerr << "udgcd: ERROR: INVALID CYCLE DETECTED, line " << __LINE__ << "\n";
		exit(1);
	}
#endif

	info.setTimeStamp();
	info.nbFinalCycles = v_cycles2.size();
	return v_cycles2;
}
//-------------------------------------------------------------------------------------------
/// Version without second argument (default version)
template<typename graph_t, typename vertex_t>
std::vector<std::vector<vertex_t>>
findCycles( graph_t& g )
{
	UdgcdInfo info;
	return findCycles<graph_t,vertex_t>( g, info );
}

//-------------------------------------------------------------------------------------------

} // udgcd namespace end

// TEMP
//#include "wrapper_m4ri_convert.hpp"

#endif // HG_UDGCD_HPP
