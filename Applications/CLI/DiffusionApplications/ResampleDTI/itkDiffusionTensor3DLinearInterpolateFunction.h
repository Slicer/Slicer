#ifndef __itkDiffusionTensor3DLinearInterpolateFunction_h
#define __itkDiffusionTensor3DLinearInterpolateFunction_h

#include "itkDiffusionTensor3DInterpolateImageFunctionReimplementation.h"
#include <itkLinearInterpolateImageFunction.h>



namespace itk
{
/** \class DiffusionTensor3DInterpolateImageFunctionReimplementation
 * 
 * Blockwise linear interpolation for diffusion tensor images 
 */
template<class TData>
class DiffusionTensor3DLinearInterpolateFunction :
  public DiffusionTensor3DInterpolateImageFunctionReimplementation< TData >
{
public:
  typedef TData DataType ;
  typedef DiffusionTensor3DLinearInterpolateFunction Self ;
  typedef DiffusionTensor3DInterpolateImageFunctionReimplementation< DataType > Superclass ;
  typedef typename Superclass::ImageType ImageType ;
  typedef SmartPointer< Self > Pointer ;
  typedef SmartPointer< const Self > ConstPointer ;
  typedef LinearInterpolateImageFunction< ImageType , double > LinearInterpolateImageFunctionType ;
  
  itkNewMacro(Self) ;

protected:
  void AllocateInterpolator() ;
  typename LinearInterpolateImageFunctionType::Pointer linearInterpolator[ 6 ] ;
};

}//end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DLinearInterpolateFunction.txx"
#endif

#endif
