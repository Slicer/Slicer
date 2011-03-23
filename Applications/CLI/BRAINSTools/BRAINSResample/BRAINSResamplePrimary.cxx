/* ==================================================================
 *
 *  TODO:  NEED TO COMMENT WHAT THIS PROGRAM IS TO BE USED FOR
 *  HACK:  Need to update documentation and licensing.
 *
 *  ================================================================== */

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

#include "itkBinaryThresholdImageFilter.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"
#include "itkStatisticsImageFilter.h"
#include "itkMaximumImageFilter.h"
#include "GenericTransformImage.h"

#include "itkTransformToDeformationFieldSource.h"
#include "itkGridForwardWarpImageFilterNew.h"
#include "itkBSplineKernelFunction.h"

#include "itkGridImageSource.h"

typedef float                                                                    InternalPixelType;
typedef itk::Image< InternalPixelType, GenericTransformImageNS::SpaceDimension > TBRAINSResampleInternalImageType;
typedef TBRAINSResampleInternalImageType                                         TBRAINSResampleReferenceImageType;

// A filter to debug the min/max values
template< class TImage >
void PrintImageMinAndMax(TImage *inputImage)
{
  typedef typename itk::StatisticsImageFilter< TImage > StatisticsFilterType;
  typename StatisticsFilterType::Pointer statsFilter = StatisticsFilterType::New();
  statsFilter->SetInput(inputImage);
  statsFilter->Update();
  std::cerr << "StatisticsFilter gave Minimum of " << statsFilter->GetMinimum()
            << " and Maximum of " << statsFilter->GetMaximum() << std::endl;
}

