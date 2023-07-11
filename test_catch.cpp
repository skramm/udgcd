/**
\file test_catch.cpp
\brief test file, needs the Catch framework

https://github.com/philsquared/Catch/
*/

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#define UDGCD_LOG_FUNC
#include "udgcd.hpp"
#include "demo/common_sample.h"

using namespace udgcd;
using namespace sample;

using graph_t = boost::adjacency_list<
		boost::vecS,
		boost::vecS,
		boost::undirectedS
	>;

//-------------------------------------------------------------------------------------------
TEST_CASE( "Conversions", "[tc]" )
{
	std::cout << "Running tests with catch " << CATCH_VERSION_MAJOR << '.' << CATCH_VERSION_MINOR << '.' << CATCH_VERSION_PATCH << '\n';

	std::vector<size_t> v1{ 1,4,9,12 };
	std::vector<priv::VertexPair<size_t>> v2{ {1,4},{4,9},{9,12},{12,1} };
	auto res1 = priv::convertCycle2VPV( v1 );

	CHECK( res1 == v2 );
	auto res2 = priv::convertVPV2Cycle( res1 );
	CHECK( res2 == v1 );
}


//-------------------------------------------------------------------------------------------
TEST_CASE( "Chordless", "[t-chordless]" )
{
	std::array<graph_t,14> gg;
	graph_t g;
	boost::add_edge( 0, 1, g );
	boost::add_edge( 1, 2, g ) ;

	{
		gg[0] = g;                            //   0---1
		boost::add_edge( 2, 0, gg[0] );       //    \  |
		;                                     //     \ |
		;                                     //       2

		gg[1] = gg[0];                         //   0---1
		boost::add_edge( 0, 3, gg[1] );        //   |\  |
		boost::add_edge( 2, 3, gg[1] );        //   | \ |
		;                                      //   3---2
	}
	{
		gg[2] = g;                             //   0---1
		boost::add_edge( 2, 3, gg[2] );        //   |   |
		boost::add_edge( 0, 3, gg[2] );        //   |   |
		;                                      //   3---2
	}
	{
		gg[3] = g;                             //   0---1
		boost::add_edge( 1, 3, gg[3] );        //   | / |
		boost::add_edge( 2, 3, gg[3] );        //   |/  |
		boost::add_edge( 0, 3, gg[3] );        //   3---2
	}
	{
		gg[4] = g;                             //   0--1--2---3
		boost::add_edge( 2, 3, gg[4] );        //   |    /|
		boost::add_edge( 2, 4, gg[4] );        //   |   / |
		boost::add_edge( 4, 5, gg[4] );        //   |  /  |
		boost::add_edge( 0, 5, gg[4] );        //   | /   |
		gg[5] = gg[4];                         //   5-----4
		boost::add_edge( 5, 2, gg[5] );
	}
	{
		gg[6] = g;                             //   0--1--2
		boost::add_edge( 2, 3, gg[6] );        //   |    /|
		boost::add_edge( 3, 4, gg[6] );        //   |   / |
		boost::add_edge( 4, 5, gg[6] );        //   |  /  3
		boost::add_edge( 0, 5, gg[6] );        //   | /   |
		gg[7] = gg[6];                         //   5-----4
		boost::add_edge( 5, 2, gg[7] );

	}
	{
		gg[8] = g;                               //   0--1--2
		boost::add_edge( 2, 3, gg[8] );          //   |    /|
		boost::add_edge( 3, 4, gg[8] );          //   |   / |
		boost::add_edge( 4, 5, gg[8] );          //   6  /  3
		boost::add_edge( 5, 6, gg[8] );          //   | /   |
		boost::add_edge( 6, 0, gg[8] );          //   5-----4
		gg[9] = gg[8];
		boost::add_edge( 5, 2, gg[9] );
	}

	{                  // two chords
		gg[10] = g;                            /*     0--1--2      */
		boost::add_edge( 2, 3, gg[10] );       /*    /|     |\     */
		boost::add_edge( 3, 4, gg[10] );       /*   / |     | \    */
		boost::add_edge( 2, 4, gg[10] );       /*  6  |     |  3   */
		boost::add_edge( 4, 5, gg[10] );       /*   \ |     | /    */
		boost::add_edge( 6, 0, gg[10] );       /*    \|     |/     */
		boost::add_edge( 6, 5, gg[10] );       /*     5-----4      */
		boost::add_edge( 5, 0, gg[10] );

		gg[11] = gg[10];                       /*     0--1--2      */
		boost::remove_edge( 5, 0, gg[11] );    /*    /|     |\     */
		boost::add_edge( 7, 5, gg[11] );       /*   / |     | \    */
		boost::add_edge( 7, 5, gg[11] );       /*  6  7     |  3   */
		;                                      /*   \ |     | /    */
		;                                      /*    \|     |/     */
		;                                      /*     5-----4      */
	}
	{
		gg[12] = g;                          //     0-------------1
		boost::add_edge( 2, 3, gg[12] );     //     |             |
		boost::add_edge( 3, 4, gg[12] );     //     |           --2
		boost::add_edge( 4, 5, gg[12] );     //     5------\   /  /
		boost::add_edge( 5, 0, gg[12] );     //      \   ---+--  /
		;                                    //       \ /    \  /
		boost::add_edge( 2, 4, gg[12] );     //        4-------3
		boost::add_edge( 3, 5, gg[12] );
	}
	{
		gg[13] = g;                        //  7--0--1--2---3--4---5---6
		boost::add_edge( 2, 3, gg[13] );   //     |  |\    /   |   |
		boost::add_edge( 3, 4, gg[13] );   //     |  | \--/    /   |
		boost::add_edge( 4, 5, gg[13] );   //     |   \-------/    |
		boost::add_edge( 5, 0, gg[13] );   //     \----------------/
		boost::add_edge( 1, 3, gg[13] );
		boost::add_edge( 1, 4, gg[13] );
		boost::add_edge( 5, 6, gg[13] );
		boost::add_edge( 0, 7, gg[13] );
	}
	{
		std::vector<size_t> v1{ 0,1,2 };
		CHECK( priv::chords::isChordless( v1, gg[0] ) );
	}
	{
		std::vector<size_t> v1{ 0,1,2,3 };
		CHECK( !priv::chords::isChordless( v1, gg[1] ) );
		std::vector<size_t> v2{ 0,2,3 };
		CHECK( v2 == priv::chords::removeChords( v1, gg[1] ) );
	}
	{
		std::vector<size_t> v1{ 0,1,2,3 };
		CHECK( priv::chords::isChordless( v1, gg[2] ) );
	}
	{
		std::vector<size_t> v1{ 0,1,2,3 };
		CHECK( !priv::chords::isChordless( v1,gg[3] ) );
		std::vector<size_t> v2{ 0,1,3 };
		CHECK( v2 == priv::chords::removeChords( v1, gg[3] ) );
	}
	{
		std::vector<size_t> v1{ 0,1,2,4,5 };
		CHECK(  priv::chords::isChordless( v1, gg[4] ) );
		CHECK( !priv::chords::isChordless( v1, gg[5] ) );

		std::vector<size_t> v2{ 0,1,2,5 };
		CHECK( v2 == priv::chords::removeChords( v1, gg[5] ) );
	}
	{
		std::vector<size_t> v1{ 0,1,2,3,4,5 };
		CHECK(  priv::chords::isChordless( v1, gg[6] ) );
		CHECK( !priv::chords::isChordless( v1, gg[7] ) );

		std::vector<size_t> v2{ 0,1,2,5 };
		CHECK( v2 == priv::chords::removeChords( v1, gg[7] ) );
	}
	{
		std::vector<size_t> v1{ 0,1,2,3,4,5,6 };
		CHECK(  priv::chords::isChordless( v1, gg[8] ) );
		CHECK( !priv::chords::isChordless( v1, gg[9] ) );

		std::vector<size_t> v2{ 0,1,2,5,6 };
		CHECK( v2 == priv::chords::removeChords( v1, gg[9] ) );
	}
	{
		std::vector<size_t> v1{ 0,1,2,3,4,5,6 };
		CHECK( !priv::chords::isChordless( v1, gg[10] ) );

		std::vector<size_t> v2a{ 0,1,2,4,5 };
		CHECK( priv::chords::isChordless( v2a, gg[10] ) );
		std::vector<size_t> v2b{ 2,3,4 };
		CHECK( priv::chords::isChordless( v2b, gg[10] ) );
		std::vector<size_t> v2c{ 0,5,6 };
		CHECK( priv::chords::isChordless( v2c, gg[10] ) );

		std::vector<size_t> v2{ 0,1,2,5,6 };
		CHECK( v2c == priv::chords::removeChords( v1, gg[10] ) );
	}
	{
		std::vector<size_t> v1{ 0,1,2,4,5,6 };
		CHECK( priv::chords::isChordless( v1, gg[11] ) );
		std::vector<size_t> v2{ 0,1,2,4,5,7 };
		CHECK( priv::chords::isChordless( v2, gg[11] ) );

		std::vector<size_t> v3a{ 0,1,2,3,4,5,7 };
		std::vector<size_t> v3b{ 0,1,2,3,4,5,6 };
		CHECK( !priv::chords::isChordless( v3a, gg[11] ) );
		CHECK( !priv::chords::isChordless( v3b, gg[11] ) );
	}
	{
		std::vector<size_t> v1{ 2,3,4 };
		CHECK( priv::chords::isChordless( v1, gg[12] ) );
		std::vector<size_t> v2{ 3,4,5 };
		CHECK( priv::chords::isChordless( v2, gg[12] ) );
		std::vector<size_t> v3{ 0,1,2,4,5 };
		CHECK( priv::chords::isChordless( v3, gg[12] ) );
		std::vector<size_t> v4{ 0,1,2,3,5 };
		CHECK( priv::chords::isChordless( v4, gg[12] ) );

		std::vector<size_t> v5{ 0,1,2,3,4,5 };
		CHECK( !priv::chords::isChordless( v5, gg[12] ) );
	}
}

