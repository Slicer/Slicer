/*=========================================================================

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkSeparateComponentsOfADiffusionTensorImage_h
#define itkSeparateComponentsOfADiffusionTensorImage_h

#include <itkImageToImageFilter.h>
#include <itkImage.h>
#include <itkDiffusionTensor3D.h>
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>

namespace itk
{
/** \class itkSeparateComponentsOfADiffusionTensorImage
 *
 *
 */
template <class TInput, class TOutput>
class SeparateComponentsOfADiffusionTensorImage
  : public ImageToImageFilter
  <Image<DiffusionTensor3D<TInput>, 3>,
   Image<TOutput, 3> >
{
public:
  using InputDataType = TInput;
  using OutputDataType = TOutput;

  using Superclass = ImageToImageFilter<Image<DiffusionTensor3D<TInput>, 3>, Image<TOutput, 3> >;

  using InputTensorDataType = DiffusionTensor3D<InputDataType>;
  using InputImageType = Image<InputTensorDataType, 3>;
  using Self = SeparateComponentsOfADiffusionTensorImage<TInput, TOutput>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using InputImagePointerType = typename InputImageType::Pointer;
  using OutputImageType = Image<OutputDataType, 3>;
  using OutputImagePointerType = typename OutputImageType::Pointer;
  using OutputIteratorType = itk::ImageRegionIterator<OutputImageType>;
  using InputIteratorType = itk::ImageRegionConstIterator<InputImageType>;
  using OutputImageRegionType = typename OutputImageType::RegionType;
// typedef typename OutputTensorDataType::RealValueType TensorRealType ;

  /** Run-time type information (and related methods). */
  itkTypeMacro(SeparateComponentsOfADiffusionTensorImage, ImageToImageFilter);

  itkNewMacro( Self );
protected:
  SeparateComponentsOfADiffusionTensorImage();

  void DynamicThreadedGenerateData( const OutputImageRegionType & outputRegionForThread) override;

  void GenerateOutputInformation() override;

  void GenerateInputRequestedRegion() override;

private:

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkSeparateComponentsOfADiffusionTensorImage.txx"
#endif

#endif
