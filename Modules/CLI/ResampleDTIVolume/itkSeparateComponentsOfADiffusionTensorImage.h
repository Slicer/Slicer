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
  typedef TInput  InputDataType;
  typedef TOutput OutputDataType;

  typedef ImageToImageFilter
  <Image<DiffusionTensor3D<TInput>, 3>,
   Image<TOutput, 3> >
  Superclass;

  typedef DiffusionTensor3D<InputDataType>              InputTensorDataType;
  typedef Image<InputTensorDataType, 3>                 InputImageType;
  typedef SeparateComponentsOfADiffusionTensorImage     Self;
  typedef SmartPointer<Self>                            Pointer;
  typedef SmartPointer<const Self>                      ConstPointer;
  typedef typename InputImageType::Pointer              InputImagePointerType;
  typedef Image<OutputDataType, 3>                      OutputImageType;
  typedef typename OutputImageType::Pointer             OutputImagePointerType;
  typedef itk::ImageRegionIterator<OutputImageType>     OutputIteratorType;
  typedef itk::ImageRegionConstIterator<InputImageType> InputIteratorType;
  typedef typename OutputImageType::RegionType          OutputImageRegionType;
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