//-------------------------------------------------------------------------------------------
#define ARE_CONNECTED( a, b, gr ) priv::areConnected( (size_t)a, (size_t)b, gr )


TEST_CASE( "connected", "[t-conn]" )
{
	graph_t g;

	boost::add_edge(2, 1, g); // a 3 node cycle
	boost::add_edge(2, 3, g);
	boost::add_edge(3, 1, g);

	boost::add_edge(4, 5, g);   // 3 nodes connected but no edge
	boost::add_edge(5, 6, g);
	boost::add_edge(6, 7, g);


	boost::add_edge( 8,  9, g); // 4 nodes with
	boost::add_edge( 9, 10, g);
	boost::add_edge(10, 11, g);
	boost::add_edge(11,  8, g);
	boost::add_edge(10,  8, g);

	CHECK( ARE_CONNECTED(  1, 2, g ) );
	CHECK( ARE_CONNECTED(  2, 1, g ) );

	CHECK( !ARE_CONNECTED( 1, 4, g ) );
	CHECK( !ARE_CONNECTED( 4, 1, g ) );

	CHECK( !ARE_CONNECTED( 7, 8, g ) );

	CHECK( !ARE_CONNECTED( 9, 11, g ) );
	CHECK( !ARE_CONNECTED( 11, 9, g ) );

	CHECK( ARE_CONNECTED( 8, 10, g ) );
	CHECK( ARE_CONNECTED( 10, 8, g ) );
}


