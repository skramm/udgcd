
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
TEST_CASE( "test 2", "[test2]" )
{
//	int nbVertices = 4;
//	auto nbCombinations = nbVertices * (nbVertices-1) / 2;

    std::vector<udgcd::priv::BinaryPath> m_in;
	{
		udgcd::priv::BinaryPath bp(std::string("1100"));
		m_in.push_back(bp);
	}
	{
		udgcd::priv::BinaryPath bp(std::string("1101"));
		m_in.push_back(bp);
	}
	{
		udgcd::priv::BinaryPath bp(std::string("1001"));
		m_in.push_back(bp);
	}
	{
		udgcd::priv::BinaryPath bp(std::string("1111"));
		m_in.push_back(bp);
	}

	udgcd::priv::printBitMatrix( std::cout, m_in, "m_in" );


	auto out = udgcd::priv::gaussianElim( m_in );
	udgcd::priv::printBitMatrix( std::cout, out, "out" );

}

//-------------------------------------------------------------------------------------------
