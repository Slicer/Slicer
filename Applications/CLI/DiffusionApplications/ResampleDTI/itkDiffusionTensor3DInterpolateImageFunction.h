#ifndef __itkDiffusionTensor3DInterpolateImageFunction_h
#define __itkDiffusionTensor3DInterpolateImageFunction_h

#include <itkObject.h>
#include "itkDiffusionTensor3D.h"
#include <itkOrientedImage.h>
#include <itkPoint.h>
#include <itkSemaphore.h>
#include <itkNumericTraits.h>

namespace itk
{
/**
 * \class DiffusionTensor3DInterpolateImageFunction
 * 
 * Virtual class to implement diffusion tensor interpolation classes 
 * 
 */
template< class TData >
class DiffusionTensor3DInterpolateImageFunction : public Object
{
public :
  typedef TData TensorType ;
  typedef DiffusionTensor3DInterpolateImageFunction Self ;
  typedef DiffusionTensor3D< TensorType > TensorDataType ;
  typedef OrientedImage< TensorDataType , 3 > DiffusionImageType ;
  typedef typename DiffusionImageType::Pointer DiffusionImageTypePointer ;
  typedef Point< double , 3 > PointType ;
  typedef SmartPointer< Self > Pointer ;
  typedef SmartPointer< const Self > ConstPointer ;

  ///Set the input image
  itkSetObjectMacro( InputImage , DiffusionImageType ) ;
  ///Evaluate the tensor value at a given position
  virtual TensorDataType Evaluate( const PointType &point ) = 0 ;

protected:
  DiffusionTensor3DInterpolateImageFunction() ;
  DiffusionImageTypePointer m_InputImage ;
  Semaphore::Pointer P ;
  PointType m_Origin ;
  PointType m_End ;
  unsigned long latestTime ;
  void PreComputeCorners() ;
};

}//end namespace itk
#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DInterpolateImageFunction.txx"
#endif

#endif
