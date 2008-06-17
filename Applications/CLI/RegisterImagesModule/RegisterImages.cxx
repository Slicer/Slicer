#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "RegisterImagesCLP.h"

#include "itkTimeProbesCollectorBase.h"

#include "itkOrientedImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkMultiThreader.h"

#include "itkImageToImageRegistrationHelper.h"

// Description:
// Get the PixelType and ComponentType from fileName
void GetImageType (std::string fileName,
                   itk::ImageIOBase::IOPixelType &pixelType,
                   itk::ImageIOBase::IOComponentType &componentType)
{
  typedef itk::OrientedImage<short, 3> ImageType;
  itk::ImageFileReader<ImageType>::Pointer imageReader =
        itk::ImageFileReader<ImageType>::New();
  imageReader->SetFileName(fileName.c_str());
  imageReader->UpdateOutputInformation();

  pixelType = imageReader->GetImageIO()->GetPixelType();
  componentType = imageReader->GetImageIO()->GetComponentType();
}

// Description:
// Get the PixelTypes and ComponentTypes from fileNames
void GetImageTypes (std::vector<std::string> fileNames,
                    std::vector<itk::ImageIOBase::IOPixelType> &pixelTypes,
                    std::vector<itk::ImageIOBase::IOComponentType> &componentTypes)
{
  pixelTypes.clear();
  componentTypes.clear();

  // For each file, find the pixel and component type
  for (std::vector<std::string>::size_type i = 0; i < fileNames.size(); i++)
    {
    itk::ImageIOBase::IOPixelType pixelType;
    itk::ImageIOBase::IOComponentType componentType;
    GetImageType (fileNames[i],
                  pixelType,
                  componentType);
    pixelTypes.push_back(pixelType);  
    componentTypes.push_back(componentType);  
    }
}

