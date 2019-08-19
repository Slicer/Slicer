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
  typedef TInput  InputDataType;
  typedef TOutput OutputDataType;

  typedef ImageToImageFilter
  <Image<DiffusionTensor3D<TInput>, 3>,
   Image<DiffusionTensor3D<TOutput>, 3> > Superclass;

  typedef DiffusionTensor3D<InputDataType>                         InputTensorDataType;
  typedef Image<InputTensorDataType, 3>                            InputImageType;
  typedef DiffusionTensor3D<OutputDataType>                        OutputTensorDataType;
  typedef Image<OutputTensorDataType, 3>                           OutputImageType;
  typedef DiffusionTensor3DResample                                Self;
  typedef SmartPointer<Self>                                       Pointer;
  typedef SmartPointer<const Self>                                 ConstPointer;
  typedef DiffusionTensor3DInterpolateImageFunction<InputDataType> InterpolatorType;
  typedef DiffusionTensor3DTransform<InputDataType>                TransformType;
  typedef typename InputImageType::Pointer                         InputImagePointerType;
  typedef typename OutputImageType::Pointer                        OutputImagePointerType;
  typedef itk::ImageRegionIterator<OutputImageType>                IteratorType;
  typedef typename OutputImageType::RegionType                     OutputImageRegionType;
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
