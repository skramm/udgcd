/**
\file test_catch.cpp
\brief test file, needs the Catch framework

https://github.com/philsquared/Catch/
*/


#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "udgcd.hpp"


//-------------------------------------------------------------------------------------------
TEST_CASE( "test buildBinaryIndexVec", "[testbiv]" )
{
	size_t nbVertices = 6;
	{
		std::vector<size_t> expected = {0,4,7,9,10};
		std::vector<size_t> idx_vec = udgcd::priv::buildBinaryIndexVec( nbVertices );
		REQUIRE( idx_vec == expected );
	}
	{
		nbVertices = 7;
		std::vector<size_t> expected = {0,5,9,12,14,15};
		std::vector<size_t> idx_vec = udgcd::priv::buildBinaryIndexVec( nbVertices );
		REQUIRE( idx_vec == expected );
	}
}

//-------------------------------------------------------------------------------------------
/// Process test: converts input cycle (expressed as a vector of vertices) into
/// a binary vector, converts it back, and checks that it is the same as input cycme
void
ProcessTest( std::vector<size_t>& cycle, size_t nbVertices )
{
	auto bim = udgcd::priv::buildBinaryIndexVec( nbVertices );
	REQUIRE( bim.size() == nbVertices-1 );

	auto nbCombinations = nbVertices * (nbVertices-1) / 2;
	udgcd::priv::BinaryPath bpa( nbCombinations );
	udgcd::priv::buildBinaryVector( cycle, bpa, bim );

//	std::cout << "binary vector:"
//	udgcd::priv::PrintBitVector( std::cout, bpa );

	auto rev_map = udgcd::priv::buildReverseBinaryMap( nbVertices );
	REQUIRE( rev_map.size() == nbCombinations );

	size_t iter;
	auto cycle2 = udgcd::priv::convertBC2VC<size_t>( bpa, rev_map, iter );
	REQUIRE( cycle == cycle2 );

}

//-------------------------------------------------------------------------------------------
TEST_CASE( "test isACycle", "[test4]" )
{
	typedef boost::adjacency_list<
		boost::vecS,
		boost::vecS,
		boost::undirectedS
	> graph_t;

	{
		graph_t g( 5 );

		add_edge(0, 1, g);   //   0--1---2
		add_edge(1, 2, g);   //       \  |
		add_edge(2, 3, g);   //        \ |
		add_edge(3, 4, g);   //          3--4
		add_edge(3, 1, g);   //

		{
			std::vector<size_t> in1{1,2,3};
			REQUIRE( udgcd::priv::isACycle( in1, g ) );
			std::vector<size_t> in2{2,3,1};
			REQUIRE( udgcd::priv::isACycle( in2, g ) );
			std::vector<size_t> in3{3,1,2};
			REQUIRE( udgcd::priv::isACycle( in3, g ) );
		}
		{
			std::vector<size_t> in{2,3,4};
			REQUIRE( !udgcd::priv::isACycle( in, g ) );
		}
		{
			std::vector<size_t> in{2,4,0};
			REQUIRE( !udgcd::priv::isACycle( in, g ) );
		}
		{
			std::vector<size_t> in{1,2,3,4};
			REQUIRE( !udgcd::priv::isACycle( in, g ) );
		}
		{
			std::vector<size_t> in{0,1,2,3,4};
			REQUIRE( !udgcd::priv::isACycle( in, g ) );
		}
	}
	{
		graph_t g( 5 );

		add_edge(0, 1, g);   //   0--1---2
		add_edge(1, 2, g);   //      |   |
		add_edge(2, 3, g);   //      |   |
		add_edge(3, 4, g);   //      4---3
		add_edge(4, 1, g);   //

		{
			std::vector<size_t> in1{1,2,3};
			REQUIRE( !udgcd::priv::isACycle( in1, g ) );
			std::vector<size_t> in2{2,3,1};
			REQUIRE( !udgcd::priv::isACycle( in2, g ) );
			std::vector<size_t> in3{3,1,2};
			REQUIRE( !udgcd::priv::isACycle( in3, g ) );
		}
		{
			std::vector<size_t> in{2,3,4};
			REQUIRE( !udgcd::priv::isACycle( in, g ) );
		}
		{
			std::vector<size_t> in{2,4,0};
			REQUIRE( !udgcd::priv::isACycle( in, g ) );
		}
		{
			std::vector<size_t> in{1,2,3,4};
			REQUIRE( udgcd::priv::isACycle( in, g ) );
		}
		{
			std::vector<size_t> in{0,1,2,3,4};
			REQUIRE( !udgcd::priv::isACycle( in, g ) );
		}
	}

}
//-------------------------------------------------------------------------------------------
TEST_CASE( "test clean cycle", "[test3]" )
{
	{
		std::vector<size_t> in{1,2,3,1};
		std::vector<size_t> out{1,2,3};
		auto newcy = udgcd::priv::findTrueCycle( in );
		REQUIRE( newcy == out );
	}
	{
		std::vector<size_t> in{1,2,3,1,2,3};
		std::vector<size_t> out{1,2,3};
		auto newcy = udgcd::priv::findTrueCycle( in );
		REQUIRE( newcy == out );
	}
	{
		std::vector<size_t> in{1,2,3,1,5,6,7};
		std::vector<size_t> out{1,2,3};
		auto newcy = udgcd::priv::findTrueCycle( in );
		REQUIRE( newcy == out );
	}
	{
		std::vector<size_t> in{6,7,8,9,1,6,0,2};
		std::vector<size_t> out{1,6,7,8,9};
		auto newcy = udgcd::priv::findTrueCycle( in );
		REQUIRE( newcy == out );
	}
}

