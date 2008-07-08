#ifndef __itkDiffusionTensor3DLinearInterpolateFunction_txx
#define __itkDiffusionTensor3DLinearInterpolateFunction_txx

#include "itkDiffusionTensor3DLinearInterpolateFunction.h"

namespace itk
{

    
template< class TData >
void
DiffusionTensor3DLinearInterpolateFunction< TData >
::AllocateInterpolator()
{
  for( int i = 0 ; i < 6 ; i++ )
    {
    linearInterpolator[ i ] = LinearInterpolateImageFunctionType::New() ;
    this->interpol[ i ] = linearInterpolator[ i ] ;
    }
}

}//end itk namespace

#endif
