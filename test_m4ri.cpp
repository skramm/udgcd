
/**
\file
\brief test of m4ri
*/

#include <iostream>
#include "udgcd.hpp"
//#include "wrapper_m4ri.hpp"
//#include "wrapper_m4ri_convert.hpp"

using namespace udgcd;

//-------------------------------------------------------------------
int main(int, char*[])
{
#if 0
	MatM4ri m(5,10);
	m.randomize();
	MatM4ri m2(m);
	std::cout << m;

	std::cout << "mzd_echelonize_naive( m._data, 0 );\n";
	mzd_echelonize_naive( m._data, 0 );
	std::cout << m;
	m = m2;

	std::cout << "mzd_echelonize_naive( m._data, 1 );\n";
	mzd_echelonize_naive( m._data, 1 );
	std::cout << m;

	priv::BinaryMatrix bmat1 = convertFromM4ri( m2 );
	size_t iter=0;
	bmat1.printMat( std::cout, "bmat1" );
	auto bmat2 = priv::gaussianElim( bmat1, iter );
	bmat2.printMat( std::cout, "bmat2" );

	MatM4ri m3 = convertToM4ri( bmat2 );
	std::cout << "m3:\n" << m3;
#endif
}

//-------------------------------------------------------------------