//-------------------------------------------------------------------------------------------
TEST_CASE( "test 1", "[test1]" )
{
	{
		std::vector<size_t> cycle{1,2,3,4};

		udgcd::PrintVector( std::cout, cycle ); //, "1: cycle" );

		ProcessTest( cycle, 5 );
		ProcessTest( cycle, 6 );
		ProcessTest( cycle, 10 );
	}
	{
		std::vector<size_t> cycle{1,2,4};
		ProcessTest( cycle, 5 );
		ProcessTest( cycle, 6 );
		ProcessTest( cycle, 10 );
	}
	{
		std::vector<size_t> cycle{1,2,4,8,9};
		ProcessTest( cycle, 10 );
		ProcessTest( cycle, 15 );
		ProcessTest( cycle, 20 );
	}
}
//-------------------------------------------------------------------------------------------
std::vector<udgcd::priv::BinaryPath>
buildBinMat( const std::vector<std::string>& m )
{
	std::vector<udgcd::priv::BinaryPath> out;
	for( auto s: m )
	{
		udgcd::priv::BinaryPath bp(s);
		out.push_back(bp);
	}
	return out;
}
//-------------------------------------------------------------------------------------------
TEST_CASE( "test GaussianElimination", "[test2]" )
{
	size_t iter;
	{
		std::vector<std::string> m1{
			"1100",
			"1101",
			"1001",
			"1111"
		};
		std::vector<udgcd::priv::BinaryPath> m_in= buildBinMat( m1 );

		udgcd::priv::printBitMatrix( std::cout, m_in, "m_in" );
		auto out = udgcd::priv::gaussianElim( m_in, iter );
		udgcd::priv::printBitMatrix( std::cout, out, "out" );

	}
	{
		std::vector<std::string> m1{
			"1100",
			"0110",
			"1010",
			"0101",
			"0011",
			"1001"
		};
		std::vector<udgcd::priv::BinaryPath> m_in= buildBinMat( m1 );

		udgcd::priv::printBitMatrix( std::cout, m_in, "m_in" );
		auto out = udgcd::priv::gaussianElim( m_in, iter );
		udgcd::priv::printBitMatrix( std::cout, out, "out" );
	}
}

//-------------------------------------------------------------------------------------------
udgcd::priv::BinaryPath
buildBinVect( const char* s )
{
	std::string str(s);
	std::reverse( str.begin(), str.end() );
	udgcd::priv::BinaryPath bp{str};
	return bp;
}
//-------------------------------------------------------------------------------------------
#if 0
TEST_CASE( "test buildPairSetFromBinaryVec", "[test5]" )
{
/*
graph: 0-1-4-3:

idx:    0  1  2  3  4  5  6  7  8  9
----------------------------------------
edges:  0  0  0  0  1  1  1  2  2  3
        1  2  3  4  2  3  4  3  4  4
----------------------------------------
vector: 1  0  1  0  0  0  1  0  0  1
*/
	size_t     nb_vertices = 5;
	udgcd::priv::BinaryPath v_in{ buildBinVect("1010001001") };
	udgcd::priv::RevBinMap  rev_map = udgcd::priv::buildReverseBinaryMap( nb_vertices );

	auto m = udgcd::priv::buildPairSetFromBinaryVec<size_t>( v_in, rev_map );
	std::map<size_t,size_t> res={
		{0,1},{3,0},{1,4},{4,3}
	};
/*	std::cout << "Expected map:\n";
	for( auto p: res )
		std::cout << "f=" << p.first << " s=" << p.second << '\n';

	std::cout << "computed map:\n";
	for( auto p: m )
		std::cout << "f=" << p.first << " s=" << p.second << '\n';
*/
	REQUIRE( res == m );
}
#endif
//-------------------------------------------------------------------------------------------
