#ifndef __BRAINSFitUtils_h
#define __BRAINSFitUtils_h

#include "itkImageRegionIteratorWithIndex.h"

#include "itkScaleVersor3DTransform.h"
#include "itkScaleSkewVersor3DTransform.h"
#include "itkAffineTransform.h"
#include "itkVersorRigid3DTransform.h"
#include "itkBSplineDeformableTransform.h"

#include "itkBRAINSROIAutoImageFilter.h"

/**
  * This file contains utility functions that are common to a few of the
  *BRAINSFit Programs.
  */

static const unsigned int BFNSSpaceDimension = 3;
static const unsigned int BFNSplineOrder = 3;
typedef double CoordinateRepType;
typedef itk::BSplineDeformableTransform<
  CoordinateRepType,
  BFNSSpaceDimension,
  BFNSplineOrder >                                        BSplineTransformType;

typedef itk::VersorRigid3DTransform< double >              VersorRigid3DTransformType;
typedef itk::ScaleVersor3DTransform< double >              ScaleVersor3DTransformType;
typedef itk::ScaleSkewVersor3DTransform< double >          ScaleSkewVersor3DTransformType;
typedef itk::AffineTransform< double, BFNSSpaceDimension > AffineTransformType;

template< class TransformType, unsigned int VImageDimension >
void DoCenteredTransformMaskClipping(
  ImageMaskPointer & fixedMask,
  ImageMaskPointer & movingMask,
  typename TransformType::Pointer transform,
  double maskInferiorCutOffFromCenter)
{
  if ( fixedMask.IsNull()  ||  movingMask.IsNull() )
    {
    return;
    }
  if ( maskInferiorCutOffFromCenter >= 1000.0 )
    {
    return;
    }
  std::cerr << "maskInferiorCutOffFromCenter is " << maskInferiorCutOffFromCenter << std::endl;

  typedef itk::ImageMaskSpatialObject< VImageDimension > ImageMaskSpatialObjectType;

  typedef unsigned char                            PixelType;
  typedef itk::Image< PixelType, VImageDimension > MaskImageType;

  typename TransformType::InputPointType rotationCenter = transform->GetCenter();
  typename TransformType::OutputVectorType translationVector = transform->GetTranslation();

  typename MaskImageType::PointType fixedCenter;
  typename MaskImageType::PointType movingCenter;

  for ( unsigned int i = 0; i < VImageDimension; i++ )
    {
    fixedCenter[i]  = rotationCenter[i];
    movingCenter[i] = translationVector[i] - rotationCenter[i];
    }

  typename ImageMaskSpatialObjectType::Pointer fixedImageMask(
    dynamic_cast< ImageMaskSpatialObjectType * >( fixedMask.GetPointer() ) );
  typename ImageMaskSpatialObjectType::Pointer movingImageMask(
    dynamic_cast< ImageMaskSpatialObjectType * >( movingMask.GetPointer() ) );

  typename MaskImageType::Pointer fixedMaskImage  = const_cast< MaskImageType * >( fixedImageMask->GetImage() );
  typename MaskImageType::Pointer movingMaskImage = const_cast< MaskImageType * >( movingImageMask->GetImage() );

  typename MaskImageType::PointType fixedInferior  = fixedCenter;
  typename MaskImageType::PointType movingInferior = movingCenter;

  fixedInferior[2] -= maskInferiorCutOffFromCenter;   // negative because
                                                      // Superior is large in
                                                      // magnitude.
  movingInferior[2] -= maskInferiorCutOffFromCenter;  // ITK works in an LPS
                                                      // system.

  //  Here we will set the appropriate parts of the f/m MaskImages to zeros....
  typename MaskImageType::PixelType zero = 0;
  typename MaskImageType::PointType location;
  typedef itk::ImageRegionIteratorWithIndex< MaskImageType > MaskIteratorType;

  MaskIteratorType fixedIter( fixedMaskImage, fixedMaskImage->GetLargestPossibleRegion() );
  fixedIter.Begin();
  while ( !fixedIter.IsAtEnd() )
    {
    fixedMaskImage->TransformIndexToPhysicalPoint(fixedIter.GetIndex(), location);
    if ( location[2] < fixedInferior[2] )
      {
      fixedIter.Set(zero);
      }
    ++fixedIter;
    }

  MaskIteratorType movingIter( movingMaskImage, movingMaskImage->GetLargestPossibleRegion() );
  movingIter.Begin();
  while ( !movingIter.IsAtEnd() )
    {
    movingMaskImage->TransformIndexToPhysicalPoint(movingIter.GetIndex(), location);
    if ( location[2] < movingInferior[2] )
      {
      movingIter.Set(zero);
      }
    ++movingIter;
    }

  fixedImageMask->SetImage(fixedMaskImage);
  movingImageMask->SetImage(movingMaskImage);

  fixedImageMask->ComputeObjectToWorldTransform();
  movingImageMask->ComputeObjectToWorldTransform();

  fixedMask  = dynamic_cast< ImageMaskSpatialObjectType * >( fixedImageMask.GetPointer() );
  movingMask = dynamic_cast< ImageMaskSpatialObjectType * >( movingImageMask.GetPointer() );
}

#endif // __BRAINSFITUTILS_h
