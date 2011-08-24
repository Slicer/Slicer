#ifndef __itkTransformDeformationFieldFilter_h
#define __itkTransformDeformationFieldFilter_h

#include <itkObject.h>
#include <itkImageToImageFilter.h>
#include <itkImage.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkTransform.h>

namespace itk
{
/** \class TransformDeformationFieldFilter
 *
 * Resample diffusion tensor images
 * A transformation and a interpolation have to be set
 */

template <class TInput, class TOutput, int NDimensions>
class TransformDeformationFieldFilter
  : public ImageToImageFilter
  <Image<itk::Vector<TInput, NDimensions>, NDimensions>,
   Image<itk::Vector<TOutput, NDimensions>, NDimensions> >
{
public:
  typedef TInput  InputDataType;
  typedef TOutput OutputDataType;
  typedef ImageToImageFilter
  <Image<itk::Vector<InputDataType, NDimensions>, NDimensions>,
   Image<itk::Vector<OutputDataType, NDimensions>, NDimensions> >
  Superclass;
  typedef itk::Vector<InputDataType, NDimensions>        InputDeformationPixelType;
  typedef Image<InputDeformationPixelType, NDimensions>  InputDeformationFieldType;
  typedef itk::Vector<OutputDataType, NDimensions>       OutputDeformationPixelType;
  typedef Image<OutputDeformationPixelType, NDimensions> OutputDeformationFieldType;
  typedef TransformDeformationFieldFilter                Self;
  typedef SmartPointer<Self>                             Pointer;
  typedef SmartPointer<const Self>                       ConstPointer;

  typedef typename InputDeformationFieldType::Pointer                  InputDeformationFieldPointerType;
  typedef typename OutputDeformationFieldType::Pointer                 OutputDeformationFieldPointerType;
  typedef ImageRegionConstIteratorWithIndex<InputDeformationFieldType> InputIteratorType;
  typedef ImageRegionIteratorWithIndex<OutputDeformationFieldType>     OutputIteratorType;
  typedef typename OutputDeformationFieldType::RegionType              OutputDeformationFieldRegionType;
  typedef Transform<OutputDataType, NDimensions, NDimensions>          TransformType;
  typedef typename OutputDeformationFieldType::RegionType              OutputImageRegionType;

  itkNewMacro( Self );
// /Set the transform
  itkSetObjectMacro( Transform, TransformType );
#if 0 // HACK ITK_VERSION_MAJOR < 4
// /Set the input deformation field
  void SetInput( const InputDeformationFieldType * inputDeformationField );

#endif

// /Get the time of the last modification of the object
  unsigned long GetMTime() const;

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro( InputConvertibleToDoubleCheck,
                   ( Concept::Convertible<InputDataType, double> ) );
  itkConceptMacro( DoubleConvertibleToOutputCheck,
                   ( Concept::Convertible<double, OutputDataType> ) );
  /** End concept checking */
#endif
protected:
  TransformDeformationFieldFilter();
#if ITK_VERSION_MAJOR < 4
  void ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, int threadId );

#else
  void ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId );

#endif
  void BeforeThreadedGenerateData();

  void GenerateOutputInformation();

  void GenerateInputRequestedRegion();

private:
  typename TransformType::Pointer m_Transform;
// InputDeformationFieldPointerType m_Input ;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTransformDeformationFieldFilter.txx"
#endif

#endif
