#ifndef __itkDiffusionTensor3DWindowedSincInterpolateImageFunction_h
#define __itkDiffusionTensor3DWindowedSincInterpolateImageFunction_h

#include "itkDiffusionTensor3DInterpolateImageFunctionReimplementation.h"
#include <itkWindowedSincInterpolateImageFunction.h>
#include <itkImage.h>

namespace itk
{
/**
 * \class DiffusionTensor3DWindowedSincInterpolateImageFunction
 * 
 * Implementation of windowed sinc blockwise interpolation for diffusion tensor images
 */
template< class TData,
         unsigned int VRadius ,
         class TWindowFunction = Function::HammingWindowFunction< VRadius > ,
         class TBoundaryCondition = ConstantBoundaryCondition< OrientedImage< TData , 3 > > >
class DiffusionTensor3DWindowedSincInterpolateImageFunction
 : public DiffusionTensor3DInterpolateImageFunctionReimplementation< TData >
{
public:
  typedef TData DataType ;
  typedef DiffusionTensor3DWindowedSincInterpolateImageFunction Self ;
  typedef DiffusionTensor3DInterpolateImageFunctionReimplementation< DataType > Superclass ;
  typedef typename Superclass::ImageType ImageType ;
  typedef SmartPointer< Self > Pointer ;
  typedef SmartPointer< const Self > ConstPointer ;
  typedef WindowedSincInterpolateImageFunction< ImageType ,
                                    VRadius , TWindowFunction,
                                    TBoundaryCondition , double > WindowedSincInterpolateImageFunction ;
  
  itkNewMacro(Self);

protected:
  void AllocateInterpolator() ;
  typename WindowedSincInterpolateImageFunction::Pointer windowedSincInterpolator[ 6 ] ;
};

}//end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DWindowedSincInterpolateImageFunction.txx"
#endif

#endif