template <class T>
int DoIt( int argc, char *argv[] )
{

  PARSE_ARGS;

  enum VerboseLevelEnum {SILENT, STANDARD, VERBOSE};
  VerboseLevelEnum verbosity = VERBOSE;

  typedef typename itk::OrientedImage<T, 3> ImageType;

  typedef typename itk::ImageToImageRegistrationHelper< ImageType >  RegerType;

  typename RegerType::Pointer reger = RegerType::New();

  reger->SetReportProgress( true );

  if (verbosity >= STANDARD)
    {
    std::cerr << "### Loading fixed image...";
    }
  reger->LoadFixedImage( fixedImage );
  if (verbosity >= STANDARD)
    {
    std::cerr << "### DONE" << std::endl;
    }

  if (verbosity >= STANDARD)
    {
    std::cerr << "### Loading moving image...";
    }
  reger->LoadMovingImage( movingImage );
  if (verbosity >= STANDARD)
    {
    std::cerr << "### DONE" << std::endl;
    }

  if( loadParameters.size() > 1)
    {
    if (verbosity >= STANDARD)
      {
      std::cerr << "### Loading parameters...";
      }
    reger->LoadParameters( loadParameters );
    if (verbosity >= STANDARD)
      {
      std::cerr << "### DONE" << std::endl;
      }
    }

  if( loadTransform.size() > 1 )
    {
    if (verbosity >= STANDARD)
      {
      std::cerr << "### Loading transform...";
      }
    reger->LoadTransform( loadTransform );
    if (verbosity >= STANDARD)
      {
      std::cerr << "### DONE" << std::endl;
      }
    }

  if( initialization == "None" )
    {
    if (verbosity >= STANDARD)
      {
      std::cerr << "### Initialization: None" << std::endl;
      }
    reger->SetInitialMethodEnum( RegerType::INIT_WITH_NONE );
    }
  else if( initialization == "ImageCenters")
    {
    if (verbosity >= STANDARD)
      {
      std::cerr << "### Initialization: ImageCenters" << std::endl;
      }
    reger->SetInitialMethodEnum( RegerType::INIT_WITH_IMAGE_CENTERS );
    }
  else if( initialization == "SecondMoments")
    {
    if (verbosity >= STANDARD)
      {
      std::cerr << "### Initialization: SecondMoments" << std::endl;
      }
    reger->SetInitialMethodEnum( RegerType::INIT_WITH_SECOND_MOMENTS );
    }
  else //if( initialization == "CentersOfMass")
    {
    if (verbosity >= STANDARD)
      {
      std::cerr << "### Initialization: CentersOfMass" << std::endl;
      }
    reger->SetInitialMethodEnum( RegerType::INIT_WITH_CENTERS_OF_MASS );
    }

  if( registration == "None" )
    {
    if (verbosity >= STANDARD)
      {
      std::cerr << "### Registration: None" << std::endl;
      }
    reger->SetEnableInitialRegistration( false );
    reger->SetEnableRigidRegistration( false );
    reger->SetEnableAffineRegistration( false );
    reger->SetEnableBSplineRegistration( false );
    }
  else if( registration == "Initial" )
    {
    if (verbosity >= STANDARD)
      {
      std::cerr << "### Registration: Initial" << std::endl;
      }
    reger->SetEnableInitialRegistration( true );
    reger->SetEnableRigidRegistration( false );
    reger->SetEnableAffineRegistration( false );
    reger->SetEnableBSplineRegistration( false );
    }
  else if( registration == "Rigid" )
    {
    if (verbosity >= STANDARD)
      {
      std::cerr << "### Registration: Rigid" << std::endl;
      }
    reger->SetEnableInitialRegistration( false );
    reger->SetEnableRigidRegistration( true );
    reger->SetEnableAffineRegistration( false );
    reger->SetEnableBSplineRegistration( false );
    }
  else if( registration == "Affine" )
    {
    if (verbosity >= STANDARD)
      {
      std::cerr << "### Registration: Affine" << std::endl;
      }
    reger->SetEnableInitialRegistration( false );
    reger->SetEnableRigidRegistration( false );
    reger->SetEnableAffineRegistration( true );
    reger->SetEnableBSplineRegistration( false );
    }
  else if( registration == "BSpline" )
    {
    if (verbosity >= STANDARD)
      {
      std::cerr << "### Registration: BSpline" << std::endl;
      }
    reger->SetEnableInitialRegistration( false );
    reger->SetEnableRigidRegistration( false );
    reger->SetEnableAffineRegistration( false );
    reger->SetEnableBSplineRegistration( true );
    }
  else if( registration == "PipelineRigid" )
    {
    if (verbosity >= STANDARD)
      {
      std::cerr << "### Registration: PipelineRigid" << std::endl;
      }
    reger->SetEnableInitialRegistration( true );
    reger->SetEnableRigidRegistration( true );
    reger->SetEnableAffineRegistration( false );
    reger->SetEnableBSplineRegistration( false );
    }
  else if( registration == "PipelineAffine" )
    {
    if (verbosity >= STANDARD)
      {
      std::cerr << "### Registration: PipelineAffine" << std::endl;
      }
    reger->SetEnableInitialRegistration( true );
    reger->SetEnableRigidRegistration( true );
    reger->SetEnableAffineRegistration( true );
    reger->SetEnableBSplineRegistration( false );
    }
  else if( registration == "PipelineBSpline" )
    {
    if (verbosity >= STANDARD)
      {
      std::cerr << "### Registration: PipelineBSpline" << std::endl;
      }
    reger->SetEnableInitialRegistration( true );
    reger->SetEnableRigidRegistration( true );
    reger->SetEnableAffineRegistration( true );
    reger->SetEnableBSplineRegistration( true );
    }

  if( metric == "NormCorr" )
    {
    if (verbosity >= STANDARD)
      {
      std::cerr << "### Metric: NormalizedCorrelation" << std::endl;
      }
    reger->SetRigidMetricMethodEnum
      ( RegerType::OptimizedRegistrationMethodType::
        NORMALIZED_CORRELATION_METRIC );
    }
  else if( metric == "MeanSqrd" )
    {
    if (verbosity >= STANDARD)
      {
      std::cerr << "### Metric: MeanSquared" << std::endl;
      }
    reger->SetRigidMetricMethodEnum
      ( RegerType::OptimizedRegistrationMethodType::MEAN_SQUARED_ERROR_METRIC );
    }
  else // if( metric == "MattesMI" )
    {
    if (verbosity >= STANDARD)
      {
      std::cerr << "### Metric: MattesMutualInformation" << std::endl;
      }
    reger->SetRigidMetricMethodEnum
      ( RegerType::OptimizedRegistrationMethodType::MATTES_MI_METRIC );
    }

  reger->SetUseOverlapAsROI( useOverlapAsROI );
  if (verbosity >= STANDARD)
    {
    std::cerr << "### UseOverlapsAsROI: " << useOverlapAsROI << std::endl;
    }

  reger->SetMinimizeMemory( minimizeMemory );
  if (verbosity >= STANDARD)
    {
    std::cerr << "### MinimizeMemory: " << minimizeMemory << std::endl;
    }

  reger->SetRigidMaxIterations( rigidMaxIterations );
  if (verbosity >= STANDARD)
    {
    std::cerr << "### RigidMaxIterations: " << rigidMaxIterations << std::endl;
    }

  reger->SetAffineMaxIterations( affineMaxIterations );
  if (verbosity >= STANDARD)
    {
    std::cerr << "### AffineMaxIterations: " << affineMaxIterations << std::endl;
    }

  reger->SetBSplineMaxIterations( bsplineMaxIterations );
  if (verbosity >= STANDARD)
    {
    std::cerr << "### BSplineMaxIterations: " << bsplineMaxIterations << std::endl;
    }

  reger->SetRigidSamplingRatio( rigidSamplingRatio );
  if (verbosity >= STANDARD)
    {
    std::cerr << "### RigidSamplingRatio: " << rigidSamplingRatio << std::endl;
    }
  reger->SetAffineSamplingRatio( affineSamplingRatio );
  if (verbosity >= STANDARD)
    {
    std::cerr << "### AffineSamplingRatio: " << affineSamplingRatio << std::endl;
    }
  reger->SetBSplineSamplingRatio( bsplineSamplingRatio );
  if (verbosity >= STANDARD)
    {
    std::cerr << "### BSplineSamplingRatio: " << bsplineSamplingRatio << std::endl;
    }

  reger->SetExpectedOffsetPixelMagnitude( expectedOffset );
  if (verbosity >= STANDARD)
    {
    std::cerr << "### ExpectedOffsetPixelMagnitude: " << expectedOffset 
              << std::endl;
    }

  reger->SetExpectedRotationMagnitude( expectedRotation );
  if (verbosity >= STANDARD)
    {
    std::cerr << "### ExpectedRotationMagnitude: " << expectedRotation 
              << std::endl;
    }

  reger->SetExpectedScaleMagnitude( expectedScale );
  if (verbosity >= STANDARD)
    {
    std::cerr << "### ExpectedScaleMagnitude: " << expectedScale 
              << std::endl;
    }

  reger->SetExpectedSkewMagnitude( expectedSkew );
  if (verbosity >= STANDARD)
    {
    std::cerr << "### ExpectedSkewMagnitude: " << expectedSkew 
              << std::endl;
    }

  reger->SetBSplineControlPointPixelSpacing( controlPointSpacing );
  if (verbosity >= STANDARD)
    {
    std::cerr << "### ExpectedBSplineControlPointPixelSpacing: " 
              << controlPointSpacing 
              << std::endl;
    }

  try
    {
    if (verbosity >= STANDARD)
      {
      std::cerr << "### Starting registration..." << std::endl;
      }
    reger->Update();
    }
  catch( itk::ExceptionObject &excep)
    {
    std::cout << "Exception caught during helper class registration." 
              << excep << std::endl;
    std::cout << "Current Matrix Transform = " << std::endl; 
    reger->GetCurrentMatrixTransform()->Print(std::cout , 2);
    return EXIT_FAILURE;
    }
  catch( ... )
    {
    std::cout << "Uncaught exception during helper class registration." 
              << std::endl;
    return EXIT_FAILURE;
    }

  typename ImageType::ConstPointer resultImage;
  if(useWindowedSinc)
    {
    try
      {
      resultImage = reger->ResampleImage( 
              RegerType::OptimizedRegistrationMethodType::SINC_INTERPOLATION );
      }
    catch( itk::ExceptionObject &excep)
      {
      std::cout << "Exception caught during helper class resampling." 
                << excep << std::endl;
      std::cout << "Current Matrix Transform = " << std::endl; 
      reger->GetCurrentMatrixTransform()->Print(std::cout , 2);
      return EXIT_FAILURE;
      }
    catch( ... )
      {
      std::cout << "Uncaught exception during helper class resampling." 
                << std::endl;
      return EXIT_FAILURE;
      }
    }
  else
    {
    try
      {
      resultImage = reger->ResampleImage();
      }
    catch( itk::ExceptionObject &excep)
      {
      std::cout << "Exception caught during helper class resampling." 
                << excep << std::endl;
      std::cout << "Current Matrix Transform = " << std::endl; 
      reger->GetCurrentMatrixTransform()->Print(std::cout , 2);
      return EXIT_FAILURE;
      }
    catch( ... )
      {
      std::cout << "Uncaught exception during helper class resampling." 
                << std::endl;
      return EXIT_FAILURE;
      }
    }

  try
    {
    reger->SaveImage( resampledImage, resultImage );
  
    if(differenceImage.size() > 1)
      {
      typedef itk::OrientedImage< float, 3 > OutputImageType;
      typedef itk::SubtractImageFilter< ImageType,
                                        ImageType, OutputImageType > FilterType;
      typename FilterType::Pointer filter = FilterType::New();
      filter->SetInput1( reger->GetFixedImage() );
      filter->SetInput2( resultImage );
      filter->Update();
      
      typedef itk::ImageFileWriter< OutputImageType > WriterType;
      WriterType::Pointer writer = WriterType::New();
      writer->SetInput( filter->GetOutput() );
      writer->SetFileName( differenceImage.c_str() );
      writer->Update();
      }

    if( saveTransform.size() > 1 )
      {
      reger->SaveTransform( saveTransform );
      }

    if( saveParameters.size() > 1 )
      {
      reger->SaveParameters( saveParameters );
      }
    }
  catch( itk::ExceptionObject &excep)
    {
    std::cout << "Exception caught during helper class saving." 
              << excep << std::endl;
    return EXIT_FAILURE;
    }
  catch( ... )
    {
    std::cout << "Uncaught exception during helper class saving." << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType pixelType;
  itk::ImageIOBase::IOComponentType componentType;
 
  // tmp for debuging (maybe make this a command line parameter?
  //itk::MultiThreader::SetGlobalDefaultNumberOfThreads(1);

  try
    {
    GetImageType (fixedImage, pixelType, componentType); 

    switch (componentType)
      {
      case itk::ImageIOBase::UCHAR:
      case itk::ImageIOBase::CHAR:
      case itk::ImageIOBase::SHORT:
        return DoIt<short>( argc, argv );
        break;
      case itk::ImageIOBase::USHORT:
      case itk::ImageIOBase::UINT:
      case itk::ImageIOBase::INT:
      case itk::ImageIOBase::ULONG:
      case itk::ImageIOBase::LONG:
      case itk::ImageIOBase::FLOAT:
      case itk::ImageIOBase::DOUBLE:
        return DoIt<float>( argc, argv );
        break;
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
        std::cout << "unknown component type" << std::endl;
        break;
      }
    }
  catch( itk::ExceptionObject &excep)
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

