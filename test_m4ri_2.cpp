
/**
\file
\brief test of m4ri
*/

#include <iostream>
#include "udgcd.hpp"
#include "wrapper_m4ri.hpp"

using namespace udgcd;

//-------------------------------------------------------------------
int main( int argc, char** argv )
{
	int r = 10;
	int c = 20;
	if( argc == 3 )
	{
		r = std::stoi( argv[1] );
		c = std::stoi( argv[2] );
	}

	MatM4ri m(r,c);
	m.randomize();
	std::cout << " start:\n" << m;

	priv::BinaryMatrix bmat1 = convertFromM4ri( m );
	size_t iter=0;
//	bmat1.print( std::cout, "bmat1" );
	auto bmat2 = priv::gaussianElim( bmat1, iter );
	bmat2.printMat( std::cout, "gaussianElim" );
	MatM4ri mine = convertToM4ri( bmat2 );


	MatM4ri m2(m);

	std::cout << "mzd_echelonize_naive( m._data, 0 );\n";
	mzd_echelonize_naive( m._data, 0 );
	std::cout << m;
	std::cout << ( mzd_equal( mine._data, m._data ) ? " -equal\n" : " -diff\n" );

	m = m2;
	std::cout << "mzd_echelonize_naive( m._data, 1 );\n";
	mzd_echelonize_naive( m._data, 1 );
	std::cout << m;
	std::cout << ( mzd_equal( mine._data, m._data ) ? " -equal\n" : " -diff\n" );
}

//-------------------------------------------------------------------

