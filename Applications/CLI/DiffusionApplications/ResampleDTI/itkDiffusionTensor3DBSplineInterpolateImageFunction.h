#ifndef __itkDiffusionTensor3DBSplineInterpolateImageFunction_h
#define __itkDiffusionTensor3DBSplineInterpolateImageFunction_h

#include "itkDiffusionTensor3DInterpolateImageFunctionReimplementation.h"
#include <itkBSplineInterpolateImageFunction.h>
#include <itkImage.h>

namespace itk
{
/**
 * \class DiffusionTensor3DSplineInterpolateImageFunction
 * 
 * Implementation of blockwise spline interpolation for diffusion tensor images
 */
template< class TData >
class DiffusionTensor3DBSplineInterpolateImageFunction :
  public DiffusionTensor3DInterpolateImageFunctionReimplementation< TData >
{
public:
  typedef TData DataType ;
  typedef DiffusionTensor3DBSplineInterpolateImageFunction Self ;
  typedef DiffusionTensor3DInterpolateImageFunctionReimplementation< DataType > Superclass ;
  typedef typename Superclass::ImageType ImageType ;
  typedef SmartPointer< Self > Pointer ;
  typedef SmartPointer< const Self > ConstPointer ;
  typedef BSplineInterpolateImageFunction< ImageType , double , double >
                                                 BSplineInterpolateFunction ;
  
  itkNewMacro( Self ) ;
  ///Get the Spline Order, supports 0th - 5th order splines. The default is a 1st order spline. 
  itkGetMacro( SplineOrder , int ) ;
  ///Set the Spline Order, supports 0th - 5th order splines. The default is a 1st order spline.
  itkSetMacro( SplineOrder , unsigned int ) ;

  protected:
  void AllocateInterpolator() ;
  DiffusionTensor3DBSplineInterpolateImageFunction() ;
  unsigned int m_SplineOrder ;
  typename BSplineInterpolateFunction::Pointer bSplineInterpolateFunction[ 6 ] ;
};

}//end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DBSplineInterpolateImageFunction.txx"
#endif

#endif
