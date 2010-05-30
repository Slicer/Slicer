/*==================================================================

TODO:  NEED TO COMMENT WHAT THIS PROGRAM IS TO BE USED FOR
HACK:  Need to update documentation and licensing.

==================================================================*/

#include "BRAINSResamplePrimary.h"
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
#include "itkMaximumImageFilter.h"
#include "GenericTransformImage.h"

#include "itkTransformToDeformationFieldSource.h"
#include "itkGridForwardWarpImageFilterNew.h"
#include "itkBSplineKernelFunction.h"

#include "itkGridImageSource.h"

//A filter to debug the min/max values
  template <class TImage>
void PrintImageMinAndMax(TImage * inputImage)
{
  typedef typename itk::StatisticsImageFilter<TImage> StatisticsFilterType;
  typename StatisticsFilterType::Pointer statsFilter = StatisticsFilterType::New();
  statsFilter->SetInput( inputImage );
  statsFilter->Update();
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

    if (useTransform == useDeformationField)
      {
      std::cout
        << "Choose one of the two possibilities, "
        "an ITK compliant transform (BSpline, Rigid, Versor3D, Affine) --or-- a high-dimensional"
        "deformation field."
        << std::endl;
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
  GenericTransformType::Pointer genericTransform;

  if ( useTransform )
    {
    genericTransform=itk::ReadTransformFromDisk(warpTransform);
    }
  ImageType::Pointer TransformedImage
    = GenericTransformImage<ImageType, ImageType, DeformationFieldType>(
      PrincipalOperandImage,
      ReferenceImage,
      DeformationField,
      genericTransform,
      defaultValue,
      interpolationMode,
      pixelType == "binary");
  if(addGrids)
    {
    // find min/max pixels for image
    //
    typedef itk::StatisticsImageFilter<ImageType> StatisticsFilterType;
    typedef itk::MaximumImageFilter<ImageType> MaxFilterType;
    typedef itk::GridForwardWarpImageFilterNew
      <DeformationFieldType,ImageType> GFType;

    StatisticsFilterType::Pointer statsFilter =
      StatisticsFilterType::New();
    statsFilter->SetInput( TransformedImage );
    statsFilter->Update();
    ImageType::PixelType minPixel(statsFilter->GetMinimum());
    ImageType::PixelType maxPixel(statsFilter->GetMaximum());

    itk::GridImageSource<ImageType>::Pointer myGrid=itk::GridImageSource<ImageType>::New();
    myGrid->SetSize(PrincipalOperandImage->GetLargestPossibleRegion().GetSize() );
    myGrid->SetSpacing(PrincipalOperandImage->GetSpacing() );
    myGrid->SetDirection(PrincipalOperandImage->GetDirection() );
    myGrid->SetOrigin(PrincipalOperandImage->GetOrigin() );
    itk::GridImageSource<ImageType>::BoolArrayType WhichDirections;
    WhichDirections[0]=true;
    WhichDirections[1]=true;
    WhichDirections[2]=false;
    myGrid->SetWhichDimensions(WhichDirections);
    itk::GridImageSource<ImageType>::ArrayType GridArray;
    GridArray[0]=15;
    GridArray[1]=15;
    GridArray[2]=0;
    myGrid->SetGridSpacing(GridArray);
      {
      // Specify 0th order B-spline function (Box function)
      typedef itk::BSplineKernelFunction <0> KernelType;
      KernelType::Pointer kernel = KernelType :: New ();
      myGrid->SetKernelFunction(kernel);
      }
    myGrid->SetScale(maxPixel);
    myGrid->Update();

    itk::BinaryThresholdImageFilter<ImageType,ImageType>::Pointer myThresholder=itk::BinaryThresholdImageFilter<ImageType,ImageType>::New();
    myThresholder->SetInput(myGrid->GetOutput());
    myThresholder->SetInsideValue(maxPixel);
    myThresholder->SetOutsideValue(minPixel);
    myThresholder->SetLowerThreshold(0.0);
    myThresholder->SetUpperThreshold(maxPixel-1);
    myThresholder->Update();

    // merge grid with warped image
    MaxFilterType::Pointer MFilter = MaxFilterType::New();
    MFilter->SetInput1(myThresholder->GetOutput() );
    MFilter->SetInput2(PrincipalOperandImage);
    MFilter->Update();
    PrincipalOperandImage = MFilter->GetOutput();

    TransformedImage= GenericTransformImage<ImageType, ImageType,
      DeformationFieldType>(
        PrincipalOperandImage,
        ReferenceImage,
        DeformationField,
        genericTransform,
        defaultValue,
        "Linear",
        pixelType == "binary");
    }

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
