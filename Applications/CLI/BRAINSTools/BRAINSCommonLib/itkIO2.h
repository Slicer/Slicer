#ifndef _ITKIO2_H_
#define _ITKIO2_H_

#include "itkWindowedSincInterpolateImageFunction.h"

#include "itkTransformFactory.h"

#include "itkScaleVersor3DTransform.h"
#include "itkScaleSkewVersor3DTransform.h"
#include "itkAffineTransform.h"
#include "itkVersorRigid3DTransform.h"

#include "ConvertToRigidAffine.h"
#include "itkTransformFileReader.h"
#include "ApplyField.h"

namespace itkUtil
{
template <class ImageType>
void Denormalize(typename ImageType::Pointer & input, int nx, int ny, int nz )

{
  typedef typename itk::ImageRegionIterator<ImageType> ConstIteratorType;
  ConstIteratorType in1( input, input->GetLargestPossibleRegion() );

  for ( in1.GoToBegin(); !in1.IsAtEnd(); ++in1 )
    {
    in1.Set(in1.Get() * nx * ny * nz);
    }
}

template <class ImageType>
void Normalize(typename ImageType::Pointer & input )

{
  itk::ImageRegionIterator<ImageType> in1( input,
                                          input->GetLargestPossibleRegion() );

  for ( in1.GoToBegin(); !in1.IsAtEnd(); ++in1 )
    {
    const typename ImageType::PixelType value
      = static_cast<typename ImageType::PixelType>( static_cast<float>( in1.Get() )
                                                    / 255.0F );
    in1.Set(value);
    }
}

// This function will allocate a new image with all the same ImageBase
// information (i.e. orientation, size, resolution, origin, MetaData).
template <class OutputImageType, class InputImagePointerType>
typename OutputImageType::Pointer
AllocateSimilarImage(InputImagePointerType ReferenceImage)
{
  // create a OutputImage containing the upper left hand corner and
  // a chunk out of the middle
  typename OutputImageType::Pointer OutputImage = OutputImageType::New();
  OutputImage->SetRegions( ReferenceImage->GetLargestPossibleRegion() );
  OutputImage->CopyInformation(ReferenceImage);
  OutputImage->Allocate();
  OutputImage->FillBuffer(itk::NumericTraits<typename OutputImageType::
        PixelType>::Zero);
  return OutputImage;
}

typedef itk::Image<itk::Vector<float, 3>, 3> TDeformationField;
typedef itk::Image<float, 3>                 FloatImage3DType;

template <class ImageType>
void ImageClear(typename ImageType::Pointer & input )
{
  typedef itk::ImageRegionIterator<ImageType> ConstIteratorType;
  ConstIteratorType in1( input, input->GetLargestPossibleRegion() );

  for ( in1.GoToBegin(); !in1.IsAtEnd(); ++in1 )
    {
    in1.Set(0.0);
    }
}

template <class ImageType>
float GetMax( const typename ImageType::Pointer & rhs)
{
  typedef itk::ImageRegionIterator<ImageType> ConstIteratorType;
  ConstIteratorType in1( rhs, rhs->GetLargestPossibleRegion() );

  in1.GoToBegin();
  typename ImageType::PixelType maxvalue = in1.Get();
  // first element read above, skip it and look at rest of image.
  for ( ++in1; !in1.IsAtEnd(); ++in1 )
    {
    typename ImageType::PixelType current = in1.Get();
    maxvalue = ( current > maxvalue ) ? current : maxvalue;
    }
  return maxvalue;
}

template <class ImageType>
float GetMin( const typename ImageType::Pointer & rhs)
{
  typedef itk::ImageRegionIterator<ImageType> ConstIteratorType;
  ConstIteratorType in1( rhs, rhs->GetLargestPossibleRegion() );

  in1.GoToBegin();
  typename ImageType::PixelType minvalue = in1.Get();

  // first element read above, skip it and look at rest of image.
  for ( ++in1; !in1.IsAtEnd(); ++in1 )
    {
    const typename ImageType::PixelType current = in1.Get();
    minvalue = ( current < minvalue ) ? current : minvalue;
    }
  return minvalue;
}

// versions of GetMin and GetMax for images with vector pixeltype
template <class ImageType>
float GetMax( const typename ImageType::Pointer & rhs, unsigned Index)
{
  typedef itk::ImageRegionIterator<ImageType> ConstIteratorType;
  ConstIteratorType in1( rhs, rhs->GetLargestPossibleRegion() );

  in1.GoToBegin();
  typename ImageType::PixelType maxvalue = in1.Get();
  for ( ++in1; !in1.IsAtEnd(); ++in1 )
    {
    typename ImageType::PixelType current = in1.Get();
    maxvalue[Index]
      = ( current[Index] > maxvalue[Index] ) ? current[Index] : maxvalue[Index];
    }
  return maxvalue[Index];
}

template <class ImageType>
float GetMin( const typename ImageType::Pointer & rhs, unsigned Index)
{
  typedef itk::ImageRegionIterator<ImageType> ConstIteratorType;
  ConstIteratorType in1( rhs, rhs->GetLargestPossibleRegion() );

  in1.GoToBegin();
  typename ImageType::PixelType minvalue = in1.Get();

  for ( ++in1; !in1.IsAtEnd(); ++in1 )
    {
    const typename ImageType::PixelType current = in1.Get();
    minvalue[Index]
      = ( current[Index] < minvalue[Index] ) ? current[Index] : minvalue[Index];
    }
  return minvalue[Index];
}
}

