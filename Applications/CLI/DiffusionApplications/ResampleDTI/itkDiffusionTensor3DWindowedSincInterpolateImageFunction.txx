#ifndef __itkDiffusionTensor3DWindowedSincInterpolateFunction_txx
#define __itkDiffusionTensor3DWindowedSincInterpolateFunction_txx

#include "itkDiffusionTensor3DWindowedSincInterpolateImageFunction.h"

namespace itk
{
    
    
template< class TData ,
         unsigned int VRadius ,
         class TWindowFunction ,
         class TBoundaryCondition >
void
DiffusionTensor3DWindowedSincInterpolateImageFunction< TData ,
                                                      VRadius ,
                                                      TWindowFunction ,
                                                      TBoundaryCondition >
::AllocateInterpolator()
{
  for( int i = 0 ; i < 6 ; i++ )
    {
    windowedSincInterpolator[ i ] = WindowedSincInterpolateImageFunction::New() ;
    this->interpol[ i ] = windowedSincInterpolator[ i ] ;
    }
}



}//end itk namespace

#endif

