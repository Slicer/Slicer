/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DResample_h
#define itkDiffusionTensor3DResample_h

#include <itkObject.h>
#include <itkImageToImageFilter.h>
#include "itkDiffusionTensor3DTransform.h"
#include "itkDiffusionTensor3DInterpolateImageFunction.h"
#include <itkImage.h>
#include <itkDiffusionTensor3D.h>
#include <itkCastImageFilter.h>
#include <itkImageRegionIterator.h>
#include <itkPoint.h>

#include "itkDiffusionTensor3DConstants.h"

namespace itk
{
/** \class DiffusionTensor3DResample
 *
 * Resample diffusion tensor images
 * A transformation and a interpolation have to be set
 */
template <class TInput, class TOutput>
class DiffusionTensor3DResample
  : public ImageToImageFilter
  <Image<DiffusionTensor3D<TInput>, 3>,
   Image<DiffusionTensor3D<TOutput>, 3> >
{
public:
  using InputDataType = TInput;
  using OutputDataType = TOutput;

  using Superclass = ImageToImageFilter<Image<DiffusionTensor3D<TInput>, 3>, Image<DiffusionTensor3D<TOutput>, 3> >;

  using InputTensorDataType = DiffusionTensor3D<InputDataType>;
  using InputImageType = Image<InputTensorDataType, 3>;
  using OutputTensorDataType = DiffusionTensor3D<OutputDataType>;
  using OutputImageType = Image<OutputTensorDataType, 3>;
  using Self = DiffusionTensor3DResample<TInput, TOutput>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using InterpolatorType = DiffusionTensor3DInterpolateImageFunction<InputDataType>;
  using TransformType = DiffusionTensor3DTransform<InputDataType>;
  using InputImagePointerType = typename InputImageType::Pointer;
  using OutputImagePointerType = typename OutputImageType::Pointer;
  using IteratorType = itk::ImageRegionIterator<OutputImageType>;
  using OutputImageRegionType = typename OutputImageType::RegionType;
// typedef typename OutputTensorDataType::RealValueType TensorRealType ;

  /** Run-time type information (and related methods). */
  itkTypeMacro(DiffusionTensor3DResample, ImageToImageFilter);

  itkNewMacro( Self );
// /Set the transform
  itkSetObjectMacro( Transform, TransformType );
// /Set the interpolation
  itkSetObjectMacro( Interpolator, InterpolatorType );

// /Set the output parameters (size, spacing, origin, orientation) from a reference image
  void SetOutputParametersFromImage( InputImagePointerType Image );

// /Get the time of the last modification of the object
  ModifiedTimeType GetMTime() const override;

  itkSetMacro( DefaultPixelValue, OutputDataType );
  itkGetMacro( DefaultPixelValue, OutputDataType );

  itkSetMacro( OutputOrigin, typename OutputImageType::PointType );
  itkSetMacro( OutputSpacing, typename OutputImageType::SpacingType );
  itkSetMacro( OutputSize, typename OutputImageType::SizeType );
  itkSetMacro( OutputDirection, typename OutputImageType::DirectionType );
  Matrix<double, 3, 3> GetOutputMeasurementFrame();

  itkGetMacro( OutputOrigin, typename OutputImageType::PointType );
  itkGetMacro( OutputSpacing, typename OutputImageType::SpacingType );
  itkGetMacro( OutputSize, typename OutputImageType::SizeType );
  itkGetMacro( OutputDirection, typename OutputImageType::DirectionType );
protected:
  DiffusionTensor3DResample();

  void DynamicThreadedGenerateData( const OutputImageRegionType & outputRegionForThread) override;

  void BeforeThreadedGenerateData() override;

  void GenerateOutputInformation() override;

  void AfterThreadedGenerateData() override;

  void GenerateInputRequestedRegion() override;

private:
  typename InterpolatorType::Pointer      m_Interpolator;
  typename TransformType::Pointer         m_Transform;
  typename OutputImageType::PointType     m_OutputOrigin;
  typename OutputImageType::SpacingType   m_OutputSpacing;
  typename OutputImageType::SizeType      m_OutputSize;
  typename OutputImageType::DirectionType m_OutputDirection;
  OutputDataType                          m_DefaultPixelValue;
  OutputTensorDataType                    m_DefaultTensor;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DResample.txx"
#endif

#endif