/* This does all the work! */
static int ResampleTransformOrDeformationField(int argc, char *argv[])
{
  PARSE_ARGS;

  const bool debug = true;
  const bool useTransform = ( warpTransform.size() > 0 );
    {
    const bool useDeformationField = ( deformationVolume.size() > 0 );

    if ( debug )
      {
      std::cout << "=====================================================" << std::endl;
      std::cout << "Input Volume:     " <<  inputVolume << std::endl;
      std::cout << "Reference Volume: " <<  referenceVolume << std::endl;
      std::cout << "Output Volume:    " <<  outputVolume << std::endl;
      std::cout << "Pixel Type:       " <<  pixelType << std::endl;
      std::cout << "Interpolation:    " <<  interpolationMode << std::endl;
      std::cout << "Background Value: " <<  defaultValue << std::endl;
      if ( useDeformationField ) { std::cout << "Warp by Deformation Volume: " <<  deformationVolume   << std::endl; }
      if ( useTransform ) { std::cout << "Warp By Transform: "  <<   warpTransform << std::endl; }
      std::cout << "=====================================================" << std::endl;
      }

    if ( useTransform == useDeformationField )
      {
      std::cout
      << "Choose one of the two possibilities, "
         "an ITK compliant transform (BSpline, Rigid, Versor3D, Affine) --or-- a high-dimensional"
         "deformation field."
      << std::endl;
      exit(1);
      }
    }

  TBRAINSResampleInternalImageType::Pointer PrincipalOperandImage;  // One name
                                                                    // for the
                                                                    // image to
                                                                    // be
                                                                    // warped.
    {
    typedef itk::ImageFileReader< TBRAINSResampleInternalImageType > ReaderType;
    ReaderType::Pointer imageReader = ReaderType::New();
    imageReader->SetFileName(inputVolume);
    imageReader->Update();

    PrincipalOperandImage = imageReader->GetOutput();
    }

  // Read ReferenceVolume and DeformationVolume
  typedef float                                                                       VectorComponentType;
  typedef itk::Vector< VectorComponentType, GenericTransformImageNS::SpaceDimension > VectorPixelType;
  typedef itk::Image< VectorPixelType,  GenericTransformImageNS::SpaceDimension >     DeformationFieldType;

  // An empty SmartPointer constructor sets up someImage.IsNull() to represent a
  // not-supplied state:
  DeformationFieldType::Pointer              DeformationField;
  TBRAINSResampleReferenceImageType::Pointer ReferenceImage;

  if ( useTransform )
    {
    typedef itk::ImageFileReader< TBRAINSResampleReferenceImageType > ReaderType;
    ReaderType::Pointer refImageReader = ReaderType::New();
    if ( referenceVolume.size() > 0 )
      {
      refImageReader->SetFileName(referenceVolume);
      }
    else
      {
      std::cout << "Alert:  missing Reference Volume defaulted to: " <<  inputVolume << std::endl;
      refImageReader->SetFileName(inputVolume);
      }
    refImageReader->Update();
    ReferenceImage = refImageReader->GetOutput();
    }
  else if ( !useTransform ) // that is, it's a warp by deformation field:
    {
    typedef itk::ImageFileReader< DeformationFieldType > DefFieldReaderType;
    DefFieldReaderType::Pointer fieldImageReader = DefFieldReaderType::New();
    fieldImageReader->SetFileName(deformationVolume);
    fieldImageReader->Update();
    DeformationField = fieldImageReader->GetOutput();

    if ( referenceVolume.size() > 0 )
      {
      typedef itk::ImageFileReader< TBRAINSResampleReferenceImageType > ReaderType;
      ReaderType::Pointer refImageReader = ReaderType::New();
      refImageReader->SetFileName(referenceVolume);
      refImageReader->Update();
      ReferenceImage = refImageReader->GetOutput();
      }
    // else ReferenceImage.IsNull() represents the delayed default
    }

  // Read optional transform:

  // An empty SmartPointer constructor sets up someTransform.IsNull() to
  // represent a not-supplied state:
  GenericTransformType::Pointer genericTransform;

  if ( useTransform )
    {
    genericTransform = itk::ReadTransformFromDisk(warpTransform);
    }
  TBRAINSResampleInternalImageType::Pointer TransformedImage =
    GenericTransformImage< TBRAINSResampleInternalImageType, TBRAINSResampleInternalImageType, DeformationFieldType >(
      PrincipalOperandImage,
      ReferenceImage,
      DeformationField,
      genericTransform,
      defaultValue,
      interpolationMode,
      pixelType == "binary");
  if ( gridSpacing.size() == TBRAINSResampleInternalImageType::ImageDimension )
    {
    // find min/max pixels for image
    //
    typedef itk::StatisticsImageFilter< TBRAINSResampleInternalImageType > StatisticsFilterType;

    StatisticsFilterType::Pointer statsFilter =
      StatisticsFilterType::New();
    statsFilter->SetInput(TransformedImage);
    statsFilter->Update();
    TBRAINSResampleInternalImageType::PixelType minPixel( statsFilter->GetMinimum() );
    TBRAINSResampleInternalImageType::PixelType maxPixel( statsFilter->GetMaximum() );

    //
    // create the grid
    if ( useTransform )
      { // HACK:  Need to make handeling of transforms more elegant as is done
        // in BRAINSFitHelper.
      typedef itk::TransformToDeformationFieldSource< DeformationFieldType, double > ConverterType;
      ConverterType::Pointer myConverter = ConverterType::New();
      myConverter->SetTransform(genericTransform);
      myConverter->SetOutputParametersFromImage(TransformedImage);
      myConverter->Update();
      DeformationField = myConverter->GetOutput();
      }
    typedef itk::MaximumImageFilter< TBRAINSResampleInternalImageType > MaxFilterType;
    typedef itk::GridForwardWarpImageFilterNew
    < DeformationFieldType, TBRAINSResampleInternalImageType > GFType;
    GFType::Pointer GFFilter = GFType::New();
    GFFilter->SetInput(DeformationField);
    GFType::GridSpacingType GridOffsets;
    GridOffsets[0] = gridSpacing[0];
    GridOffsets[1] = gridSpacing[1];
    GridOffsets[2] = gridSpacing[2];
    GFFilter->SetGridPixelSpacing(GridOffsets);
    GFFilter->SetBackgroundValue(minPixel);
    GFFilter->SetForegroundValue(maxPixel);
    // merge grid with warped image
    MaxFilterType::Pointer MFilter = MaxFilterType::New();
    MFilter->SetInput1( GFFilter->GetOutput() );
    MFilter->SetInput2(TransformedImage);
    MFilter->Update();
    TransformedImage = MFilter->GetOutput();
    }

  // Write out the output image;  threshold it if necessary.
  if ( pixelType == "binary" )
    {
    // A special case for dealing with binary images
    // where signed distance maps are warped and thresholds created
    typedef short int                                                               MaskPixelType;
    typedef itk::Image< MaskPixelType,  GenericTransformImageNS::SpaceDimension >   MaskImageType;
    typedef itk::CastImageFilter< TBRAINSResampleInternalImageType, MaskImageType > CastImageFilter;
    CastImageFilter::Pointer castFilter = CastImageFilter::New();
    castFilter->SetInput(TransformedImage);
    castFilter->Update();

    MaskImageType::Pointer outputImage = castFilter->GetOutput();
    typedef itk::ImageFileWriter< MaskImageType > WriterType;
    WriterType::Pointer imageWriter = WriterType::New();
    imageWriter->UseCompressionOn();
    imageWriter->SetFileName(outputVolume);
    imageWriter->SetInput( castFilter->GetOutput() );
    imageWriter->Update();
    }
  else if ( pixelType == "uchar" )
    {
    typedef unsigned char                                                          NewPixelType;
    typedef itk::Image< NewPixelType, GenericTransformImageNS::SpaceDimension >    NewImageType;
    typedef itk::CastImageFilter< TBRAINSResampleInternalImageType, NewImageType > CastImageFilter;
    CastImageFilter::Pointer castFilter = CastImageFilter::New();
    castFilter->SetInput(TransformedImage);
    castFilter->Update();

    typedef itk::ImageFileWriter< NewImageType > WriterType;
    WriterType::Pointer imageWriter = WriterType::New();
    imageWriter->UseCompressionOn();
    imageWriter->SetFileName(outputVolume);
    imageWriter->SetInput( castFilter->GetOutput() );
    imageWriter->Update();
    }
  else if ( pixelType == "short" )
    {
    typedef signed short                                                           NewPixelType;
    typedef itk::Image< NewPixelType, GenericTransformImageNS::SpaceDimension >    NewImageType;
    typedef itk::CastImageFilter< TBRAINSResampleInternalImageType, NewImageType > CastImageFilter;
    CastImageFilter::Pointer castFilter = CastImageFilter::New();
    castFilter->SetInput(TransformedImage);
    castFilter->Update();

    typedef itk::ImageFileWriter< NewImageType > WriterType;
    WriterType::Pointer imageWriter = WriterType::New();
    imageWriter->UseCompressionOn();
    imageWriter->SetFileName(outputVolume);
    imageWriter->SetInput( castFilter->GetOutput() );
    imageWriter->Update();
    }
  else if ( pixelType == "ushort" )
    {
    typedef unsigned short                                                         NewPixelType;
    typedef itk::Image< NewPixelType, GenericTransformImageNS::SpaceDimension >    NewImageType;
    typedef itk::CastImageFilter< TBRAINSResampleInternalImageType, NewImageType > CastImageFilter;
    CastImageFilter::Pointer castFilter = CastImageFilter::New();
    castFilter->SetInput(TransformedImage);
    castFilter->Update();

    typedef itk::ImageFileWriter< NewImageType > WriterType;
    WriterType::Pointer imageWriter = WriterType::New();
    imageWriter->UseCompressionOn();
    imageWriter->SetFileName(outputVolume);
    imageWriter->SetInput( castFilter->GetOutput() );
    imageWriter->Update();
    }
  else if ( pixelType == "int" )
    {
    typedef int                                                                    NewPixelType;
    typedef itk::Image< NewPixelType, GenericTransformImageNS::SpaceDimension >    NewImageType;
    typedef itk::CastImageFilter< TBRAINSResampleInternalImageType, NewImageType > CastImageFilter;
    CastImageFilter::Pointer castFilter = CastImageFilter::New();
    castFilter->SetInput(TransformedImage);
    castFilter->Update();

    typedef itk::ImageFileWriter< NewImageType > WriterType;
    WriterType::Pointer imageWriter = WriterType::New();
    imageWriter->UseCompressionOn();
    imageWriter->SetFileName(outputVolume);
    imageWriter->SetInput( castFilter->GetOutput() );
    imageWriter->Update();
    }
  else if ( pixelType == "uint" )
    {
    typedef unsigned int                                                           NewPixelType;
    typedef itk::Image< NewPixelType, GenericTransformImageNS::SpaceDimension >    NewImageType;
    typedef itk::CastImageFilter< TBRAINSResampleInternalImageType, NewImageType > CastImageFilter;
    CastImageFilter::Pointer castFilter = CastImageFilter::New();
    castFilter->SetInput(TransformedImage);
    castFilter->Update();
    typedef itk::ImageFileWriter< NewImageType > WriterType;
    WriterType::Pointer imageWriter = WriterType::New();
    imageWriter->UseCompressionOn();
    imageWriter->SetFileName(outputVolume);
    imageWriter->SetInput( castFilter->GetOutput() );
    imageWriter->Update();
    }
  else if ( pixelType == "float" )
    {
    typedef itk::ImageFileWriter< TBRAINSResampleInternalImageType > WriterType;
    WriterType::Pointer imageWriter = WriterType::New();
    imageWriter->UseCompressionOn();
    imageWriter->SetFileName(outputVolume);
    imageWriter->SetInput(TransformedImage);
    imageWriter->Update();
    }
  else
    {
    std::cout << "ERROR:  Invalid pixelType" << std::endl;
    exit (-1);
    }
  return EXIT_SUCCESS;
}

#ifdef WIN32
#define MODULE_EXPORT __declspec(dllexport)
#else
#define MODULE_EXPORT
#endif

extern "C" MODULE_EXPORT int BRAINSResamplePrimary(int, char* []);

int BRAINSResamplePrimary(int argc, char *argv[])
{
  // Apparently when you register one transform, you need to register all your
  // transforms.
  itk::AddExtraTransformRegister();
  return ResampleTransformOrDeformationField(argc, argv);
}