//-------------------------------------------------------------------------------------------
TEST_CASE( "test buildFullBinaryIndex", "[testbiv]" )
{
	size_t nbVertices = 6;
	{
		std::vector<size_t> expected = {0,4,7,9,10};
		std::vector<size_t> idx_vec = priv::deprec::buildFullBinaryIndex( nbVertices );
		REQUIRE( idx_vec == expected );
	}
	{
		nbVertices = 7;
		std::vector<size_t> expected = {0,5,9,12,14,15};
		std::vector<size_t> idx_vec = priv::deprec::buildFullBinaryIndex( nbVertices );
		REQUIRE( idx_vec == expected );
	}
}

//-------------------------------------------------------------------------------------------
/// Process test: converts input cycle (expressed as a vector of vertices) into
/// a binary vector, converts it back, and checks that it is the same as input cycle
void
ProcessTest( std::vector<size_t>& cycle, size_t nbVertices )
{
	auto bim = udgcd::priv::deprec::buildFullBinaryIndex( nbVertices );
	REQUIRE( bim.size() == nbVertices-1 );

	auto nbCombinations = nbVertices * (nbVertices-1) / 2;
	udgcd::priv::BinaryVec bpa( nbCombinations );
	udgcd::priv::deprec::buildFullBinaryVector( cycle, bpa, bim );

//	std::cout << "binary vector:"
//	udgcd::priv::PrintBitVector( std::cout, bpa );

	auto rev_map = udgcd::priv::deprec::buildReverseBinaryMap<size_t>( nbVertices );
	REQUIRE( rev_map.size() == nbCombinations );

	auto cycle2 = udgcd::priv::convertBC2VC<size_t>( bpa, rev_map );
	REQUIRE( cycle == cycle2 );
}

