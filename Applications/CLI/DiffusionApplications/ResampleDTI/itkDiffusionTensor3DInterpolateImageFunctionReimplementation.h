#ifndef __itkDiffusionTensor3DInterpolateImageFunctionReimplementation_h
#define __itkDiffusionTensor3DInterpolateImageFunctionReimplementation_h

#include "itkDiffusionTensor3DInterpolateImageFunction.h"
#include <itkOrientedImage.h>
#include <itkImageRegionIterator.h>
#include <itkInterpolateImageFunction.h>

namespace itk
{

/**
 * \class DiffusionTensor3DInterpolateImageFunctionReimplementation
 * 
 * Abstract class allowing to implement blockwise interpolation for diffusion tensor images
 */

template< class TData >
class DiffusionTensor3DInterpolateImageFunctionReimplementation :
  public DiffusionTensor3DInterpolateImageFunction< TData >
{
public :
  typedef TData DataType ;
  typedef DiffusionTensor3DInterpolateImageFunctionReimplementation Self ;
  typedef DiffusionTensor3DInterpolateImageFunction< DataType > Superclass ;
  typedef typename Superclass::TensorDataType TensorDataType ;
  typedef typename Superclass::DiffusionImageType DiffusionImageType ;
  typedef typename Superclass::DiffusionImageTypePointer DiffusionImageTypePointer ;
  typedef OrientedImage< DataType , 3 > ImageType ;
  typedef typename ImageType::Pointer ImagePointer ;
  typedef typename Superclass::PointType PointType ;
  typedef SmartPointer< Self > Pointer ;
  typedef SmartPointer< const Self > ConstPointer ;
  typedef typename itk::ImageRegionIterator< DiffusionImageType > IteratorDiffusionImageType ;
  typedef typename itk::ImageRegionIterator< ImageType > IteratorImageType ;
  typedef InterpolateImageFunction< ImageType , double > InterpolateImageFunctionType ;
  /** Evaluate the interpolated tensor at a position
   */
  TensorDataType Evaluate( const PointType &point ) ;

protected:
  virtual void AllocateInterpolator() = 0 ;
  void PreCompute() ;  
 // DiffusionTensor3DInterpolateImageFunctionReimplementation();
  typename InterpolateImageFunctionType::Pointer interpol[ 6 ] ;
  ImagePointer m_Image[ 6 ] ;

};

}//end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DInterpolateImageFunctionReimplementation.txx"
#endif

#endif
