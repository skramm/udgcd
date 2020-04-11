/**
\file wrapper_m4ri_convert.hpp
\brief C++ wrapper, see https://bitbucket.org/malb/m4ri/
*/
#ifndef HG_WRAPPER_M4RI_CONVERT_HPP
#define HG_WRAPPER_M4RI_CONVERT_HPP


#include <m4ri/m4ri.h>
#include "udgcd.hpp"


MatM4ri convertToM4ri( const udgcd::priv::BinaryMatrix& mat_in )
{
	MatM4ri out( mat_in.nbLines(), mat_in.nbCols() );
	size_t row = 0;
	for( const auto& line: mat_in )
	{
		for( size_t col=0; col<line.size(); col++ )
			out.set( row, col, line[col] );
		row++;
	}
	return out;
}

/// UNTESTED !!
udgcd::priv::BinaryMatrix
convertFromM4ri( const MatM4ri& mat_in )
{
	udgcd::priv::BinaryMatrix out( mat_in.nbRows(), mat_in.nbCols() );

	for( size_t row=0; row<mat_in.nbRows(); row++ )
	{
		udgcd::priv::BinaryVec& vec = *(out.begin() + row);
		for( size_t col=0; col<out.nbCols(); col++ )
			vec[col] = mzd_read_bit( mat_in._data, row, col );
	}
	return out;
}

#endif // HG_WRAPPER_M4RI_CONVERT_HPP
