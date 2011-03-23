#ifndef _GenericTransformImage_H_
#define _GenericTransformImage_H_

#include "BRAINSCommonLibWin32Header.h"

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
#include "itkScaleVersor3DTransform.h"
#include "itkScaleSkewVersor3DTransform.h"
#include "itkAffineTransform.h"
#include <itkBSplineDeformableTransform.h>
#include <itkThinPlateR2LogRSplineKernelTransform.h>
#include "itkVersorRigid3DTransform.h"
#include "ConvertToRigidAffine.h"
#include "itkResampleImageFilter.h"
#include "itkImageDuplicator.h"
#include "Imgmath.h"

#include "itkTransformFactory.h"
#include "itkTransformFileReader.h"
#include "itkTransformFileWriter.h"
//
//TODO:  The next two should be hidden in the cxx files again.
typedef itk::TransformFileReader                    TransformReaderType;
typedef itk::TransformFileReader::TransformListType TransformListType;

//TODO:  This should really be taken as a typedef from the BSpline class.
//TODO:  These should be hidden in the BRAINSFit namespace.
typedef itk::Transform< double, 3, 3 > GenericTransformType;

namespace GenericTransformImageNS
{
static const unsigned int SpaceDimension = 3;
static const unsigned int SplineOrder = 3;
}

typedef double CoordinateRepType;
typedef itk::BSplineDeformableTransform<
  CoordinateRepType,
  GenericTransformImageNS::SpaceDimension,
  GenericTransformImageNS::SplineOrder > BSplineTransformType;

typedef itk::AffineTransform< double, 3 >         AffineTransformType;
typedef itk::VersorRigid3DTransform< double >     VersorRigid3DTransformType;
typedef itk::ScaleVersor3DTransform< double >     ScaleVersor3DTransformType;
typedef itk::ScaleSkewVersor3DTransform< double > ScaleSkewVersor3DTransformType;
typedef itk::ThinPlateR2LogRSplineKernelTransform< double, 3 > ThinPlateSpline3DTransformType;

namespace itk
{
/**
 * \author Hans J. Johnson
 * \brief This templated function will duplicate the input image, change the direction and origin to refelect the physical space
 * tranform that would be equivalent to calling the resample image filter.
 * InplaceImage=SetRigidTransformInPlace(RigidTransform,InputImage); ResampleImage(InplaceImage,Identity);
 * should produce the same result as ResampleImage(InputImage,RigidTransform);
 * \param RigidTransform -- Currently must be a VersorRigid3D
 * \param InputImage The image to be duplicated and modified to incorporate the rigid transform.
 * \return an image with the same voxels values as the input, but with differnt physical space representation.
 */
  template< class IOImageType >
  typename IOImageType::Pointer SetRigidTransformInPlace(typename VersorRigid3DTransformType::ConstPointer RigidTransform,//typename
                                                                                                                          //IOImageType::ConstPointer
                                                                                                                          //InputImage)
                                                         const IOImageType *InputImage)
      {
      typename VersorRigid3DTransformType::Pointer InvOfRigidTransform=VersorRigid3DTransformType::New();
      const typename IOImageType::PointType centerPoint = RigidTransform->GetCenter();
      InvOfRigidTransform->SetCenter( centerPoint );
      InvOfRigidTransform->SetIdentity();
      RigidTransform->GetInverse(InvOfRigidTransform);

      /** Wei: The output image will have exact the same index contents
        but with modified image info so that the index-to-physical mapping
        makes the image in the physical space AC-PC aligned */
      typedef ImageDuplicator< IOImageType > DuplicatorType;
      typename DuplicatorType::Pointer duplicator = DuplicatorType::New();
      duplicator->SetInputImage(InputImage);
      duplicator->Update();
      typename IOImageType::Pointer OutputAlignedImage = duplicator->GetOutput();
      //Now change the Origin and Direction to make data aligned.
      OutputAlignedImage->SetOrigin( InvOfRigidTransform->GetMatrix() * InputImage->GetOrigin() + InvOfRigidTransform->GetTranslation() );
      OutputAlignedImage->SetDirection( InvOfRigidTransform->GetMatrix() * InputImage->GetDirection() );
      OutputAlignedImage->SetMetaDataDictionary(InputImage->GetMetaDataDictionary());
      return OutputAlignedImage;
      }
/**
  * \author Hans J. Johnson
  * \brief A utility function to write ITK compliant transforms to disk in a way
  *that is compliant with the ReadTransformFromDisk
  * \param genericTransformToWrite A pointer to baseclass
  *itk::Transform<double,3,3> that is
  * polymorphically cast to a real class like (i.e. itk::VersorRigid,
  *itk::Affine, itk::BSpline, or similar)
  * \param outputTransform the filename of the output transform.
  * \code
  * //To convert from non-const smart pointers ploymorphically to the smart
  *ConstPointer up the class tree, use the GetPointer
  * AffineTransformType::Pointer myAffine=AffineTransformType::New(); //NOTE:
  * This is not a const smart pointer
  * WriteTransformToDisk(myAffine.GetPointer(), "myAffineFile.mat");
  * \endcode
  */
BRAINSCommonLib_EXPORT extern void WriteTransformToDisk(GenericTransformType const *const genericTransformToWrite,
                                                        const std::string outputTransform);

/**
  * \author Hans J. Johnson
  * \brief A utility function to read ITK compliant transforms to disk in a way
  *that is compliant with the WriteTransformFromDisk
  * \param outputTransform the filename of the output transform.
  * \return A pointer to baseclass itk::Transform<double,3,3> that is
  * polymorphically convertable to a real class like (i.e. itk::VersorRigid,
  *itk::Affine, itk::BSpline, or similar)
  * \code
  * //To convert from non-const smart pointers ploymorphically to the smart
  *ConstPointer up the class tree, use the GetPointer
  * GenericTransformType::Pointer
  *myGenericTransform=ReadTransformFromDisk(myAffine.GetPointer(),
  *"myAffineFile.mat");
  *
  * VersorRigid3DTransformType::Pointer myVersorRigid3D=NULL;
  * {
  * const std::string transformFileType = myGenericTransform->GetNameOfClass();
  * if ( transformFileType == "VersorRigid3DTransform" )
  *   {
  *   myVersorRigid3D->SetParameters( versorRigid->GetParameters() );
  *   myVersorRigid3D->SetFixedParameters( versorRigid->GetFixedParameters() );
  *   }
  *   NOTE: It is more safe to copy parameters into the concrete class rather
  *than attempting to dynamically
  *   cast the base classes.  The reason is that the smart pointer management
  *becomes very unweildy and
  *   is hard to keep straight between the pointer that may delete the base
  *class, and the pointer that
  *   is the derived class.
  * }
  * \endcode
  */
BRAINSCommonLib_EXPORT extern GenericTransformType::Pointer ReadTransformFromDisk(const std::string initialTransform);

/**
  * \author Hans J. Johnson
  * \brief A utility function to write ITK compliant transforms to disk in a way
  *that is compliant with the ReadTransformFromDisk
  * \param genericTransformToWrite A pointer to baseclass
  *itk::Transform<double,3,3> that is
  * polymorphically cast to a real class like (i.e. itk::VersorRigid,
  *itk::Affine, itk::BSpline, or similar)
  * \param outputTransform the filename of the output transform.
  * \code
  * //To convert from non-const smart pointers ploymorphically to the smart
  *ConstPointer up the class tree, use the GetPointer
  * AffineTransformType::Pointer myAffine=AffineTransformType::New(); //NOTE:
  * This is not a const smart pointer
  * WriteTransformToDisk(myAffine.GetPointer(), "myAffineFile.mat");
  * \endcode
  */
BRAINSCommonLib_EXPORT extern VersorRigid3DTransformType::Pointer ComputeRigidTransformFromGeneric(
  const GenericTransformType::ConstPointer genericTransformToWrite);

/**
  * \author Hans J. Johnson
  * \brief Special purpose convenience function -- should not have a public
  *interface.
  */
BRAINSCommonLib_EXPORT extern int WriteBothTransformsToDisk(
  const GenericTransformType::ConstPointer genericTransformToWrite,
  const std::string & outputTransform,
  const std::string & strippedOutputTransform);

/**
  * \author Hans J. Johnson
  * \brief Special purpose convenience function -- should not have a public
  *interface.
  */
BRAINSCommonLib_EXPORT extern int WriteStrippedRigidTransformToDisk(
  const GenericTransformType::ConstPointer genericTransformToWrite,
  const std::string & strippedOutputTransform);

BRAINSCommonLib_EXPORT extern void AddExtraTransformRegister(void);
}