// a nasty hack to get around recursive dependencies between
// itkIO.h and TransformToDeformationField.h
#include "TransformToDeformationField.h"

namespace itkUtil
{
#if 0
template <typename TDeformationField, typename ImageType>
typename TDeformationField::Pointer
RegisterLandmarksToDeformationField(typename ImageType::Pointer & InputImage,
  const std::string & InputLandmarkFilename,
  const std::string & TemplateLandmarkFilename,
  typename ImageType::Pointer & TemplateImage)
{
  try
    {
    typedef itk::ThinPlateR2LogRSplineKernelTransform<double,
      3>
    ThinPlateSplineTransformType;
    typedef ThinPlateSplineTransformType::Pointer
    ThinPlateSplineTransformPointer;
    typedef ThinPlateSplineTransformType::PointSetType            PointSetType;
    typedef itk::InverseConsistentLandmarks<double, PointSetType> LandmarksType;

    ThinPlateSplineTransformPointer ThinPlateSplineTransformITK
      = ThinPlateSplineTransformType::New();

    typename ImageType::SizeType OutputSize
      = TemplateImage->GetLargestPossibleRegion().GetSize();
    typename ImageType::SpacingType OutputSpacing = TemplateImage->GetSpacing();

    typename ImageType::SizeType InputSize
      = InputImage->GetLargestPossibleRegion().GetSize();
    typename ImageType::SpacingType InputSpacing = InputImage->GetSpacing();

    LandmarksType InputLandmarks;
    LandmarksType TemplateLandmarks;
    unsigned int  InputLandmarkSize;
    unsigned int  TemplateLandmarkSize;
    bool          landmarkReadTest = InputLandmarks.ReadPointTypes(
      InputLandmarkFilename.c_str() );
    if ( !landmarkReadTest )
      {
      std::cout << "Error in Input Landmark File Read." << std::endl;
      exit(-1);
      }
    landmarkReadTest = TemplateLandmarks.ReadPointTypes(
      TemplateLandmarkFilename.c_str() );
    if ( !landmarkReadTest )
      {
      std::cout << "Error in Template Landmark File Read." << std::endl;
      exit(-1);
      }

    InputLandmarks.RemoveUnmatchedPoints(TemplateLandmarks);
    TemplateLandmarks.RemoveUnmatchedPoints(InputLandmarks);

    InputLandmarkSize = InputLandmarks.size();
    TemplateLandmarkSize = TemplateLandmarks.size();

    ThinPlateSplineTransformITK->SetSourceLandmarks(
      TemplateLandmarks.GetPointSet( TemplateImage->GetOrigin() ) );
    ThinPlateSplineTransformITK->SetTargetLandmarks( InputLandmarks.GetPointSet(
        InputImage->GetOrigin() ) );
    ThinPlateSplineTransformITK->ComputeWMatrix();
      {
      typename TDeformationField::Pointer tmpdef =
        TransformToDeformationField
        <TDeformationField::Pointer,ThinPlateSplineTransformPointer>
        (TemplateImage,ThinPlateSplineTransformITK);
      return tmpdef;
      }
    }
  catch (... )                  // if anything fails, return empty smart
                                // pointer.
    {
    return typename TDeformationField::Pointer();
    }
}

template <typename TDeformationField, typename ImageType>
typename TDeformationField::Pointer
RegisterLandmarksToDeformationField(const std::string & InputImageFilename,
  const std::string & InputLandmarkFilename,
  const std::string & TemplateLandmarkFilename,
  const std::string & TemplateImageFilename)
{
  typename ImageType::Pointer TemplateImage;

  typename ImageType::Pointer InputImage;
  try
    {
    TemplateImage = itkUtil::ReadImage<ImageType>( TemplateImageFilename.c_str() );
    InputImage = itkUtil::ReadImage<ImageType>( InputImageFilename.c_str() );
    }
  catch (... )
    {
    return typename TDeformationField::Pointer();
    }

  return RegisterLandmarksToDeformationField
         <TDeformationField, ImageType>(InputImage,
                                        InputLandmarkFilename,
                                        TemplateLandmarkFilename,
                                        TemplateImage);
}

#endif
#if 0
template <class ImageType>
typename ImageType::Pointer
AllocateImageFromSize(const typename ImageType::SizeType & size)
{
  typename ImageType::SpacingType spacing;
  typename ImageType::IndexType index;
  for ( unsigned i = 0; i < ImageType::GetImageDimension(); i++ )
    {
    index[i] = 0;
    spacing[i] = 1.0;
    }
  typename ImageType::RegionType region;
  region.SetSize(size);
  region.SetIndex(index);
  return itkUtil::AllocateImageFromRegionAndSpacing<ImageType>
                                       (region, spacing);
}

#endif
/** AllocateImageFromExample creates and allocates an image of the type OutputImageType,
 * using TemplateImageType as the source of size and spacing...
 *
 */
template <class TemplateImageType, class OutputImageType>
typename OutputImageType::Pointer
AllocateImageFromExample(
  const typename TemplateImageType::Pointer & TemplateImage)
{
  typename OutputImageType::Pointer rval = OutputImageType::New();
  rval->CopyInformation(TemplateImage);
  rval->SetRegions( TemplateImage->GetLargestPossibleRegion() );
  rval->Allocate();
  return rval;
}

//
// convenience function where template and output images type are the same
template <class ImageType>
typename ImageType::Pointer
AllocateImageFromExample(const typename ImageType::Pointer & TemplateImage)
{
  return AllocateImageFromExample<ImageType, ImageType>(TemplateImage);
}

/**
 * \author Hans Johnson
 * A utility function for reading in itkAffine transforms (often affine matrix in .mat format).
 */
template <class itkAffineTransformType>
typename itkAffineTransformType::Pointer
ReadITKAffineTransform(const std::string inputTransform)
{
  typedef itk::TransformFileReader              TransformReaderType;
  typedef TransformReaderType::TransformPointer TransformPointer;
  TransformReaderType::Pointer affineReader = TransformReaderType::New();

  affineReader->SetFileName( inputTransform );

  typename itkAffineTransformType::Pointer ITKAffineTransform
    = itkAffineTransformType::New();

  try
    {
    affineReader->Update();
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cerr
     <<
    "ExceptionObject caught trying affineReader in ReadITKAffineTransform !"
     << std::endl;
    std::cerr << err << std::endl;
    return ITKAffineTransform;  // at this point, this is an IsNull() transform
                                // pointer.
    }

  //  This interpretation of a Transform List file as a single AffineTransform's
  // Parameters
  //  is ridiculously restrictive, but there it is.
  TransformPointer initTransform
    = *affineReader->GetTransformList()->begin();
  std::string transformType( initTransform->GetTransformTypeAsString() );
  if ( transformType != "AffineTransform_double_3_3"
       && transformType != "AffineTransform_float_3_3" )
    {
    std::cout
     << "Unsupported transform file TransformBase first transform typestring,"
     << initTransform->GetTransformTypeAsString()
     << std::endl;
    return ITKAffineTransform;  // at this point, this is an IsNull() transform
                                // pointer.
    }

  ITKAffineTransform->SetIdentity();

  ITKAffineTransform->SetParameters( initTransform->GetParameters() );
  ITKAffineTransform->SetFixedParameters( initTransform->GetFixedParameters() );

  return ITKAffineTransform;
}

/**
 * \author Greg Harris
 * A utility function for reading in BRAINSFit output transforms (often affine matrix in .mat format).
 */

template <class itkDesiredTransformType>
typename itkDesiredTransformType::Pointer
ReadITKDesiredTransform(const std::string inputTransform)
{
  typedef itk::TransformFileReader              TransformReaderType;
  typedef TransformReaderType::TransformPointer TransformPointer;
  TransformReaderType::Pointer transformReader = TransformReaderType::New();

  transformReader->SetFileName( inputTransform );

  typename itkDesiredTransformType::Pointer ITKDesiredTransform
    = itkDesiredTransformType::New();
  ITKDesiredTransform->SetIdentity();

  try
    {
    transformReader->Update();
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cerr
     <<
    "ExceptionObject caught trying transformReader in ReadITKDesiredTransform !"
     << std::endl;
    std::cerr << err << std::endl;
    return ITKDesiredTransform;  // at this point, this is an IsNull() transform
                                 // pointer.
    }

  // relaxing restrictions just a bit, there are 4 transform types recognized
  // here:

  enum {
    Dimension = 3,
    MaxInputDimension = 4
    };

  // 6 Parameters
  typedef itk::VersorRigid3DTransform<double> VersorRigid3DTransformType;
  // 9 Parameters
  typedef itk::ScaleVersor3DTransform<double> ScaleVersor3DTransformType;
  // 15 Parameters
  typedef itk::ScaleSkewVersor3DTransform<double> ScaleSkewVersor3DTransformType;
  // 12 Parameters
  typedef itk::AffineTransform<double, Dimension> AffineTransformType;

  typedef itk::TransformFileReader::TransformListType TransformListType;
  TransformListType *transforms = transformReader->GetTransformList();

  if ( ITKDesiredTransform->GetNumberOfParameters() == 6 )    // Process
                                                              // ITKDesiredTransform
                                                              // as
                                                              // VersorRigid3DTransform:
    {
    try
      {
      typename TransformListType::const_iterator it = transforms->begin();
      const std::string transformFileType = ( *it )->GetNameOfClass();
      if ( transformFileType == "VersorRigid3DTransform" )
        {
        typename VersorRigid3DTransformType::Pointer tempITKTransform
          = static_cast<VersorRigid3DTransformType *>( ( *it ).GetPointer() );
        AssignRigid::AssignConvertedTransform(ITKDesiredTransform,
          tempITKTransform);
        }
      else  // can't be ScaleVersor3DTransform || ScaleSkewVersor3DTransform ||
            //  AffineTransform
        {
        std::cout
         <<
        "Unsupported initial transform file -- TransformBase first transform typestring, "
         << transformFileType
         << " not equal to required type VersorRigid3DTransform"
         << std::endl;

        ITKDesiredTransform->SetIdentity();
        return ITKDesiredTransform;
        }
      }
    catch ( itk::ExceptionObject & excp )
      {
      std::cout << "[FAILED]" << std::endl;
      std::cerr << "Error while processing first transform on list of "
                << transforms->size() << " transforms" << std::endl;
      std::cerr << excp << std::endl;

      ITKDesiredTransform->SetIdentity();
      return ITKDesiredTransform;
      }
    }
  else if ( ITKDesiredTransform->GetNumberOfParameters() == 9 )    // Process
                                                                   // ITKDesiredTransform
                                                                   // as
                                                                   // ScaleVersor3DTransform:
    {
    try
      {
      typename TransformListType::const_iterator it = transforms->begin();
      const std::string transformFileType = ( *it )->GetNameOfClass();
      if ( transformFileType == "VersorRigid3DTransform" )
        {
        typename VersorRigid3DTransformType::Pointer tempITKTransform
          = static_cast<VersorRigid3DTransformType *>( ( *it ).GetPointer() );
        AssignRigid::AssignConvertedTransform(ITKDesiredTransform,
          tempITKTransform);
        }
      else if ( transformFileType == "ScaleVersor3DTransform" )
        {
        typename ScaleVersor3DTransformType::Pointer tempITKTransform
          = static_cast<ScaleVersor3DTransformType *>( ( *it ).GetPointer() );
        AssignRigid::AssignConvertedTransform(ITKDesiredTransform,
          tempITKTransform);
        }
      else  // can't be ScaleSkewVersor3DTransform || AffineTransform
        {
        std::cout
         <<
        "Unsupported initial transform file -- TransformBase first transform typestring, "
         << transformFileType
         <<
        " not equal to required type VersorRigid3DTransform OR ScaleVersor3DTransform"
         << std::endl;

        ITKDesiredTransform->SetIdentity();
        return ITKDesiredTransform;
        }
      }
    catch ( itk::ExceptionObject & excp )
      {
      std::cout << "[FAILED]" << std::endl;
      std::cerr << "Error while processing first transform on list of "
                << transforms->size() << " transforms" << std::endl;
      std::cerr << excp << std::endl;

      ITKDesiredTransform->SetIdentity();
      return ITKDesiredTransform;
      }
    }
  else if ( ITKDesiredTransform->GetNumberOfParameters() == 15 )   // Process
                                                                   // ITKDesiredTransform
                                                                   // as
                                                                   // ScaleSkewVersor3DTransform:
    {
    try
      {
      typename TransformListType::const_iterator it = transforms->begin();
      const std::string transformFileType = ( *it )->GetNameOfClass();
      if ( transformFileType == "VersorRigid3DTransform" )
        {
        typename VersorRigid3DTransformType::Pointer tempITKTransform
          = static_cast<VersorRigid3DTransformType *>( ( *it ).GetPointer() );
        AssignRigid::AssignConvertedTransform(ITKDesiredTransform,
          tempITKTransform);
        }
      else if ( transformFileType == "ScaleVersor3DTransform" )
        {
        typename ScaleVersor3DTransformType::Pointer tempITKTransform
          = static_cast<ScaleVersor3DTransformType *>( ( *it ).GetPointer() );
        AssignRigid::AssignConvertedTransform(ITKDesiredTransform,
          tempITKTransform);
        }
      else if ( transformFileType == "ScaleSkewVersor3DTransform" )
        {
        typename ScaleSkewVersor3DTransformType::Pointer tempITKTransform
          = static_cast<ScaleSkewVersor3DTransformType *>( ( *it ).GetPointer() );
        AssignRigid::AssignConvertedTransform(ITKDesiredTransform,
          tempITKTransform);
        }
      else  // || transformFileType == "AffineTransform" || transformFileType ==
            // "ScaleVersor3DTransform"
        {
        std::cout
         <<
        "Unsupported initial transform file -- TransformBase first transform typestring, "
         << transformFileType
         <<
        " not equal to required type VersorRigid3DTransform OR ScaleVersor3DTransform OR ScaleSkewVersor3DTransform"
         << std::endl;

        ITKDesiredTransform->SetIdentity();
        return ITKDesiredTransform;
        }
      }
    catch ( itk::ExceptionObject & excp )
      {
      std::cout << "[FAILED]" << std::endl;
      std::cerr << "Error while processing first transform on list of "
                << transforms->size() << " transforms" << std::endl;
      std::cerr << excp << std::endl;

      ITKDesiredTransform->SetIdentity();
      return ITKDesiredTransform;
      }
    }
  else if ( ITKDesiredTransform->GetNumberOfParameters() == 12 )  // Process
                                                                  // ITKDesiredTransform
                                                                  // as
                                                                  // AffineTransform:
    {
    try
      {
      typename TransformListType::const_iterator it = transforms->begin();
      const std::string transformFileType = ( *it )->GetNameOfClass();
      if ( transformFileType == "VersorRigid3DTransform" )
        {
        typename VersorRigid3DTransformType::Pointer tempITKTransform
          = static_cast<VersorRigid3DTransformType *>( ( *it ).GetPointer() );
        AssignRigid::AssignConvertedTransform(ITKDesiredTransform,
          tempITKTransform);
        }
      else if ( transformFileType == "ScaleVersor3DTransform" )
        {
        typename ScaleVersor3DTransformType::Pointer tempITKTransform
          = static_cast<ScaleVersor3DTransformType *>( ( *it ).GetPointer() );
        AssignRigid::AssignConvertedTransform(ITKDesiredTransform,
          tempITKTransform);
        }
      else if ( transformFileType == "ScaleSkewVersor3DTransform" )
        {
        typename ScaleSkewVersor3DTransformType::Pointer tempITKTransform
          = static_cast<ScaleSkewVersor3DTransformType *>( ( *it ).GetPointer() );
        AssignRigid::AssignConvertedTransform(ITKDesiredTransform,
          tempITKTransform);
        }
      else if ( transformFileType == "AffineTransform" )
        {
        typename AffineTransformType::Pointer tempITKTransform
          = static_cast<AffineTransformType *>( ( *it ).GetPointer() );
        AssignRigid::AssignConvertedTransform(ITKDesiredTransform,
          tempITKTransform);
        }
      else  //  NO SUCH CASE!!
        {
        std::cout
         <<
        "Unsupported initial transform file -- TransformBase first transform typestring, "
         << transformFileType
         <<
        " not equal to any recognized type VersorRigid3DTransform OR ScaleVersor3DTransform OR ScaleSkewVersor3DTransform OR AffineTransform"
         << std::endl;

        ITKDesiredTransform->SetIdentity();
        return ITKDesiredTransform;
        }
      }
    catch ( itk::ExceptionObject & excp )
      {
      std::cout << "[FAILED]" << std::endl;
      std::cerr << "Error while processing first transform on list of "
                << transforms->size() << " transforms" << std::endl;
      std::cerr << excp << std::endl;

      ITKDesiredTransform->SetIdentity();
      return ITKDesiredTransform;
      }
    }
  else
    {
    std::cerr
                 << "Error choosing what kind of transform to fit "
                 << std::endl;

    ITKDesiredTransform->SetIdentity();
    return ITKDesiredTransform;
    }

  return ITKDesiredTransform;
}

template <class WarperImageType>
typename WarperImageType::Pointer MIWarper(
  const std::string & SubjToAtlasRegistrationFilename,
  const std::string & MaskName)
{
#if 0
  try
    {
    typedef B2AffineTransform<WarperImageType> B2itk::
      AffineTransform<double, 3>;
    typedef typename B2itk::AffineTransform<double,
      3>::TransformType TransformType;
    B2itk::AffineTransform<double, 3> transformObject;
    transformObject.Read(SubjToAtlasRegistrationFilename);
    typename TransformType::Pointer affineTransform
      = transformObject.GetAffineTransformPointer();
    if ( affineTransform.IsNull() )
      {
      std::cerr << "failed to read " << SubjToAtlasRegistrationFilename
                << std::endl;
      throw;
      }
    typename WarperImageType::Pointer InputImage
      = itkUtil::ReadImage<WarperImageType>(MaskName);
    if ( InputImage.IsNull() )
      {
      std::cerr << "failed to read " << MaskName << std::endl;
      throw;
      }
    std::cout << "Mask Registration Complete." << std::endl;
    return transformObject.ApplyTransform(InputImage);
    }
  catch ( itk::ExceptionObject & e )
    {
    std::cerr << "Exception in Registration." << std::endl;
    std::cerr << e.GetDescription() << std::endl;
    std::cerr << e.GetLocation() << std::endl;
    exit(-1);
    }
  return 0;
#else
  itk::AffineTransform<double,
    3>::Pointer transformObject
    = itkUtil::ReadITKDesiredTransform<itk::AffineTransform<double, 3> >(
    SubjToAtlasRegistrationFilename);

  typename WarperImageType::Pointer InputImage
    = itkUtil::ReadImage<WarperImageType>(MaskName);
  if ( InputImage.IsNull() )
    {
    std::cerr << "failed to read " << MaskName << std::endl;
    throw;
    }
  typedef itk::ResampleImageFilter<
    WarperImageType,
    WarperImageType>    ResampleFilterType;

  typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();

  resampler->SetTransform( transformObject );
  resampler->SetInput( InputImage );
  // TODO:  Verify that this is always a mapping onto the same space!
  // TODO:  Since this is mapping of masks, it should use a signed distance
  // mapping.
  // TODO:  or at least a nearest neighbor. Unless a probability map is being
  // generated.
  // TODO:  in which case it should just be nearest neighbor.
  resampler->SetOutputParametersFromImage( InputImage );
  typename WarperImageType::PixelType DefaultPixelValue(
    static_cast<typename WarperImageType::PixelType>( 0 ) );
  resampler->SetDefaultPixelValue( DefaultPixelValue );
  try
    {
    resampler->Update();
    }
  catch ( itk::ExceptionObject & e )
    {
    std::cerr << "Exception in resampling from transformation." << std::endl;
    std::cerr << e.GetDescription() << std::endl;
    std::cerr << e.GetLocation() << std::endl;
    exit(-1);
    }

  typename WarperImageType::Pointer returnImage = resampler->GetOutput();
  // std::cout << returnImage << std::endl;
  std::cout << "Registration Complete for: " << MaskName << "." << std::endl;
  return returnImage;
#endif
}

template <class WarperImageType>
typename WarperImageType::Pointer TDWarper(
  const std::string & SubjToAtlasRegistrationFilename,
  const std::string & MaskName)
{
  typedef itk::Image<itk::Vector<float, 3>, 3> TDWarperTDeformationFieldType;
  try
    {
    TDWarperTDeformationFieldType::Pointer SubjectToAtlasDeformationField
      = itkUtil::ReadImage<TDWarperTDeformationFieldType>(
      SubjToAtlasRegistrationFilename.c_str() );
    typename itk::ApplyField<TDWarperTDeformationFieldType, WarperImageType,
      WarperImageType>::Pointer applyfield
      = itk::ApplyField<TDWarperTDeformationFieldType, WarperImageType,
      WarperImageType>::New();
    applyfield->SetDeformationField(SubjectToAtlasDeformationField);
    typename WarperImageType::Pointer MaskImage
      = itkUtil::ReadImage<WarperImageType>(MaskName);
#if 0
    // Now that we respect ITK physical coordinates, these checks are no longer
    // necessary:
    // Direction, Spacing, Origin ---
    if ( SubjectToAtlasDeformationField->GetDirection() !=
        MaskImage->GetDirection() )
      {
      std::cout << "Directions are different" << std::endl;
      std::cout << SubjectToAtlasDeformationField->GetDirection() << "\n"
                << MaskImage->GetDirection() << std::endl;
      exit(-1);
      }
    if ( SubjectToAtlasDeformationField->GetSpacing() != MaskImage->GetSpacing() )
      {
      std::cout << "Spacings are different" << std::endl;
      std::cout << SubjectToAtlasDeformationField->GetSpacing() << "\n"
                << MaskImage->GetSpacing() << std::endl;
      exit(-1);
      }
    if ( SubjectToAtlasDeformationField->GetOrigin() != MaskImage->GetOrigin() )
      {
      std::cout
       <<
      "WARNING:  Mask and deformation field have different origins.  Resetting the mask origin!"
       << std::endl;
      std::cout << SubjectToAtlasDeformationField->GetOrigin() << "\n"
                << MaskImage->GetOrigin() << std::endl;
      //      MaskImage->SetOrigin(SubjectToAtlasDeformationField->GetOrigin());
      exit(-1);
      }
#endif
    typename itk::MinimumMaximumImageCalculator<WarperImageType>::Pointer
    MinMaxFilter = itk::MinimumMaximumImageCalculator<WarperImageType>::New();
    MinMaxFilter->SetImage(MaskImage);
    MinMaxFilter->Compute();
    std::cout << "Image " << MaskName << " [min,max]= ["
              << static_cast<double>( MinMaxFilter->GetMinimum() ) << ","
              << static_cast<double>( MinMaxFilter->GetMaximum() ) << "]."
              << std::endl;

    // VerifyNonZeroImage<UCHARImageType>(MaskImage,MaskName);
    applyfield->SetInputImage(MaskImage);
    applyfield->Execute();
    std::cout << "Mask Registration Complete." << std::endl;
    typename WarperImageType::Pointer OutputImage = applyfield->GetOutputImage();
    MinMaxFilter->SetImage(OutputImage);
    MinMaxFilter->Compute();
    std::cout << "Image " << MaskName << " [min,max]= ["
              << static_cast<double>( MinMaxFilter->GetMinimum() ) << ","
              << static_cast<double>( MinMaxFilter->GetMaximum() ) << "]."
              << std::endl;
    const bool PrintVerbose = false; // HACK : This should really be something
                                     // controlled by the command line.
    if ( PrintVerbose == true )
      {
      std::cout << "===============" << std::endl;
      std::cout << SubjectToAtlasDeformationField << std::endl;
      std::cout << "===============" << std::endl;
      std::cout << MaskImage << std::endl;
      std::cout << "===============" << std::endl;
      std::cout << OutputImage << std::endl;
      }
    return OutputImage;
    }
  catch ( itk::ExceptionObject & e )
    {
    std::cerr << "Exception in Registration." << std::endl;
    std::cerr << e.GetDescription() << std::endl;
    std::cerr << e.GetLocation() << std::endl;
    exit(-1);
    }
  return 0;
}
}
#endif // __ITKIO2.h
