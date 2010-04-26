#ifndef _GenericTransformImage_H_
#define _GenericTransformImage_H_

#include <iostream>
#include "itkImage.h"
#include "itkCastImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkWarpImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkBSplineInterpolateImageFunction.h"
#include "itkWindowedSincInterpolateImageFunction.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"
#include "itkStatisticsImageFilter.h"
#include "itkTransformFactory.h"
#include <itkTransformFileReader.h>
#include "itkScaleVersor3DTransform.h"
#include "itkScaleSkewVersor3DTransform.h"
#include "itkAffineTransform.h"
#include <itkBSplineDeformableTransform.h>
#include "itkVersorRigid3DTransform.h"
#include "ConvertToRigidAffine.h"
#include "itkResampleImageFilter.h"
#include "itkImageDuplicator.h"
#include "Imgmath.h"


namespace GenericTransformImageNS {
static const unsigned int SpaceDimension = 3;
static const unsigned int SplineOrder = 3;
}


typedef double CoordinateRepType;
typedef itk::BSplineDeformableTransform<
  CoordinateRepType,
  GenericTransformImageNS::SpaceDimension,
  GenericTransformImageNS::SplineOrder > BSplineTransformType;

typedef itk::AffineTransform<double, 3> AffineTransformType;
typedef itk::VersorRigid3DTransform<double> VersorRigid3DTransformType;
typedef itk::ScaleVersor3DTransform<double> ScaleVersor3DTransformType;
typedef itk::ScaleSkewVersor3DTransform<double> ScaleSkewVersor3DTransformType;

  //  These were hoisted from the ApplyWarp main executable.
  //  REFACTOR:  It turned out to be very inconvenient to let RefImage differ from Image.
  typedef float                              PixelType;
  typedef itk::Image<PixelType, GenericTransformImageNS::SpaceDimension>    ImageType;
  typedef float                              RefPixelType;
  typedef itk::Image<RefPixelType, GenericTransformImageNS::SpaceDimension> RefImageType;


AffineTransformType::Pointer
ReadLinearTransform(const std::string & filename);


void ReadDotMatTransformFile(std::string RegistrationFilename,
  BSplineTransformType::Pointer &itkBSplineTransform,
  AffineTransformType::Pointer &ITKAffineTransform,
  bool invertTransform);


template <class TransformType, class InterpolatorType>
typename ImageType::Pointer
TransformResample(
    typename ImageType::Pointer & inputImage,
    typename RefImageType::Pointer & ReferenceImage,
    typename ImageType::PixelType defaultValue,
    typename TransformType::Pointer & transform);


template <typename ImageType,
    typename ReferenceImageType,
    typename DeformationImageType>
typename ImageType::Pointer GenericTransformImage(
    typename ImageType::Pointer OperandImage,
    typename ReferenceImageType::Pointer ReferenceImage,
    typename DeformationImageType::Pointer DeformationField,
    typename ImageType::PixelType defaultValue,
    typename BSplineTransformType::Pointer itkBSplineTransform,
    typename AffineTransformType::Pointer ITKAffineTransform,
    const std::string &interpolationMode,
    bool binaryFlag);



#ifndef ITK_MANUAL_INSTANTIATION
  #include "GenericTransformImage.txx"
#endif

#endif