//-------------------------------------------------------------------------------------------
TEST_CASE( "test isACycle", "[test4]" )
{
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

		{  // erroneous cycle: node appears multiple times
			std::vector<size_t> in{0,1,2,3,2,3,4};
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

		udgcd::printVector( std::cout, cycle ); //, "1: cycle" );

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
std::vector<udgcd::priv::BinaryVec>
buildBinMat( const std::vector<std::string>& m )
{
	std::vector<udgcd::priv::BinaryVec> out;
	for( auto s: m )
	{
		udgcd::priv::BinaryVec bp(s);
		out.push_back(bp);
	}
	return out;
}
//-------------------------------------------------------------------------------------------
#if 0
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
		std::vector<udgcd::priv::BinaryVec> m_in= buildBinMat( m1 );

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
		std::vector<udgcd::priv::BinaryVec> m_in= buildBinMat( m1 );

		udgcd::priv::printBitMatrix( std::cout, m_in, "m_in" );
		auto out = udgcd::priv::gaussianElim( m_in, iter );
		udgcd::priv::printBitMatrix( std::cout, out, "out" );
	}
}
#endif

//-------------------------------------------------------------------------------------------
udgcd::priv::BinaryVec
buildBinVect( const char* s )
{
	std::string str(s);
	std::reverse( str.begin(), str.end() );
	udgcd::priv::BinaryVec bp{str};
	return bp;
}
//-------------------------------------------------------------------------------------------
TEST_CASE( "test dotproduct", "[tdp]" )
{
	{
		priv::BinaryVec v1{ buildBinVect("00000") };
		priv::BinaryVec v2{ buildBinVect("00000") };
		CHECK( priv::dotProduct( v1, v2 ) == 0 );
	}
	{
		priv::BinaryVec v1{ buildBinVect("11100") };
		priv::BinaryVec v2{ buildBinVect("00000") };
		CHECK( priv::dotProduct( v1, v2 ) == 0 );
	}
	{
		priv::BinaryVec v1{ buildBinVect("11100") };
		priv::BinaryVec v2{ buildBinVect("00011") };
		CHECK( priv::dotProduct( v1, v2 ) == 0 );
	}
	{
		priv::BinaryVec v1{ buildBinVect("11110") };
		priv::BinaryVec v2{ buildBinVect("00011") };
		CHECK( priv::dotProduct( v1, v2 ) == 1 );
	}
	{
		priv::BinaryVec v1{ buildBinVect("11111") };
		priv::BinaryVec v2{ buildBinVect("00011") };
		CHECK( priv::dotProduct( v1, v2 ) == 0 );
	}
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
	priv::BinaryVec v_in{ buildBinVect("1010001001") };
	priv::RevBinMap  rev_map = udgcd::priv::deprec::buildReverseBinaryMap( nb_vertices );

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
TEST_CASE( "splitstring", "[tsplit]" )
{
	{
		std::string a("abc---efg---qqq");
		auto t = splitString( a, "---" );

		REQUIRE( t.size() == 3 );

		CHECK( t[0] == "abc" );
		CHECK( t[1] == "efg" );
		CHECK( t[2] == "qqq" );
	}
	{
		std::string a("abc");
		auto t = splitString( a, "---" );
		REQUIRE( t.size() == 0 );
	}
	{
		std::string a("abc---");
		auto t = splitString( a, "---" );
		REQUIRE( t.size() == 1 );
		CHECK( t[0] == "abc" );
	}

    CHECK( trimString( "abc" )       == "abc" );
	CHECK( trimString( "    abc" )   == "abc" );
	CHECK( trimString( "" )          == "" );
	CHECK( trimString( "   " )       == "" );
    CHECK( trimString( "abc   " )    == "abc" );
    CHECK( trimString( "   abc   " ) == "abc" );

}
//-------------------------------------------------------------------------------------------
TEST_CASE( "tree stripping", "[tree-strip]" )
{
	graph_t gr(8);

	std::vector<std::vector<size_t>> vcy = {
		{1,2,3,1},
		{1,2,3,0,1},
		{0,3,4,1,0},
		{0,3,4,0},
		{0,3,4,0},
	};
	priv::stripCycles( vcy, gr );



}

//-------------------------------------------------------------------------------------------