/**
  * \author Hans J. Johnson
  * \brief A class to transform images
  */
template< class InputImageType, class OutputImageType >
typename OutputImageType::Pointer
TransformResample(
  InputImageType const *const inputImage,
  const itk::ImageBase< InputImageType::ImageDimension > *ReferenceImage,
  typename InputImageType::PixelType defaultValue,
  typename itk::InterpolateImageFunction< InputImageType,
                                          typename itk::NumericTraits< typename InputImageType::PixelType >::RealType >
  ::Pointer interp,
  typename GenericTransformType::Pointer transform);

/**
  * \author Hans J. Johnson
  * \brief A class to transform images
  */
template< class InputImageType, class OutputImageType, class DeformationImageType >
typename OutputImageType::Pointer
TransformWarp(
  InputImageType const *const inputImage,
  const itk::ImageBase< InputImageType::ImageDimension > *ReferenceImage,
  typename InputImageType::PixelType defaultValue,
  typename itk::InterpolateImageFunction< InputImageType,
                                          typename itk::NumericTraits< typename InputImageType::PixelType >::RealType >
  ::Pointer interp,
  typename DeformationImageType::Pointer deformationField);

/**
  * \author Hans J. Johnson
  * \brief A class to transform images.  Only one of genericTransform or
  *DeformationField can be non-null.
  */
template< typename InputImageType, class OutputImageType, typename DeformationImageType >
typename OutputImageType::Pointer GenericTransformImage(
  InputImageType const *const OperandImage,
  const itk::ImageBase< InputImageType::ImageDimension > *ReferenceImage,
  typename DeformationImageType::Pointer DeformationField,
  typename GenericTransformType::Pointer genericTransform,
  typename InputImageType::PixelType suggestedDefaultValue, //NOTE:  This is
                                                            // ignored in the
                                                            // case of binary
                                                            // image!
  const std::string interpolationMode,
  const bool binaryFlag);

#ifndef ITK_MANUAL_INSTANTIATION
#  include "GenericTransformImage.txx"
#endif

#endif
