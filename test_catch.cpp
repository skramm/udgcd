
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
ProcessTest( std::vector<size_t>& cycle , size_t nbVertices )
{
	auto bim = udgcd::priv::BuildBinaryIndexMap( nbVertices );
	REQUIRE( bim.size() == nbVertices-1 );

	size_t nbCombinations = nbVertices * (nbVertices-1) / 2;
	udgcd::priv::BinaryPath bc( nbCombinations );
	udgcd::priv::BuildBinaryVector( cycle, bc, bim );

	auto rev_map = udgcd::priv::BuildReverseBinaryMap( nbVertices );
	REQUIRE( rev_map.size() == nbCombinations );

	auto cycle2 = udgcd::priv::ConvertBC2VC<size_t>( bc, nbVertices, rev_map );
	REQUIRE( cycle == cycle2 );

}

//-------------------------------------------------------------------------------------------
TEST_CASE( "test 1", "[test1]" )
{
	{
		std::vector<size_t> cycle{1,2,3,4};
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
