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

#ifdef UDGCD_USE_M4RI
	#include "wrapper_m4ri.hpp"
#endif


#ifdef UDGCD_LOG_FUNC
	#define PRINT_FUNCTION std::cout << "*** start function " <<  __FUNCTION__ << "()\n"
	#define PRINT_FUNCTION_2 if(1) std::cout << "*** start function " <<  __FUNCTION__ << "()"
#else
	#define PRINT_FUNCTION
	#define PRINT_FUNCTION_2 if(0) std::cout
#endif

#ifdef UDGCD_DEV_MODE
	#include <iostream>
	#define COUT if(1) std::cout << std::setw(4) << __LINE__ << ": "
	#define UDGCD_PRINT_STEPS
	#define PRINT_DIFF( step, v_after, v_before ) std::cout << step << ": REMOVAL OF " << v_before.size() - v_after.size() << " cycles\n"
#else
	#define COUT if(0) std::cout
	#define PRINT_DIFF(a,b,c) ;
#endif

/// All the provided code is in this namespace
namespace udgcd {

//-------------------------------------------------------------------------------------------
template<typename T>
void
printVector( std::ostream& f, const std::vector<T>& vec )
{
	for( const auto& elem : vec )
		f << elem << "-";
	f << "\n";
}

/// Additional helper function, can be used to print the cycles found
template<typename T>
void
printPaths( std::ostream& f, const std::vector<std::vector<T>>& v_paths, const char* msg=0 )
{
	static int iter=0;
	f << "Paths (" << iter++ << "): nb=" << v_paths.size();
	if( msg )
		f << ": " << msg;
	f << "\n";

	for( size_t i=0; i<v_paths.size(); i++ )
	{
		f << " - " << i << ": ";
		printVector( f, v_paths[i] );
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
/// Holds two vertices
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

	bool operator == ( const VertexPair& p ) const
	{
		if( p.v1 != v1 )
			return false;
		if( p.v2 != v2 )
			return false;
		return true;
	}

//#ifdef UDGCD_DEV_MODE
	friend std::ostream& operator << ( std::ostream& s, const VertexPair& vp )
	{
		s << '(' << vp.v1 << '-' << vp.v2 << ')';
		return s;
	}
//#endif
};

//-------------------------------------------------------------------------------------------
/// A binary matrix, implemented as a vector of BinaryVec
/**
This type will allow to fetch some relevant information on what the matrix holds
*/
struct BinaryMatrix
{
	friend bool operator == ( const BinaryMatrix&, const BinaryMatrix& );
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
		if( nbLines() )                                    // make sure that the added vector has the same
			assert( bvec.size() == _data.back().size() );  // size as the one we are adding
		_data.push_back(bvec);
	}

	void addCol( const BinaryVec& vin )
	{
        assert( vin.size() == nbLines() );
        for( size_t i=0; i<vin.size(); i++ )
			_data[i].push_back( vin[i] );
	}

	/// Creates a binary vector, fills it with the column content, and returns it
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
	void clear()
	{
		for( auto & li: _data )
			li.clear();
	}

	void setDiag()
	{
		clear();
		for( size_t i=0; i<nbLines(); i++ )
			_data[i][i] = 1;
	}
	/// return number of ones
	size_t count() const
	{
		size_t c = 0;
		for( const auto& l: _data )
			c += l.count();
		return c;
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

	void printMat( std::ostream& f, std::string msg=std::string() ) const
	{
		size_t c=0, i=0;
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
			c += line.count();
		}
		f << "Total count=" << c << "\n";
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

inline
bool
operator == ( const BinaryMatrix& m1, const BinaryMatrix& m2 )
{
	if( m1.nbLines() != m2.nbLines() )
		return false;
	if( m1.nbCols() != m2.nbCols() )
		return false;

	for( size_t i=0; i<m1.nbLines(); i++ )
		if( m1.line(i) != m2.line(i) )
			return false;
	return true;
}


//######################
namespace deprec {
//######################

//-------------------------------------------------------------------------------------------
/// A binary matrix with an added data member so we know to which
/// edge a given column is about. Holds incidence matrix,
/**
that describes the whole graph
- rows: vertices
- cols: edges

See https://en.wikipedia.org/wiki/Incidence_matrix#Undirected_and_directed_graphs
*/
template<typename vertex_t>
struct IncidenceMatrix : public BinaryMatrix
{
	std::vector<VertexPair<vertex_t>> _columnEdge;

	IncidenceMatrix( size_t nbLines, size_t nbCols )
		: BinaryMatrix( nbLines, nbCols )
		, _columnEdge( nbCols )
	{}

	/// Set (=1) at lines v1 and v2, column \c col, and assigns comun edge
	void setPair( vertex_t v1, vertex_t v2, size_t col )
	{
		assert( v1  < nbLines() );
		assert( v2  < nbLines() );
		assert( col < nbCols() );

		_columnEdge[col] = VertexPair<vertex_t>( v1, v2 );

		auto& row1 = _data[v1];
		auto& row2 = _data[v2];
		row1[col] = row2[col] = 1;
	}

	void printMat( std::ostream& f, std::string msg=std::string() ) const
	{
		f << "IncidenceMatrix:" << msg << "\n -columns:\n";
		for( size_t i=0; i<nbCols(); i++ )
			f << i << ": " << _columnEdge[i] << "\n";
		BinaryMatrix::printMat( f, "IncidenceMatrix" );
	}
};
//-------------------------------------------------------------------------------------------

//######################
} // namespace deprec
//######################

// TEMP
} // namespace priv
} // namespace udgcd

#ifdef UDGCD_USE_M4RI
	#include "wrapper_m4ri_convert.hpp"
#endif

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
template <class vertex_t, class graph_t>
bool
explore(
	const vertex_t& v1,                            ///< the starting vertex we want to explore
	const graph_t&  gr,
	std::vector<std::vector<vertex_t>>& vv_paths,
	std::vector<std::vector<vertex_t>>& v_cycles, ///< this is where we store the paths that have cycles
	int depth = 0
)
{
//	PRINT_FUNCTION_2 << " depth=" << depth << " v1=" << v1 << "\n";

	++depth;
	static int max_depth = std::max( depth, max_depth );
	assert( vv_paths.size()>0 );

	std::vector<vertex_t> src_path = vv_paths.back();

	bool found = false;
	for( auto oei = boost::out_edges( v1, gr ); oei.first != oei.second; ++oei.first )          // iterating on all the output edges
	{
		bool b = false;
		vertex_t v2a = boost::source( *oei.first, gr );
		vertex_t v2b = boost::target( *oei.first, gr );
#ifdef UDGCD_DEV_MODE
//		COUT << ++iter << '/' << nbedges << " - v1=" << v1 << ": connected edges v2a=" << v2a << " v2b=" << v2b << "\n";
#endif
		if( v2b == v1 && v2a == src_path[0] ) // we just found the edge that we started on, so no need to finish the current iteration, just move on.
			continue;

		std::vector<vertex_t> newv(src_path);  // create new path from source
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
				b = explore( v2b, gr, vv_paths, v_cycles, depth );
			}
		}
		if( b )
			found = true;
	}
	return found;
}
//-------------------------------------------------------------------------------------------
/// putSmallestElemFirst
template<typename T>
void
putSmallestElemFirst( std::vector<T>& vec )
{
	auto it = std::min_element( vec.begin(), vec.end() );     // rotate so that smallest is first
	std::rotate( vec.begin(), it, vec.end() );
}

