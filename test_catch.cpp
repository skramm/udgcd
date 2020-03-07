/**
\file test_catch.cpp
\brief test file, needs the Catch framework

https://github.com/philsquared/Catch/
*/


#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "udgcd.hpp"


//-------------------------------------------------------------------------------------------
/// Process test: converts input cycle (expressed as a vector of vertices) into
/// a binary vector, converts it back, and checks that it is the same as input cycme
void
ProcessTest( std::vector<size_t>& cycle, size_t nbVertices )
{
	auto bim = udgcd::priv::BuildBinaryIndexMap( nbVertices );
	REQUIRE( bim.size() == nbVertices-1 );

	auto nbCombinations = nbVertices * (nbVertices-1) / 2;
	udgcd::priv::BinaryPath bpa( nbCombinations );
	udgcd::priv::BuildBinaryVector( cycle, bpa, bim );

//	std::cout << "binary vector:"
//	udgcd::priv::PrintBitVector( std::cout, bpa );

	auto rev_map = udgcd::priv::BuildReverseBinaryMap( nbVertices );
	REQUIRE( rev_map.size() == nbCombinations );

	auto cycle2 = udgcd::priv::ConvertBC2VC<size_t>( bpa, nbVertices, rev_map );
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
	{
		std::vector<std::string> m1{
			"1100",
			"1101",
			"1001",
			"1111"
		};
		std::vector<udgcd::priv::BinaryPath> m_in= buildBinMat( m1 );

		udgcd::priv::printBitMatrix( std::cout, m_in, "m_in" );
		auto out = udgcd::priv::gaussianElim( m_in );
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
		auto out = udgcd::priv::gaussianElim( m_in );
		udgcd::priv::printBitMatrix( std::cout, out, "out" );

	}
}

//-------------------------------------------------------------------------------------------
