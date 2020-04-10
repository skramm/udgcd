
/**
\file
\brief test of m4ri
*/

#include <iostream>
#include "wrapper_m4ri.hpp"



//-------------------------------------------------------------------
int main(int, char*[])
{
//	SHOW_INFO;

	MatM4ri m(10,20);
	m.randomize();
	MatM4ri m2(10,20);
	m2 = m;
	std::cout << m;

	std::cout << "mzd_echelonize_naive( m._data, 0 );\n";
	mzd_echelonize_naive( m._data, 0 );
	std::cout << m;
	m = m2;

	std::cout << "mzd_echelonize_naive( m._data, 1 );\n";
	mzd_echelonize_naive( m._data, 1 );
	std::cout << m;

}

//-------------------------------------------------------------------

