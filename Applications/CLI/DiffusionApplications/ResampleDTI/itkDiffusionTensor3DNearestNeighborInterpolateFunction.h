#ifndef __itkDiffusionTensor3DNearestNeighborInterpolateFunction_h
#define __itkDiffusionTensor3DNearestNeighborInterpolateFunction_h


#include "itkDiffusionTensor3DInterpolateImageFunction.h"

namespace itk
{

/** \class DiffusionTensor3DInterpolateImageFunction
 * 
 * Implementation of the nearest neighborhood interpolation for diffusion tensor images
 */

template< class TData >
class DiffusionTensor3DNearestNeighborInterpolateFunction :
  public DiffusionTensor3DInterpolateImageFunction< TData >
{
public:
  typedef TData DataType ;
  typedef DiffusionTensor3DNearestNeighborInterpolateFunction Self ;
  typedef DiffusionTensor3DInterpolateImageFunction< DataType > Superclass ;
  typedef typename Superclass::TensorDataType TensorDataType ;
  typedef typename Superclass::DiffusionImageType DiffusionImageType ;
  typedef typename Superclass::PointType PointType ;
  typedef SmartPointer< Self > Pointer ;
  typedef SmartPointer< const Self > ConstPointer ;

  itkNewMacro( Self ) ;
  ///Evaluate the value of a tensor at a given position
  TensorDataType Evaluate( const PointType &point ) ;

};

}//end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DNearestNeighborInterpolateFunction.txx"
#endif

#endif
