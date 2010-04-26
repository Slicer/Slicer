/*==================================================================

TODO:  NEED TO COMMENT WHAT THIS PROGRAM IS TO BE USED FOR
HACK:  Need to update documentation and licensing.

==================================================================*/

#include <iostream>
#include "itkVector.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkWarpImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkBSplineInterpolateImageFunction.h"
#include "itkWindowedSincInterpolateImageFunction.h"
#include "BRAINSResamplePrimaryCLP.h"

#include "itkBrains2MaskImageIO.h"
#include "itkBrains2MaskImageIOFactory.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"
#include "itkStatisticsImageFilter.h"

#include "GenericTransformImage.h"
#if 0
static const unsigned int SpaceDimension = 3;
static const unsigned int SplineOrder = 3;
typedef double CoordinateRepType;
typedef itk::BSplineDeformableTransform<
  CoordinateRepType,
  SpaceDimension,
  SplineOrder > BSplineTransformType;

typedef itk::AffineTransform<double, 3> AffineTransformType;
typedef itk::VersorRigid3DTransform<double> VersorRigid3DTransformType;
typedef itk::ScaleVersor3DTransform<double> ScaleVersor3DTransformType;
typedef itk::ScaleSkewVersor3DTransform<double> ScaleSkewVersor3DTransformType;

  //  These were hoisted from the ResampleTransformOrDeformationField main executable.
  //  REFACTOR:  It turned out to be very inconvenient to let RefImage differ from Image.
  const unsigned int GenericTransformImageNS::SpaceDimension = 3;
  typedef float                              PixelType;
  typedef itk::Image<PixelType, GenericTransformImageNS::SpaceDimension>    ImageType;
  typedef float                              RefPixelType;
  typedef itk::Image<RefPixelType, GenericTransformImageNS::SpaceDimension> RefImageType;
#endif

//A filter to debug the min/max values
template <class TImage>
void PrintImageMinAndMax(TImage * inputImage)
{
//  typename TImage::PixelType resultMaximum:
//  typename TImage::PixelType resultMinimum;
  typedef typename itk::StatisticsImageFilter<TImage> StatisticsFilterType;
  typename StatisticsFilterType::Pointer statsFilter = StatisticsFilterType::New();
  statsFilter->SetInput( inputImage );
  statsFilter->Update();
//  resultMaximum = statsFilter->GetMaximum();
//  resultMinimum = statsFilter->GetMinimum();
  std::cerr << "StatisticsFilter gave Minimum of " << statsFilter->GetMinimum()
            << " and Maximum of " << statsFilter->GetMaximum() << std::endl;
}

