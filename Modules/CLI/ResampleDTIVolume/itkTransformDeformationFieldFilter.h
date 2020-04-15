#ifndef itkTransformDeformationFieldFilter_h
#define itkTransformDeformationFieldFilter_h

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
  using InputDataType = TInput;
  using OutputDataType = TOutput;

  using Superclass = ImageToImageFilter<Image<itk::Vector<InputDataType, NDimensions>, NDimensions>, Image<itk::Vector<OutputDataType, NDimensions>, NDimensions> >;

  using InputDeformationPixelType = itk::Vector<InputDataType, NDimensions>;
  using InputDeformationFieldType = Image<InputDeformationPixelType, NDimensions>;
  using OutputDeformationPixelType = itk::Vector<OutputDataType, NDimensions>;
  using OutputDeformationFieldType = Image<OutputDeformationPixelType, NDimensions>;
  using Self = TransformDeformationFieldFilter<TInput, TOutput, NDimensions>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  using InputDeformationFieldPointerType = typename InputDeformationFieldType::Pointer;
  using OutputDeformationFieldPointerType = typename OutputDeformationFieldType::Pointer;
  using InputIteratorType = ImageRegionConstIteratorWithIndex<InputDeformationFieldType>;
  using OutputIteratorType = ImageRegionIteratorWithIndex<OutputDeformationFieldType>;
  using OutputDeformationFieldRegionType = typename OutputDeformationFieldType::RegionType;
  using TransformType = Transform<OutputDataType, NDimensions, NDimensions>;
  using OutputImageRegionType = typename OutputDeformationFieldType::RegionType;

  /** Run-time type information (and related methods). */
  itkTypeMacro(TransformDeformationFieldFilter, ImageToImageFilter);

  itkNewMacro( Self );
// /Set the transform
  itkSetObjectMacro( Transform, TransformType );

// /Get the time of the last modification of the object
  ModifiedTimeType GetMTime() const override;

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

  void DynamicThreadedGenerateData( const OutputImageRegionType & outputRegionForThread) override;

  void BeforeThreadedGenerateData() override;

  void GenerateOutputInformation() override;

  void GenerateInputRequestedRegion() override;

private:
  typename TransformType::Pointer m_Transform;
// InputDeformationFieldPointerType m_Input ;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTransformDeformationFieldFilter.txx"
#endif

#endif
