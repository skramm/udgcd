/**
\file wrapper_m4ri.hpp
\brief C++ wrapper, see https://bitbucket.org/malb/m4ri/
*/

#include <cassert>
#include <iostream>
#include <utility> // needed for std::move

//#include <m4ri/config.h>
#include <m4ri/m4ri.h>

#include "udgcd.hpp"

/// Wrapper over the m4ri dense matrix datatype
struct MatM4ri
{
	friend std::ostream& operator << ( std::ostream& f, const MatM4ri& m );
//	private:
		mzd_t* _data  = nullptr;
	public:
		MatM4ri( size_t row, size_t col );
		~MatM4ri();

		void set( size_t row, size_t col, int val );
		int get( size_t row, size_t col );
		void randomize()
		{
			mzd_randomize( _data );
		}
		size_t nbRows() const { return  _data->nrows; }
		size_t nbCols() const { return  _data->ncols; }

		MatM4ri& operator = ( const MatM4ri& other )
		{
			if( &other == this ) // check for self-assignment
            	return *this;
			MatM4ri newmat( other.nbRows(), other.nbCols() );
			mzd_copy( newmat._data, _data );
			return *this;
		}

		MatM4ri& operator = ( const MatM4ri&& other ) = delete; // move assignement operator
		MatM4ri( const MatM4ri&& other ) = delete;         // move copy constructor

};

MatM4ri::MatM4ri( size_t row, size_t col )
{
	_data = mzd_init( row, col );
	assert( _data != nullptr );
}

MatM4ri::~MatM4ri()
{
	assert( _data != nullptr );
	mzd_free(_data);
}

void MatM4ri::set( size_t row, size_t col, int val )
{
	assert( _data != nullptr );
	assert( row < nbRows() );
	assert( col < nbCols() );
	mzd_write_bit( _data, row, col, val );
}

int MatM4ri::get( size_t row, size_t col )
{
	assert( _data != nullptr );
	assert( row < nbRows() );
	assert( col < nbCols() );
	return mzd_read_bit( _data, row, col );
}

#if 0
/// UNTESTED !!
MatM4ri convert2M4ri( const udgcd::priv::BinaryMatrix& mat_in )
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
#endif

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

std::ostream&
operator << ( std::ostream& f, const MatM4ri& mat )
{
	f << "MatM4ri: " << mat.nbRows() << " rows x " << mat.nbCols() << " cols\n      ";

	for( size_t col=0; col<mat.nbCols(); col++ )
		f << (col+1)%10 << " ";
	f << "\n    -";
	for( size_t col=0; col<mat.nbCols(); col++ )
		f << "--"	;
	f << "\n";
	for( size_t row=0; row<mat.nbRows(); row++ )
	{
		f << std::setw(3) << row+1 << " | ";
		for( size_t col=0; col<mat.nbCols(); col++ )
			f <<  mzd_read_bit( mat._data, row, col ) << " ";
		f << '\n';
	}
	return f;
}