/* This does all the work! */
static int ResampleTransformOrDeformationField(int argc, char *argv[])
{
  PARSE_ARGS;

  const bool         debug = true;

  const bool useTransform = (warpTransform.size() > 0);
    {
    const bool useDeformationField = (deformationVolume.size() > 0);

    if ( debug )
      {
      std::cout << "=====================================================" << std::endl;
      std::cout << "Input Volume:     " <<  inputVolume << std::endl;
      std::cout << "Reference Volume: " <<  referenceVolume << std::endl;
      std::cout << "Output Volume:    " <<  outputVolume << std::endl;
      std::cout << "Pixel Type:       " <<  pixelType << std::endl;
      std::cout << "Interpolation:    " <<  interpolationMode << std::endl;
      std::cout << "Background Value: " <<  defaultValue << std::endl;
      if (useDeformationField) std::cout << "Warp by Deformation Volume: " <<   deformationVolume   << std::endl;
      if (useTransform)  std::cout << "Warp By Transform: "   <<   warpTransform << std::endl;
      std::cout << "=====================================================" << std::endl;
      }

    if (useTransformMode.size() > 0) 
      {
      std::cout << "Scripting 'code rot' note:  The useTransformMode parameter will be ignored.  Now ResampleTransformOrDeformationField infers the warpTransform type from the contents of the .mat file." << std::endl;
      }

    if (useTransform == useDeformationField) 
      {
      std::cout << "Choose one of the two possibilities, a BRAINSFit transform --or-- a high-dimensional deformation field." << std::endl;
      exit(1);
      }
    }



  ImageType::Pointer PrincipalOperandImage;  // One name for the image to be warped.
    {
    typedef itk::ImageFileReader<ImageType>  ReaderType;
    ReaderType::Pointer imageReader = ReaderType::New();
    imageReader->SetFileName( inputVolume );
    imageReader->Update( );

    PrincipalOperandImage = imageReader->GetOutput();
    //PrincipalOperandImage->DisconnectPipeline();
    }




  // Read ReferenceVolume and DeformationVolume

  typedef float                                      VectorComponentType;
  typedef itk::Vector<VectorComponentType, GenericTransformImageNS::SpaceDimension> VectorPixelType;
  typedef itk::Image<VectorPixelType,  GenericTransformImageNS::SpaceDimension>     DeformationFieldType;

  // An empty SmartPointer constructor sets up someImage.IsNull() to represent a not-supplied state:
  DeformationFieldType::Pointer DeformationField;
  RefImageType::Pointer ReferenceImage;

  if ( useTransform )
    {
    typedef itk::ImageFileReader<RefImageType>   ReaderType;
    ReaderType::Pointer refImageReader = ReaderType::New();
    if ( referenceVolume.size() > 0 )
      {
      refImageReader->SetFileName( referenceVolume );
      }
    else 
      {
      std::cout << "Alert:  missing Reference Volume defaulted to: " <<  inputVolume << std::endl;
      refImageReader->SetFileName( inputVolume );
      }
    refImageReader->Update( );
    ReferenceImage = refImageReader->GetOutput();
    }
  else if (!useTransform) // that is, it's a warp by deformation field:
    {
    typedef itk::ImageFileReader<DeformationFieldType>  DefFieldReaderType;
    DefFieldReaderType::Pointer fieldImageReader = DefFieldReaderType::New();
    fieldImageReader->SetFileName( deformationVolume );
    fieldImageReader->Update( );
    DeformationField = fieldImageReader->GetOutput();

    if ( referenceVolume.size() > 0 )
      {
      typedef itk::ImageFileReader<RefImageType>   ReaderType;
      ReaderType::Pointer refImageReader = ReaderType::New();
      refImageReader->SetFileName( referenceVolume );
      refImageReader->Update( );
      ReferenceImage = refImageReader->GetOutput();
      }
    // else ReferenceImage.IsNull() represents the delayed default
    }



  // Read optional transform:
  
  // An empty SmartPointer constructor sets up someTransform.IsNull() to represent a not-supplied state:
  BSplineTransformType::Pointer itkBSplineTransform;
  AffineTransformType::Pointer ITKAffineTransform;

  if ( useTransform )
    {
#if 0
    bool definitelyBSpline = false;
    
    itk::TransformFileReader::Pointer transformReader
      = itk::TransformFileReader::New();

    transformReader->SetFileName( warpTransform.c_str() );
    transformReader->Update();

    typedef itk::TransformFileReader::TransformListType *TransformListType;
    TransformListType transforms = transformReader->GetTransformList();
    std::cout << "Number of transforms = " << transforms->size() << std::endl;

    itk::TransformFileReader::TransformListType::const_iterator it
      = transforms->begin();

    if (transforms->size() == 1) // There is no bulk transform.
      {
      BulkTransform = AffineTransformType::New();
      BulkTransform->SetIdentity();
      const std::string firstNameOfClass = ( *it )->GetNameOfClass();
      std::cout << "FIRST (and only) NameOfClass = " << firstNameOfClass << std::endl;
      definitelyBSpline = (firstNameOfClass == "BSplineDeformableTransform");
      }
    else // Pick up what we presume was the bulk transform.
      {
      BulkTransform = static_cast<AffineTransformType *>( ( *it ).GetPointer() );
      const std::string firstNameOfClass = ( *it )->GetNameOfClass();
      std::cout << "First (Bulk) NameOfClass = " << firstNameOfClass << std::endl;
      it++;
      const std::string secondNameOfClass = ( *it )->GetNameOfClass();
      std::cout << "SECOND NameOfClass = " << secondNameOfClass << std::endl;
      definitelyBSpline = (secondNameOfClass == "BSplineDeformableTransform");
      }

    if (definitelyBSpline)
      {
      itkBSplineTransform = static_cast<BSplineTransformType *>( ( *it ).GetPointer() );
      itkBSplineTransform->SetBulkTransform( BulkTransform );
      std::cout << "warpTransform recognized as a BSpline." << std::endl;
      }
    else
      {
      ITKAffineTransform = ReadTransform( warpTransform.c_str() );
      std::cout << "warpTransform recognized as one of the linear transforms." << std::endl;

      if ( invertTransform )
        {
        AffineTransformType::Pointer ITKAffineTempTransform
          = AffineTransformType::New( );
        ITKAffineTempTransform->SetIdentity();

        ITKAffineTempTransform->SetFixedParameters(
          ITKAffineTransform->GetFixedParameters() );
        ITKAffineTempTransform->SetParameters( 
          ITKAffineTransform->GetParameters() );
        ITKAffineTempTransform->GetInverse( ITKAffineTransform );
        }
      }
#else
    ReadDotMatTransformFile(warpTransform,
        itkBSplineTransform,
        ITKAffineTransform,
        invertTransform);
#endif
    }

  

  ImageType::Pointer TransformedImage 
    = GenericTransformImage<ImageType,
        RefImageType,
        DeformationFieldType>(
    PrincipalOperandImage,
    ReferenceImage,
    DeformationField,
    defaultValue,
    itkBSplineTransform,
    ITKAffineTransform,
    interpolationMode,
    pixelType == "binary");
  


  // Write out the output image;  threshold it if necessary.
      
  if ( pixelType == "binary" )
    {
    // A special case for dealing with binary images
    // where signed distance maps are warped and thresholds created
    typedef short int                             MaskPixelType;
    typedef itk::Image<MaskPixelType,  GenericTransformImageNS::SpaceDimension> MaskImageType;
    typedef itk::CastImageFilter<ImageType, MaskImageType> CastImageFilter;
    CastImageFilter::Pointer castFilter = CastImageFilter::New();
    castFilter->SetInput( TransformedImage );
    castFilter->Update( );

    MaskImageType::Pointer outputImage = castFilter->GetOutput();
    typedef itk::ImageFileWriter<MaskImageType> WriterType;
    WriterType::Pointer imageWriter = WriterType::New();
    imageWriter->SetFileName( outputVolume );
    imageWriter->SetInput( castFilter->GetOutput() );
    imageWriter->Update( );
    }
  else if ( pixelType == "uchar" )
    {
    typedef unsigned char                    NewPixelType;
    typedef itk::Image<NewPixelType, GenericTransformImageNS::SpaceDimension> NewImageType;
    typedef itk::CastImageFilter<ImageType, NewImageType> CastImageFilter;
    CastImageFilter::Pointer castFilter = CastImageFilter::New();
    castFilter->SetInput( TransformedImage );
    castFilter->Update( );

    typedef itk::ImageFileWriter<NewImageType> WriterType;
    WriterType::Pointer imageWriter = WriterType::New();
    imageWriter->SetFileName( outputVolume );
    imageWriter->SetInput( castFilter->GetOutput() );
    imageWriter->Update( );
    }
  else if ( pixelType == "short" )
    {
    typedef signed short                     NewPixelType;
    typedef itk::Image<NewPixelType, GenericTransformImageNS::SpaceDimension> NewImageType;
    typedef itk::CastImageFilter<ImageType, NewImageType> CastImageFilter;
    CastImageFilter::Pointer castFilter = CastImageFilter::New();
    castFilter->SetInput( TransformedImage );
    castFilter->Update( );

    typedef itk::ImageFileWriter<NewImageType> WriterType;
    WriterType::Pointer imageWriter = WriterType::New();
    imageWriter->SetFileName( outputVolume );
    imageWriter->SetInput( castFilter->GetOutput() );
    imageWriter->Update( );
    }
  else if ( pixelType == "ushort" )
    {
    typedef unsigned short                  NewPixelType;
    typedef itk::Image<NewPixelType, GenericTransformImageNS::SpaceDimension> NewImageType;
    typedef itk::CastImageFilter<ImageType, NewImageType> CastImageFilter;
    CastImageFilter::Pointer castFilter = CastImageFilter::New();
    castFilter->SetInput( TransformedImage );
    castFilter->Update( );

    typedef itk::ImageFileWriter<NewImageType> WriterType;
    WriterType::Pointer imageWriter = WriterType::New();
    imageWriter->SetFileName( outputVolume );
    imageWriter->SetInput( castFilter->GetOutput() );
    imageWriter->Update( );
    }
  else if ( pixelType == "int" )
    {
    typedef int                             NewPixelType;
    typedef itk::Image<NewPixelType, GenericTransformImageNS::SpaceDimension> NewImageType;
    typedef itk::CastImageFilter<ImageType, NewImageType> CastImageFilter;
    CastImageFilter::Pointer castFilter = CastImageFilter::New();
    castFilter->SetInput( TransformedImage );
    castFilter->Update( );

    typedef itk::ImageFileWriter<NewImageType> WriterType;
    WriterType::Pointer imageWriter = WriterType::New();
    imageWriter->SetFileName( outputVolume );
    imageWriter->SetInput( castFilter->GetOutput() );
    imageWriter->Update( );
    }
  else if ( pixelType == "uint" )
    {
    typedef unsigned int                  NewPixelType;
    typedef itk::Image<NewPixelType, GenericTransformImageNS::SpaceDimension> NewImageType;
    typedef itk::CastImageFilter<ImageType, NewImageType> CastImageFilter;
    CastImageFilter::Pointer castFilter = CastImageFilter::New();
    castFilter->SetInput( TransformedImage );
    castFilter->Update( );
;
    typedef itk::ImageFileWriter<NewImageType> WriterType;
    WriterType::Pointer imageWriter = WriterType::New();
    imageWriter->SetFileName( outputVolume );
    imageWriter->SetInput( castFilter->GetOutput() );
    imageWriter->Update( );
    }
  else if ( pixelType == "float" )
    {
    typedef itk::ImageFileWriter<ImageType> WriterType;
    WriterType::Pointer imageWriter = WriterType::New();
    imageWriter->SetFileName( outputVolume );
    imageWriter->SetInput( TransformedImage );
    imageWriter->Update( );
    }
  else
    {
    std::cout << "ERROR:  Invalid pixelType" << std::endl;
    exit (-1);
    }

  return EXIT_SUCCESS;
}

int BRAINSResamplePrimary( int argc, char *argv[] )
{
  //HACK:  BRAINS2 Masks are currently broken
  //The direction cosines are and the direction labels are not consistently being set.

  itk::Brains2MaskImageIOFactory::RegisterOneFactory();

  // Apparently when you register one transform, you need to register all your
  // transforms.
  //
  itk::TransformFactory<VersorRigid3DTransformType>::RegisterTransform();
  itk::TransformFactory<ScaleVersor3DTransformType>::RegisterTransform();
  itk::TransformFactory<ScaleSkewVersor3DTransformType>::RegisterTransform();
  itk::TransformFactory<AffineTransformType>::RegisterTransform();
  itk::TransformFactory<BSplineTransformType>::RegisterTransform();

  return ResampleTransformOrDeformationField(argc, argv);
}
