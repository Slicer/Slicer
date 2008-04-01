#ifndef __itkDiffusionTensor3DResample_h
#define __itkDiffusionTensor3DResample_h

#include <itkObject.h>
#include <itkImageToImageFilter.h>
#include "itkDiffusionTensor3DTransform.h"
#include "itkDiffusionTensor3DInterpolateImageFunction.h"
#include <itkOrientedImage.h>
#include <itkDiffusionTensor3D.h>
#include <itkCastImageFilter.h>
#include <itkImageRegionIterator.h>
#include <itkPoint.h>


namespace itk
{
/** \class DiffusionTensor3DResample
 * 
 * Resample diffusion tensor images
 * A transformation and a interpolation have to be set
 */
template< class TInput , class TOutput >
class DiffusionTensor3DResample
: public ImageToImageFilter
< OrientedImage < DiffusionTensor3D < TInput > , 3 > ,
  OrientedImage < DiffusionTensor3D < TOutput > , 3 > >
{
public :
typedef TInput InputDataType ;
typedef TOutput OutputDataType ;
typedef ImageToImageFilter
          < OrientedImage < DiffusionTensor3D < TInput > , 3 > ,
            OrientedImage < DiffusionTensor3D < TOutput > , 3 > >
Superclass ;
typedef DiffusionTensor3D< InputDataType > InputTensorDataType ;
typedef OrientedImage< InputTensorDataType , 3 > InputImageType ;
typedef DiffusionTensor3D< OutputDataType > OutputTensorDataType ;
typedef OrientedImage< OutputTensorDataType , 3 > OutputImageType ;
typedef DiffusionTensor3DResample Self ;
typedef SmartPointer< Self > Pointer ;
typedef SmartPointer< const Self > ConstPointer ;
typedef DiffusionTensor3DInterpolateImageFunction< InputDataType > InterpolatorType ;
typedef DiffusionTensor3DTransform< InputDataType > TransformType ;
typedef typename InputImageType::Pointer InputImagePointerType ;
typedef typename OutputImageType::Pointer OutputImagePointerType ;
typedef itk::ImageRegionIterator< OutputImageType > IteratorType ;
typedef typename OutputImageType::RegionType OutputImageRegionType ;

itkNewMacro( Self ) ;
///Set the transform
itkSetObjectMacro( Transform , TransformType ) ;
///Set the interpolation
itkSetObjectMacro( Interpolator , InterpolatorType ) ;
///Set the input image
void SetInput( InputImagePointerType inputImage ) ;
///Set the output parameters (size, spacing, origin, orientation) from a reference image 
void SetOutputParametersFromImage( InputImagePointerType Image ) ;
///Get the time of the last modification of the object
unsigned long GetMTime() const ;


itkSetMacro( OutputOrigin , typename OutputImageType::PointType ) ;
itkSetMacro( OutputSpacing , typename OutputImageType::SpacingType ) ;
itkSetMacro( OutputSize , typename OutputImageType::SizeType ) ;
itkSetMacro( OutputDirection , typename OutputImageType::DirectionType ) ;
Matrix< double , 3 , 3 > GetOutputMeasurementFrame() ;

protected :
DiffusionTensor3DResample() ;
void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread ,
                           int threadId ) ;
void BeforeThreadedGenerateData() ;
void GenerateOutputInformation() ;
void AfterThreadedGenerateData() ;
void GenerateInputRequestedRegion() ;

private :
typename InterpolatorType::Pointer m_Interpolator ;
typename TransformType::Pointer m_Transform ;
typename OutputImageType::PointType m_OutputOrigin ;
typename OutputImageType::SpacingType m_OutputSpacing ;
typename OutputImageType::SizeType m_OutputSize ;
typename OutputImageType::DirectionType m_OutputDirection ;

};

}//end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DResample.txx"
#endif

#endif
