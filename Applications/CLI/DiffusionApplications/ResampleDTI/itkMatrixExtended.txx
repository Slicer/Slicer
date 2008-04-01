#ifndef __itkMatrixExtended_txx
#define __itkMatrixExtended_txx

#include "itkMatrixExtended.h"

namespace itk
{

template< class T , unsigned int NRows , unsigned int NColumns >
MatrixExtended< T , NRows , NColumns >
::MatrixExtended():Matrix< T , NRows , NColumns >()
{
}

template< class T , unsigned int NRows , unsigned int NColumns >
MatrixExtended< T , NRows , NColumns >
::MatrixExtended( const Self &matrix )
:Matrix< T , NRows , NColumns >( Superclass( matrix ) )
{
}

template< class T , unsigned int NRows , unsigned int NColumns >
MatrixExtended< T , NRows , NColumns >
::MatrixExtended( const Superclass &matrix ) : Matrix< T , NRows , NColumns >( matrix )
{
}



template< class T , unsigned int NRows , unsigned int NColumns >
MatrixExtended< T , NRows , NColumns >
::MatrixExtended( const InternalMatrixType &matrix )
 : Matrix< T , NRows , NColumns >( matrix )
{
}

template< class T , unsigned int NRows , unsigned int NColumns > 
template<class C,unsigned int NRowsC,unsigned int NColumnsC>
MatrixExtended< T , NRows , NColumns >
::operator MatrixExtended< C , NRowsC , NColumnsC > const()
{
  MatrixExtended< C , NRowsC , NColumnsC > tmp ;
  for( int i = 0 ; i < NRows ; i++ )
    {
    for( int j = 0 ; j < NColumns ; j++ )
      {
      tmp[ i ][ j ] = ( C ) ( *this ) [ i ][ j ] ;
      }
    }
  return tmp ;
}



template< class T , unsigned int NRows , unsigned int NColumns >
MatrixExtended< T , NRows , NColumns >
MatrixExtended< T , NRows , NColumns >
::operator=( const Self & matrix )
{
  for( int i = 0 ; i < NRows ; i++ )
    {
    for( int j = 0 ; j < NColumns ; j++ )
      {
      ( *this ) [ i ][ j ] = matrix[ i ][ j ] ;
      }
    }
  return *this ;
}

template< class T, unsigned int NRows, unsigned int NColumns >
MatrixExtended< T , NRows , NColumns >
MatrixExtended< T , NRows , NColumns >
::operator=( const Superclass & matrix )
{
  for( int i = 0 ; i < NRows ; i++ )
    {
    for( int j = 0 ; j < NColumns ; j++ )
      {
      ( *this ) [ i ][ j ] = matrix[ i ][ j ] ;
      }
    }
  return *this ;
}


template< class T , unsigned int NRows , unsigned int NColumns >
MatrixExtended< T , NRows , NColumns >
MatrixExtended< T , NRows , NColumns >
::operator=( const InternalMatrixType & matrix )
{
  for( int i = 0 ; i < NRows ; i++ )
    {
    for( int j = 0 ; j < NColumns ; j++ )
      {
      ( *this ) [ i ][ j ] = matrix[ i ][ j ] ;
      }
    }
  return *this ;
}



}//end namespace itk
#endif