//-------------------------------------------------------------------------------------------
/// Normalize the cycle: puts the smallest index in first position, and reverses it if needed
/// so that the second element if less than the last one.
template<typename T>
void
normalizeCycle( std::vector<T>& cycle )
{
	assert( cycle.size() > 2 );

	putSmallestElemFirst( cycle ); // turn 2-1-4-5 into 1-4-5-2

	if( cycle.back() < cycle[1] )                     // if we have 1-4-5-2, then
	{
		std::reverse( cycle.begin(), cycle.end() );   // we transform it into 2-5-4-1
		putSmallestElemFirst( cycle );                // and put smallest first: 1-2-5-4
	}
}

//-------------------------------------------------------------------------------------------
/// Normalizes the set of cycles, see normalizeCycle()
template<typename T>
void
normalizeCycles( std::vector<std::vector<T>>& cycles )
{
	for( auto& cycle: cycles )
		normalizeCycle( cycle );
}

//-------------------------------------------------------------------------------------------
/// Removes the parts that are not part of the cycle, and normalize the order
/**
Example:
- in: 1-2-3-4-5-3
- out: 3-4-5

\sa putSmallestElemFirst()
*/
template<typename T>
std::vector<T>
findTrueCycle( const std::vector<T>& cycle )
{
	PRINT_FUNCTION;

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

\sa findTrueCycle()
*/
template<typename T>
std::vector<std::vector<T>>
cleanCycles( const std::vector<std::vector<T>>& v_cycles )
{
	PRINT_FUNCTION;
	assert( v_cycles.size() );

	std::vector<std::vector<T>> out;
	out.reserve( v_cycles.size() );

	size_t identical = 0;
	for( const auto& cycle: v_cycles )
	{
		auto newcy = findTrueCycle( cycle );
		if( std::find( std::begin(out), std::end(out), newcy ) == std::end(out) )     // add to output vector only if not already present
			out.push_back( newcy );
		else
			identical++;
	}
	std::cout << __FUNCTION__ << "(): nb of identical cycles removed=" << identical << "\n";
	return out;
}

//-------------------------------------------------------------------------------------------
/// Returns true if vertices \c v1 and \c v2 are connected by an edge
/**
http://www.boost.org/doc/libs/1_59_0/libs/graph/doc/IncidenceGraph.html#sec:out-edges

\todo Replace the calling to this function (that needs to iterate at every call)
by a static binary vector, givening the result instantly from an index value.
*/
template<typename vertex_t, typename graph_t>
bool
areConnected( const vertex_t& v1, const vertex_t& v2, const graph_t& gr )
{
	auto pair_edge = boost::out_edges( v1, gr );                      // get iterator range on edges
	for( auto it = pair_edge.first; it != pair_edge.second; ++it )
	{
		assert( boost::source( *it, gr ) == v1 );
		if( v2 == boost::target( *it, gr ) )
			return true;
	}
	return false;
}

//######################
/// Holds all chordless cycles related code, not used at present
namespace chords {
//######################

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

\warning Does not check that the path \b is a cycle !
*/
template<typename vertex_t, typename graph_t>
bool
isChordless( const std::vector<vertex_t>& path, const graph_t& gr )
{
	if( path.size() < 4 ) // no need to test if less than 4 vertices
		return true;

	for( size_t i=0; i<path.size()-2; ++i )
	{
		for( size_t j=i+2; j<path.size(); ++j )
		{
			if( i != 0 || j != path.size()-1 )
				if( areConnected( path[i], path[j], gr ) )
					return false;
		}
	}
	return true;
}
//-------------------------------------------------------------------------------------------
/// Returns input cycle but with (potential) chord(s) removed
template<typename vertex_t, typename graph_t>
std::vector<vertex_t>
removeChords( const std::vector<vertex_t>& cycle, const graph_t& gr )
{
	PRINT_FUNCTION;

	if( cycle.size() < 4 ) // no need to test if less than 4 vertices
		return cycle;

	std::vector<vertex_t> out;
	out.push_back( cycle[0] );
	size_t idx_connected = 0;
	bool connected = false;
	size_t i=0;
	for( i=0; i<cycle.size()-1; ++i )
	{
		connected = false;
		for( size_t j=i+2; j<cycle.size(); ++j )
		{
			if( i != 0 || j != cycle.size()-1 )
				if( areConnected( cycle[i], cycle[j], gr ) )
				{
					connected = true;
					idx_connected = j;
					break;
				}
		}
		if( !connected )
			out.push_back( cycle[i+1] );
		else
		{
			out.push_back( cycle[idx_connected] );
			i = idx_connected;
		}
	}

	if( i == cycle.size()-1 )
		out.push_back( cycle.back() );

	return out;
}
//-------------------------------------------------------------------------------------------
template<typename vertex_t, typename graph_t>
std::vector<std::vector<vertex_t>>
removeChords( std::vector<std::vector<vertex_t>>& cycles, const graph_t& gr )
{
	std::vector<std::vector<vertex_t>> out;
	out.reserve( cycles.size() );
	for( const auto& cycle : cycles )
		out.push_back( removeChords( cycle, gr ) );
	return out;
}

//-------------------------------------------------------------------------------------------
/// Remove non-chordless cycles
template<typename vertex_t, typename graph_t>
std::vector<std::vector<vertex_t>>
removeNonChordless( const std::vector<std::vector<vertex_t>>& v_in, const graph_t& gr )
{
	PRINT_FUNCTION;

	std::vector<std::vector<vertex_t>> v_out;
	v_out.reserve( v_in.size() ); // to avoid unnecessary memory reallocations and copies

    for( const auto& cycle: v_in )
		if( isChordless( cycle, gr ) )
			v_out.push_back( cycle );
	return v_out;
}

//######################
} // namespace chords
//######################

//-------------------------------------------------------------------------------------------
/// A vector holding a pair of indexes/vertices. See page \ref p_data_representation.
template<typename T>
using RevBinMap = std::vector<VertexPair<T>>;


//######################
/// Holds some deprecated/unused code, but kept, well... just in case.
namespace deprec {
//######################

//-------------------------------------------------------------------------------------------
/// Builds the binary vector \c binvect associated to the cycle \c cycle.
/// The index vector \c idx_vec is used to fetch the index in binary vector from the two vertices indexes
/**
This function builds the \b full vector, that is for all possible edges.
See page \ref p_data_reprensentation.

\todo 20200413: write down the reason why the function does not return the result, and passes it
as reference. Must be a reason but can't remember each time!
*/
template<typename vertex_t>
void
buildFullBinaryVector(
	const std::vector<vertex_t>& cycle,    ///< input cycle
	BinaryVec&                   binvect,  ///< output binary vector (must be allocated)
	const std::vector<size_t>&   idx_vec   ///< reference index vector, see how it is build in \ref buildBinaryMatrix() and \ref buildFullBinaryIndex(()
)
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
/// Build table of series \f$ y_n = y_{n-1}+N-n-1 \f$
/**
This is needed to build the binary vector associated with a path, see \ref buildFullBinaryVector()

For example, if 6 vertices (indexes 0 to 5), then there are \f$ 6*(6-1)/2 = 15 \f$ possible edges:
\verbatim
        0  1  2  3  4  5  6  7  8  9  10 11 12 13 14
edges:  01-02-03-04-05-12-13-14-15-23-24-25-34-35-45
\endverbatim

This function builds a vector of 6-1=5 elements:
\verbatim
0-4-7-9-10
\endverbatim
With those values, we get the index of a given edge in the binary vector with the formulae:
<code> idx = idx_vec[v1] + v2 - 1 </code>
with \c v1 being the smallest vertex and \c v2 the other one.

For example, for a graph having 6 vertices, and we want to know the index in the binary vector
for the edge between vertices 2 and 4, then the above formula gives:<br>
<code> idx = idx_vec[2] + 4 - 1 </code><br>
We have <code> idx[2] = 7 </code>, so the index of that edge will be
\f$ 7 + 4 -1 = 10 \f$<br>
This can be checked above.

See page \ref p_data_representation.

\todo Maybe we can remove the first value, as it is always 0.
*/
std::vector<size_t>
buildFullBinaryIndex( size_t nbVertices )
{
	PRINT_FUNCTION;

	std::vector<size_t> idx_map( nbVertices-1 );
	idx_map[0] = 0;
	for( size_t i=1;i<nbVertices-1; i++ )
		idx_map[i] = idx_map[i-1] + nbVertices - i - 1;
	return idx_map;
}

//-------------------------------------------------------------------------------------------
/// Builds all the binary vectors for all the cycles, using ALL potential edges (not only the ones used)
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

	std::vector<size_t> idx_vec = buildFullBinaryIndex( nbVertices );
	std::cout << "idx_vec: "; printVector( std::cout, idx_vec );

	for( size_t i=0; i<v_cycles.size(); i++ )
	{
		std::cout << "build for: "; printVector( std::cout, v_cycles[i]);
		buildFullBinaryVector( v_cycles[i], out.line(i), idx_vec );
		std::cout << " => result="; printBitVector( std::cout, out.line(i) );
	}
	return out;
}


//-------------------------------------------------------------------------------------------
/// Builds an table giving from an index in the binary vector the indexes of the two vertices
/// that are connected. See \ref convertBC2VC() and page \ref p_data_representation.
/**
\li Size: \f$ v*(v-1)/2 \f$
\li Example for \f$ v=5 \f$ (=> size=10)
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
template<typename T>
RevBinMap<T>
buildReverseBinaryMap( size_t nb_vertices )
{
	PRINT_FUNCTION;

	size_t nb_combinations = nb_vertices*(nb_vertices-1)/2;
	COUT << "nb_vertices=" << nb_vertices << " nb_combinations=" << nb_combinations << '\n';
	RevBinMap<T> out( nb_combinations );
	size_t v1 = 0;
	size_t v2 = 1;
	for( size_t i=0; i<nb_combinations; ++i )
	{
		if( v2 == nb_vertices )
		{
			v1++;
			v2 = v1 + 1;
		}
		out[i].v1 = v1;
		out[i].v2 = v2++;
	}
	return out;
}

//######################
} // namespace deprec
//######################

//-------------------------------------------------------------------------------------------
/// Returns false if a given vertex appears more than once in the vector \c vp
template<typename vertex_t>
bool
checkVertexPairSet( const std::vector<VertexPair<vertex_t>>& vp, bool print=true )
{
	std::map<vertex_t,int> vmap;
	bool correct = true;
    for( auto p: vp )
    {
		vmap[p.v1]++;
		vmap[p.v2]++;
		if( vmap[p.v1] > 2 )
		{
			if( print )
				std::cout << __FUNCTION__ << "(): Error, vertex " << p.v1 << " appears " << vmap[p.v1] << " times in set\n";
			correct = false;
		}
		if( vmap[p.v2] > 2 )
		{
			if( print )
				std::cout << __FUNCTION__ << "(): Error, vertex " << p.v2 << " appears " << vmap[p.v2] << " times in set\n";
			correct = false;
		}
	}
	return correct;
}

//######################
namespace deprec {
//######################

//-------------------------------------------------------------------------------------------
/// Convert cycle expressed as a binary vector to a Vector of Pair of Vertices (VPV)
/// See \ref p_data_representation
template<typename vertex_t, typename T>
std::vector<VertexPair<vertex_t>>
convertBinVec2VPV_v2
(
	const BinaryVec&           v_in,    ///< A binary vector holding 1 at each position where there is an edge
	const RevBinMap<T>&        rev_map, ///< Reverse map, build with buildReverseBinaryMap()
	const std::vector<size_t>& nec      ///< non-empty columns of the original matrix
)
{
	PRINT_FUNCTION;
//	std::cout << v_in << "\n";
	std::vector<VertexPair<vertex_t>> v_out;
	for( size_t i=0; i<v_in.size(); ++i )
		if( v_in[i] == 1 ) // if we find a '1', then we have found a connection
			v_out.push_back( rev_map[nec[i]] );
	return v_out;
}

//-------------------------------------------------------------------------------------------
/// Similar to convertBC2VC() but to be used in case we do the "matrix reduction" trick
template<typename vertex_t, typename T>
std::vector<vertex_t>
convertBC2VC_v2(
	const BinaryVec&           v_in,       ///< input binary path
	const RevBinMap<T>&        rev_map,    ///< required map, has to be build before, see buildReverseBinaryMap()
	const std::vector<size_t>& nec         ///< non-empty columns
)
{
	PRINT_FUNCTION;

// step 1: build set of pairs from binary vector
	auto v_pvertex = convertBinVec2VPV_v2<vertex_t>( v_in, rev_map, nec );
	assert( v_pvertex.size()>0 );

	if( false == checkVertexPairSet( v_pvertex ) )
	{
		std::cout << "Fatal error: invalid set of pairs\n";
		std::exit(1);
	}

// step 2: build cycle from set of pairs
	return convertVPV2Cycle( v_pvertex );
}

//######################
} // namespace deprec
//######################

//-------------------------------------------------------------------------------------------
/// Convert cycle expressed as a binary vector to a Vector of Pair of Vertices (VPV)
/// See \ref p_data_representation
/**
The idea here is to avoid doing an extensive search each time to see if node is already present, which
can be costly for large cycles.
*/
template<typename vertex_t, typename T>
std::vector<VertexPair<vertex_t>>
convertBinVec2VPV
(
	const BinaryVec&    v_in,          ///< A binary vector holding 1 at each position where there is an edge
	const RevBinMap<T>& rev_map        ///< Reverse map, see buildReverseBinaryMap()
)
{
	PRINT_FUNCTION;

	std::vector<VertexPair<vertex_t>> v_out;
	for( size_t i=0; i<v_in.size(); ++i )
		if( v_in[i] == 1 )                       // if we find a '1', then we have found a connection
			v_out.push_back( rev_map[i] );

	return v_out;
}
//-------------------------------------------------------------------------------------------
/**
\page p_data_representation Data Representation

A cycle can be represented in several ways:
\li as a vector of vertices.
For example:  <code>(2-6-14-17)</code>.
Order matters !
\li as a Vector of Pair of Vertices (VPV).
With the above example, this would be:
<code>( {2-6},{6-14},{14-17},{17-2} )</code>.
Here, the order doesn't matter.
\li as a binary vector, which is related to a reference index map.
<br>
The function buildFullBinaryIndex() will build the reference map, given the number of vertices, and for ALL possible edges
(even if they don't appear in the graph).
Thus, it's number of columns can be quickly large (equal to \f$ v*(v-1)/2 \f$ )

In such a vector, we have a 1 at every position where there is an edge.

To convert a binary vector to a vector of vertices is done with convertBC2VC()
or convertBC2VC_v2() if matrix reduction is used.

To convert a binary vector to a Vector of Pair of Vertices, see
convertBinVec2VPV()



*/

//-------------------------------------------------------------------------------------------
/// Convert a cycle expressed as a set of pairs (VPV:Vector of Pairs of Vertices) to
/// a vector of vertices. See \ref p_data_representation
/**
\sa convertCycle2VPV()

Takes as input a vector of pairs:<br>
<code>{12-18},{12-22},{9-18},{9-4},{4-22}</code><br>
and will return the cycle:<br>
<code>4-9-18-12-22</code>
*/
template<typename vertex_t>
std::vector<vertex_t>
convertVPV2Cycle( const std::vector<VertexPair<vertex_t>>& v_pvertex )
{
	PRINT_FUNCTION;

	assert( !v_pvertex.empty() );
	std::vector<vertex_t> v_out(2);
	v_out[0] = v_pvertex[0].v1;
	v_out[1] = v_pvertex[0].v2;
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
				if( curr_v == p.v1 )
				{
					v_out.push_back( p.v2 );
					curr_v   = p.v2;
					curr_idx = i;
					stop     = true;
				}
				else
				{
					if( curr_v == p.v2 )
					{
						v_out.push_back( p.v1 );
						curr_v   = p.v1;
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

	v_out.pop_back();              // remove last one, so the first/last does not appear twice
	return v_out;
}

//-------------------------------------------------------------------------------------------
/// Converts cycle expressed as a vector of vertices to a cycle expressed as a set of pairs.
/// (VPV:Vector of Pairs of Vertices). See \ref p_data_representation
/**
\sa convertVPV2Cycle()
*/
template<typename vertex_t>
std::vector<VertexPair<vertex_t>>
convertCycle2VPV( const std::vector<vertex_t>& cycle )
{
	PRINT_FUNCTION;

	assert( cycle.size()>2 );
	std::vector<VertexPair<vertex_t>> out;
	for( size_t i=0; i<cycle.size(); i++ )
	{
		vertex_t v1 = cycle[i];
		vertex_t v2 = ( i != cycle.size()-1 ? cycle[i+1] : cycle[0] );

		VertexPair<vertex_t> pair(v1,v2);
		out.push_back( pair );
	}
	return out;
}

//-------------------------------------------------------------------------------------------
/// Converts a set of cycles expressed as a vector of vertices to a set of cycles expressed
/// as a set of pairs. (VPV:Vector of Pairs of Vertices). See \ref p_data_representation
/**
\sa convertVPV2Cycle()
\sa convertCycle2VPV()
*/
template<typename vertex_t>
std::vector<std::vector<VertexPair<vertex_t>>>
convertCycles2VVPV( const std::vector<std::vector<vertex_t>>& cycles )
{
	PRINT_FUNCTION;

	assert( cycles.size()>0 );

	std::vector<std::vector<VertexPair<vertex_t>>> out;
	for( const auto& cycle : cycles )
		out.emplace_back( convertCycle2VPV(cycle) );
	return out;
}

//-------------------------------------------------------------------------------------------
/// Convert, for a given graph, a Binary Cycle (BC) \c v_in to a Vertex Cycle (VC)
/**
Algorithm:
-using rev_map, that gives for a given index in the binary vector the two corresponding vertices

* step 1: build a map, giving for each vertex, the next one

* step 2: parse the map, and add the vertices to the output vector

*/
template<typename vertex_t, typename T>
std::vector<vertex_t>
convertBC2VC(
	const BinaryVec&    v_in,        ///< input binary path
	const RevBinMap<T>& rev_map      ///< required map, has to be build before, see buildReverseBinaryMap()
)
{
	PRINT_FUNCTION;

	assert( v_in.size() == rev_map.size() );

//	printBitVector( std::cout, v_in );

// step 1: build set of pairs from binary vector
	auto v_pvertex = convertBinVec2VPV<vertex_t>( v_in, rev_map );
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
	return convertVPV2Cycle( v_pvertex );
}
//-------------------------------------------------------------------------------------------
/// Gaussian binary elimination
/**
- Input: a binary matrix
- Output: a reduced matrix

Assumes no identical rows
*/
inline
BinaryMatrix
gaussianElim( BinaryMatrix& m_in, size_t& nbIter )
{
	PRINT_FUNCTION;

	size_t col = 0;
	size_t nb_rows = m_in.nbLines();
	size_t nb_cols = m_in.nbCols();
	assert( nb_rows > 1 );

	BinaryMatrix m_out;

	nbIter = 0;
	bool done = false;

/// \todo 20200422: Any reason not to use a BinaryVec here ?
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
					for( size_t i=row+1; i<nb_rows; i++ )      // search for all following rows that have a 1 in that column
					{
						if( tag[i] == false )                  // AND that are not tagged.
							if( m_in.line(i)[col] == 1 )            // it there is, we XOR them with initial line
								m_in.line(i) = m_in.line(i) ^ m_in.line(row);
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
	}
	while( !done );
	return m_out;
}

//######################
namespace deprec {
//######################

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

	auto rev_map = buildReverseBinaryMap<size_t>( nbVertices );
	COUT << "revmap size=" << rev_map.size() << '\n';

	for( auto bcycle: binmat )
		if( bcycle.count() )
		{
			auto cycle = convertBC2VC_v2<vertex_t>( bcycle, rev_map, nec );
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

//######################
} // namespace deprec
//######################

//-------------------------------------------------------------------------------------------
/// Convert vector of cycles expressed as binary vectors to vector of cycles expressed as a vector of vertices
template<typename vertex_t>
std::vector<std::vector<vertex_t>>
convertBinary2Vertex
(
	const BinaryMatrix&        binmat,     ///< input binary matrix
	const RevBinMap<vertex_t>& incidMap    ///< incidence map
)
{
	PRINT_FUNCTION;

	std::vector<std::vector<vertex_t>> out;
	for( const auto& li: binmat )
		if( li.count() )     // if line holds some '1'
			out.push_back( convertBC2VC<vertex_t>( li, incidMap ) );
	return out;
}

//-------------------------------------------------------------------------------------------
/// Builds and returns the binary incidence vector associated to \c cycle , given the index \c incidMap.
/**
Here, the length of the vector is equal to \f$ e \f$, number of edges.

Reverse operation done with convertIVtoCycle()

See page \ref p_data_representation.
*/
template<typename vertex_t>
priv::BinaryVec
buildIncidenceVector( const std::vector<vertex_t>& cycle, const RevBinMap<vertex_t>& incidMap )
{
	PRINT_FUNCTION;

	priv::BinaryVec out( incidMap.size() );

	for( size_t i=0; i<cycle.size(); ++i )
	{
		auto v1 = cycle[i];
		auto v2 = ( i==0 ? cycle[cycle.size()-1] : cycle[i-1] );
		VertexPair<vertex_t> vp( v1, v2 );

		auto it = std::find( incidMap.begin(), incidMap.end(), vp );
		assert( it != incidMap.end() ); // should not happen !
		out[ it - incidMap.begin() ] = 1;
	}
	return out;
}

//-------------------------------------------------------------------------------------------
int
dotProduct( const BinaryVec& v1, const BinaryVec& v2 )
{
	size_t countOnes = 0;
	assert( v1.size()  == v2.size() );
	for( size_t i=0; i<v1.size(); i++ )
		if( v1[i] == 1 && v2[i] == 1 )                // if both 1, we have a 1 (AND)
			countOnes++;
	return countOnes%2;           // if odd number of ones, then output is 1 (XOR)
}
//-------------------------------------------------------------------------------------------
/// Returns the total size of cycles and the mean number of nodes that the set of cycles has.
template<typename vertex_t>
std::pair<size_t,double>
getSizeInfo( const std::vector<std::vector<vertex_t>>& cycles )
{
	size_t sum=0;
	std::for_each(
		cycles.begin(),
		cycles.end(),
			[ &sum ]
			(const std::vector<vertex_t>& cycle)
			{ sum += cycle.size(); }
		);
//	std::cout << "CYCLE MEAN SIZE="	<< 1.0 * sum / cycles.size()	<< "\n";
	return std::make_pair( sum, 1.0*sum/cycles.size() );
}
//-------------------------------------------------------------------------------------------
template<typename vertex_t>
void
printStatus( std::ostream& f, const std::vector<std::vector<vertex_t>>& cycles, int line=0 )
{
	f << "l." << ( line ? std::to_string(line) : std::string("???") )
		<< ": status: #=" << cycles.size()
		<< ", total size=" << getSizeInfo( cycles ).first
		<< ", mean size="  << getSizeInfo( cycles ).second
		<< "\n";
	printPaths( f, cycles );
}

//-------------------------------------------------------------------------------------------
/// Builds the reference incidence map
/**
This ones differs from buildReverseBinaryMap() in the sense that it only builds the
map (a vector actually) for edges that are present in the graph, whilst the other one
builds it for ALL possible edges.

Once build, we can convert a cycle expressed as a set of vertices to the corresponding binary form with
buildIncidenceVector()

\todo 20200419: Try to evaluate the difference in case of large, sparse graphs
*/
template<typename vertex_t, typename graph_t>
RevBinMap<vertex_t>
buildTrueIncidMap( const graph_t& gr )
{
	PRINT_FUNCTION;
	RevBinMap<vertex_t> out;

	size_t i=0;
	for(                              // enumerate all edges
		auto p_it = boost::edges(gr);
		p_it.first != p_it.second;
		p_it.first++, i++
	)
	{
		auto v1 = boost::source( *p_it.first, gr );
		auto v2 = boost::target( *p_it.first, gr );
		auto vp = VertexPair<vertex_t>(v1,v2);
		if( std::find( out.begin(), out.end(), vp ) == out.end() )
			out.push_back( vp );
	}
	std::cout << __FUNCTION__ << "() map size=" << out.size() << "\n";
	return out;
}
//-------------------------------------------------------------------------------------------
/// Builds all the binary vectors for all the cycles, using only the existing edges in the graph
/**
\todo Some optimization needed here: the matrix is first built, then we copy another value on each line.
Find a way to build each line in-place.
*/
template<typename vertex_t>
BinaryMatrix
buildBinaryMatrix2(
	const std::vector<std::vector<vertex_t>>& v_cycles,    ///< input cycles
	const RevBinMap<vertex_t>&                incidMap     ///< Incidence map
)
{
	PRINT_FUNCTION;

	BinaryMatrix out( v_cycles.size(), incidMap.size() );  // lines x cols

	for( size_t i=0; i<v_cycles.size(); i++ )
		out.line(i) = buildIncidenceVector( v_cycles[i], incidMap );

	return out;
}

//######################
namespace deprec {
//######################

//-------------------------------------------------------------------------------------------
/// Post-process step: 2005 Melhorn and Dimitrios Michail, page 3 (UNFINISHED !!!)
/**
- assumes set is sorted (shortest first)
- arg is not const, because it gets sorted here.
*/
template<typename vertex_t, typename graph_t>
std::vector<std::vector<vertex_t>>
removeRedundant3( std::vector<std::vector<vertex_t>>& v_in, const graph_t& gr )
{
	PRINT_FUNCTION;

	assert( !v_in.empty() );
	if( v_in.size() < 2 )
		return v_in;

	RevBinMap<vertex_t> incidMap = buildTrueIncidMap<vertex_t,graph_t>( gr );

	size_t idx = 0;
	for( const auto& p : incidMap )
		std::cout << idx++ << ": " << p << "\n";

	std::vector<std::vector<vertex_t>> out;

	auto N = boost::num_edges(gr) - boost::num_vertices(gr) + 1; /// \todo: correct this; only valid if 1 connected graph !!!
	BinaryMatrix mat_S( N, N );

	mat_S.setDiag();

	for( size_t i=0; i<N; i++ )
	{
		auto Si = mat_S.line(i);

// step 1: find shorted cycle s.t <C i , S_i> = 1
		size_t min_value = 11111; // std::max_element();
		size_t min_value_idx=0;
		BinaryVec C;
		for( size_t j=0; j<v_in.size(); j++ )
		{
			C = buildIncidenceVector( v_in[j], incidMap );
			if( dotProduct( C, mat_S.line(i) ) == 1 )
			{
				std::cout << "Found <Si,Ci>=1 for j=" << j << "\n";
				if( v_in[j].size() < min_value )
				{
					min_value = v_in[j].size();
					min_value_idx = i;
				}
			}
		}
		std::cout << "minimal cycle at pos " << min_value_idx;
		auto Ci = buildIncidenceVector( v_in[min_value_idx], incidMap );
		std::cout << ": "; printBitVector( std::cout, Ci );
// step 2
		for( size_t j=i+1; j<N; j++ )
		{
			auto Sj = mat_S.line(j);
			if( dotProduct( Ci, Sj ) == 1 )
				mat_S.line(j) = Si ^ Sj;
		}

	}

// convert to cycles

}
//-------------------------------------------------------------------------------------------
/// Post-process step: taken from Almadi slides (UNFINISHED !!!)
/**
- assumes set is sorted (shortest first)
- arg is not const, because it gets sorted here.
*/
template<typename vertex_t, typename graph_t>
std::vector<std::vector<vertex_t>>
removeRedundant2( std::vector<std::vector<vertex_t>>& v_in, const graph_t& gr )
{
	std::cout << __FUNCTION__ << "() START : " << v_in.size() << " cycles\n";

	assert( !v_in.empty() );
	if( v_in.size() < 2 )
		return v_in;

	RevBinMap<vertex_t> incidMap = buildTrueIncidMap<vertex_t,graph_t>( gr );

	size_t idx = 0;
	for( const auto& p : incidMap )
		std::cout << idx++ << ": " << p << "\n";

	std::vector<std::vector<vertex_t>> out;
//	out.push_back( v_in.front() );
	std::cout << "-input 0: "; printVector( std::cout, v_in.front()  );

	BinaryMatrix mat;
	mat.addLine( buildIncidenceVector( v_in[0], incidMap ) );

	for( size_t i=1; i<v_in.size(); i++ )
	{
		auto v = buildIncidenceVector( v_in[i], incidMap );
		std::cout << "-input " << i << ": "; printVector( std::cout, v_in[i] ); printBitVector( std::cout, v );

		bool all_indep = true;
		for( size_t j=0; j<mat.nbLines(); j++ )
		{
			if (i != j)
			{
				auto v2 = mat.line( j );
				std::cout << " j=" << j << ": checking with "; printBitVector( std::cout, v2 );
				if( dotProduct( v, v2 ) == 1 )
				{
					std::cout << " -NOT independent\n";
					all_indep = false;
				}
				else
				{
					std::cout << " -independent\n";
				}
			}
		}
		if( all_indep )
		{
			mat.addLine( v );
			mat.printMat( std::cout );
		}
	}
	std::cout << "Nb LInes of bin mat=" << mat.nbLines() << "\n";
	for( size_t j=0; j<mat.nbLines(); j++ )
	{
		auto cy = convertBC2VC<vertex_t>( mat.line(j), incidMap );
		out.push_back( cy );
	}

	return out;
}

//######################
} // namespace deprec
//######################

//-------------------------------------------------------------------------------------------
/// Post-process step: removes cycles based on Gaussian Elimination
/**
arg is not const, because it gets sorted here.
*/
template<typename vertex_t, typename graph_t>
std::vector<std::vector<vertex_t>>
removeRedundant(
	std::vector<std::vector<vertex_t>>& v_in,
	const graph_t& gr
)
{
	PRINT_FUNCTION;
	std::cout << __FUNCTION__ << "() START : " << v_in.size() << " cycles\n";
	priv::printStatus( std::cout, v_in, __LINE__ );
// IMPORTANT: the code below assumes we have at least 3 cycles, so lets exit right away if not !
	if( v_in.size() < 3 )
		return v_in;

// build for each cycle its associated binary vector
	priv::RevBinMap<vertex_t> incidMap = buildTrueIncidMap<vertex_t,graph_t>( gr );
	auto binMat_in = buildBinaryMatrix2( v_in, incidMap );

	binMat_in.getInfo().print( std::cout );//, "removeRedundant(): input binary matrix" );
	BinaryMatrix* p_binmat_in = &binMat_in;

	p_binmat_in->printMat( std::cout, "binMat_in" );

	BinaryMatrix* p_binmat = nullptr;

#ifndef UDGCD_USE_M4RI
	size_t nbIter1 = 0;
	auto binMat_out = gaussianElim( *p_binmat_in, nbIter1 );
	COUT << "gaussianElim: nbIter=" << nbIter1 << '\n';
	p_binmat = &binMat_out;
#else
	MatM4ri m4rmiA1 = convertToM4ri( *p_binmat_in );
	MatM4ri m4rmiA0 = m4rmiA1;

//	mzd_echelonize_naive( m4rmiA1._data, 1 );
//	mzd_echelonize_naive( m4rmiA0._data, 0 );
	mzd_echelonize_pluq(  m4rmiA1._data, 1 );
	mzd_echelonize_pluq(  m4rmiA0._data, 0 );

	convertFromM4ri( m4rmiA0 ).printMat( std::cout, "A0" );
	convertFromM4ri( m4rmiA1 ).printMat( std::cout, "A1" );

	auto binMat_out_0 = convertFromM4ri( m4rmiA0 );
	auto binMat_out_1 = convertFromM4ri( m4rmiA1 );

	printStatus( std::cout, convertBinary2Vertex( binMat_out_0, incidMap ) );
	printStatus( std::cout, convertBinary2Vertex( binMat_out_1, incidMap ) );

	p_binmat = &binMat_out_0;
	if( binMat_out_1.count() < binMat_out_0.count() )
		p_binmat = &binMat_out_1;
#endif

	p_binmat->printMat( std::cout, "binMat_out" );

#ifdef UDGCD_NORMALIZE_CYCLES
	auto out = convertBinary2Vertex( *p_binmat, incidMap );
	normalizeCycles( out );
	return out;
#else
	return convertBinary2Vertex( *p_binmat, incidMap );
#endif
}

//-------------------------------------------------------------------------------------------
/// Recursive function, will iterate in graph and return true if cycle is correct. Called by isACycle()
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
/// Returns true if \c cycle is correct, entry point to recursive function checkNextNode()
/**
This function is only there for checking purposes
*/
template<typename vertex_t, typename graph_t>
bool
isACycle( const std::vector<vertex_t>& cycle, const graph_t& gr )
{
//	PRINT_FUNCTION;
	if( cycle.size() > boost::num_vertices(gr) )
		return false;

	return checkNextNode( cycle, 0, gr );
}

//-------------------------------------------------------------------------------------------
/// Checks the cycles in \c V_in and returns as a pair of values:
/**
- first: the number of cycles in \c v_in that are NOT cycles
- second: the number of cycles that are NOT chordless

This function is only there for checking purposes.
It does NOT check that the number of cycles is the correct one.
*/
template<typename vertex_t, typename graph_t>
std::pair<size_t,size_t>
checkCycles( const std::vector<std::vector<vertex_t>>& v_in, const graph_t& gr )
{
	PRINT_FUNCTION;

	size_t c1 = 0;
	size_t c2 = 0;
	for( auto cycle: v_in )
	{
		assert( cycle.size() );

		bool b = isACycle( cycle, gr );
		if( !b )
		{
//			std::cout << __FUNCTION__ << "(): Error, computed cycle not a cycle:\n"; printVector( std::cout, cycle );
			c1++;
		}

		bool b2 = chords::isChordless( cycle, gr );
		if( !b2 )
		{
//			std::cout << __FUNCTION__ << "(): Error, computed cycle not chordless:\n"; printVector( std::cout, cycle );
			c2++;
		}
	}
	return std::make_pair(c1,c2);
}

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
findCycles( graph_t& gr, UdgcdInfo& info )
{
	PRINT_FUNCTION;

	info.startTiming();

	if( boost::num_vertices(gr) < 3 || boost::num_edges(gr) < 3 )
		return std::vector<std::vector<vertex_t>>();

	CycleDetector<vertex_t> cycleDetector;

// vertex color map
	std::vector<boost::default_color_type> vertex_color( boost::num_vertices(gr) );
	auto idmap = boost::get( boost::vertex_index, gr );
	auto vcmap = make_iterator_property_map( vertex_color.begin(), idmap );

// edge color map
	std::map<typename graph_t::edge_descriptor, boost::default_color_type> edge_color;
	auto ecmap = boost::make_assoc_property_map( edge_color );

//////////////////////////////////////
// step 1: do a DFS
//////////////////////////////////////
	boost::undirected_dfs( gr, cycleDetector, vcmap, ecmap, 0 );

	info.setTimeStamp();

	if( !cycleDetector.cycleDetected() )             // if no detection,
		return std::vector<std::vector<vertex_t>>(); //  return empty vector, no cycles found

	std::cout << "cycleDetector: nbSourceVertices=" << cycleDetector.v_source_vertex.size() << '\n';
	std::vector<std::vector<vertex_t>> v_cycles;     // else, get the cycles.

//////////////////////////////////////
// step 2: search paths only starting from vertices that were registered as source vertex
//////////////////////////////////////
	for( const auto& vi: cycleDetector.v_source_vertex )
	{
		COUT << "\n * Start exploring from source vertex " << vi << "\n";
		std::vector<std::vector<vertex_t>> v_paths;
		std::vector<vertex_t> newv(1, vi ); // start by one of the filed source vertex
		v_paths.push_back( newv );
		priv::explore( vi, gr, v_paths, v_cycles );    // call of recursive function
	}

	info.setTimeStamp();
	info.nbRawCycles = v_cycles.size();
	std::cout << "-Nb initial cycles: " << info.nbRawCycles << '\n';

//////////////////////////////////////
// step 3 (post process): cleanout the cycles by removing steps that are not part of the cycle
//////////////////////////////////////

// post process 0: cleanout the cycles by removing steps that are not part of the cycle
	auto v_cycles0 = priv::cleanCycles( v_cycles );
	priv::printStatus( std::cout, v_cycles0, __LINE__ );

	info.setTimeStamp();
	info.nbCleanedCycles = v_cycles0.size();
	std::cout << "-Nb cleaned cycles: " << info.nbCleanedCycles << '\n';

	std::vector<std::vector<vertex_t>>* p_cycles = &v_cycles0;

// SORTING
	std::sort(
		std::begin(*p_cycles),
		std::end(*p_cycles),
		[]                                        // lambda
		( const std::vector<vertex_t> &a, const std::vector<vertex_t> &b )
		{
			return a.size()<b.size();
		}
	);
	priv::printStatus( std::cout, *p_cycles, __LINE__ );



//////////////////////////////////////
// step 4 (post process): remove redundant cycles using Gaussian Elimination
//////////////////////////////////////

#if 0
	auto v_cycles2 = priv::removeRedundant3( *p_cycles, gr );
#else
	auto v_cycles2 = priv::removeRedundant( *p_cycles, gr );
#endif
	p_cycles = &v_cycles2;

#ifdef UDGCD_DO_CYCLE_CHECKING
	if( 0 != priv::checkCycles( *p_cycles, gr ).first )
	{
		std::cerr << "udgcd: ERROR: INVALID CYCLE DETECTED, line " << __LINE__ << "\n";
//		exit(1);
	}
#endif
	priv::printStatus( std::cout, *p_cycles, __LINE__ );


	info.setTimeStamp();
	info.nbFinalCycles = p_cycles->size();
	return *p_cycles;
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

#endif // HG_UDGCD_HPP

/**
\page p_sample_graphs Samples (in test)

<table>
<tr>
<th>graph_0</th>
<th>graph_1</th>
<th>graph_2</th>
</tr>
<tr>
<td>
\dotfile graph_0_0.dot
</td>
<td>
\dotfile graph_1_0.dot
</td>
<td>
\dotfile graph_2_0.dot
</td>
</tr>
</table>
*/
