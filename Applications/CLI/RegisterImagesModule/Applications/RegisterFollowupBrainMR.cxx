#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "RegisterFollowupBrainMRCLP.h"

#include "itkTimeProbesCollectorBase.h"

#include "itkOrientedImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkMultiThreader.h"

#undef itkDebugMacro
#define itkDebugMacro(args) {}

#include "itkImageToImageRegistrationHelper.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace {


// Description:
// Get the PixelType and ComponentType from fileName
void GetImageType (std::string fileName,
                   itk::ImageIOBase::IOPixelType &pixelType,
                   itk::ImageIOBase::IOComponentType &componentType,
                   unsigned int & dimensions )
{
  typedef itk::OrientedImage<short, 3> ImageType;
  itk::ImageFileReader<ImageType>::Pointer imageReader =
        itk::ImageFileReader<ImageType>::New();
  imageReader->SetFileName(fileName.c_str());
  imageReader->UpdateOutputInformation();

  pixelType = imageReader->GetImageIO()->GetPixelType();
  componentType = imageReader->GetImageIO()->GetComponentType();
  dimensions = imageReader->GetImageIO()->GetNumberOfDimensions();
}

template <unsigned int DimensionT, class T>
int DoIt( int argc, char *argv[] )
{

  PARSE_ARGS;

  enum VerboseLevelEnum {SILENT, STANDARD, VERBOSE};
  VerboseLevelEnum verbosity = STANDARD;

  typedef typename itk::OrientedImage<T, DimensionT> ImageType;

  typedef typename itk::ImageToImageRegistrationHelper< ImageType >  RegerType;

  typename RegerType::Pointer reger = RegerType::New();

  reger->SetReportProgress( true );

  if (verbosity >= STANDARD)
    {
    std::cout << "### Loading fixed image...";
    }
  reger->LoadFixedImage( fixedImage );
  if (verbosity >= STANDARD)
    {
    std::cout << "### DONE" << std::endl;
    }

  if (verbosity >= STANDARD)
    {
    std::cout << "### Loading moving image...";
    }
  reger->LoadMovingImage( movingImage );
  if (verbosity >= STANDARD)
    {
    std::cout << "### DONE" << std::endl;
    }

  /*
  if( loadParameters.size() > 1)
    {
    if (verbosity >= STANDARD)
      {
      std::cout << "### Loading parameters...";
      }
    reger->LoadParameters( loadParameters );
    if (verbosity >= STANDARD)
      {
      std::cout << "### DONE" << std::endl;
      }
    }
  */

  if( loadTransform.size() > 1 )
    {
    if (verbosity >= STANDARD)
      {
      std::cout << "### Loading transform...";
      }
    reger->LoadTransform( loadTransform );
    if (verbosity >= STANDARD)
      {
      std::cout << "### DONE" << std::endl;
      }
    }

  if (verbosity >= STANDARD)
    {
    std::cout << "### Initialization: CentersOfMass" << std::endl;
    }
  reger->SetInitialMethodEnum( RegerType::INIT_WITH_CENTERS_OF_MASS );

  if (verbosity >= STANDARD)
    {
    std::cout << "### Registration: PipelineRigid" << std::endl;
    }
  reger->SetEnableInitialRegistration( true );
  reger->SetEnableRigidRegistration( true );
  reger->SetEnableAffineRegistration( false );
  reger->SetEnableBSplineRegistration( false );

  if (verbosity >= STANDARD)
    {
    std::cout << "### Metric: MattesMutualInformation" << std::endl;
    }
  reger->SetRigidMetricMethodEnum( RegerType
                                   ::OptimizedRegistrationMethodType
                                   ::MATTES_MI_METRIC );

  reger->SetSampleFromOverlap( true );
  if (verbosity >= STANDARD)
    {
    std::cout << "### sampleFromOverlap: " << true << std::endl;
    }

  reger->SetSampleIntensityPortion( 0 );
  if (verbosity >= STANDARD)
    {
    std::cout << "### sampleIntensityPortion: " << 0 << std::endl;
    }


  reger->SetMinimizeMemory( false );
  if (verbosity >= STANDARD)
    {
    std::cout << "### MinimizeMemory: " << false << std::endl;
    }

  //reger->SetRandomNumberSeed( randomNumberSeed );

  reger->SetRigidMaxIterations( 50 );
  if (verbosity >= STANDARD)
    {
    std::cout << "### RigidMaxIterations: " << 50
              << std::endl;
    }

  reger->SetRigidSamplingRatio( .1 );
  if (verbosity >= STANDARD)
    {
    std::cout << "### RigidSamplingRatio: " << .1
              << std::endl;
    }

  reger->SetRigidInterpolationMethodEnum( RegerType
                                          ::OptimizedRegistrationMethodType
                                          ::LINEAR_INTERPOLATION );
  if (verbosity >= STANDARD)
    {
    std::cout << "### RigidInterpolationMethod: " << "linear"
              << std::endl;
    }

  reger->SetExpectedOffsetPixelMagnitude( expectedOffset );
  if (verbosity >= STANDARD)
    {
    std::cout << "### ExpectedOffsetPixelMagnitude: " << expectedOffset 
              << std::endl;
    }

  reger->SetExpectedRotationMagnitude( expectedRotation );
  if (verbosity >= STANDARD)
    {
    std::cout << "### ExpectedRotationMagnitude: " << expectedRotation 
              << std::endl;
    }

  try
    {
    if (verbosity >= STANDARD)
      {
      std::cout << "### Starting registration..." << std::endl;
      }
    reger->Update();
    }
  catch( itk::ExceptionObject &excep)
    {
    std::cerr << "Exception caught during helper class registration." 
              << excep << std::endl;
    std::cerr << "Current Matrix Transform = " << std::endl; 
    reger->GetCurrentMatrixTransform()->Print(std::cerr , 2);
    return EXIT_FAILURE;
    }
  catch( ... )
    {
    std::cerr << "Uncaught exception during helper class registration." 
              << std::endl;
    return EXIT_FAILURE;
    }

  if( resampledImage.size() > 1 )
    {
    if (verbosity >= STANDARD)
      {
      std::cout << "### Resampling..." << std::endl;
      }
    typename ImageType::ConstPointer resultImage;
    try
      {
        resultImage = reger->ResampleImage( RegerType
                                           ::OptimizedRegistrationMethodType
                                           ::LINEAR_INTERPOLATION );
      }
    catch( itk::ExceptionObject &excep)
      {
      std::cerr << "Exception caught during helper class resampling." 
                << excep << std::endl;
      std::cerr << "Current Matrix Transform = " << std::endl; 
      reger->GetCurrentMatrixTransform()->Print(std::cerr , 2);
      return EXIT_FAILURE;
      }
    catch( ... )
      {
      std::cerr << "Uncaught exception during helper class resampling." 
                << std::endl;
      return EXIT_FAILURE;
      }

    try
      {
      reger->SaveImage( resampledImage, resultImage );
      }
    catch( itk::ExceptionObject &excep)
      {
      std::cerr << "Exception caught during helper class resampled image saving." 
                << excep << std::endl;
      return EXIT_FAILURE;
      }
    catch( ... )
      {
      std::cerr << "Uncaught exception during helper class resampled image saving." << std::endl;
      return EXIT_FAILURE;
      }
    }
    
  if( saveTransform.size() > 1 )
    {
    try
      {
      reger->SaveTransform( saveTransform );
      }
    catch( itk::ExceptionObject &excep)
      {
      std::cerr << "Exception caught during helper class transform saving." 
                << excep << std::endl;
      return EXIT_FAILURE;
      }
    catch( ... )
      {
      std::cerr << "Uncaught exception during helper class saving." << std::endl;
      return EXIT_FAILURE;
      }
    }
  
  return EXIT_SUCCESS;
}

} // end of anonymous namespace


int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  unsigned int fixedDimensions = 0;
  itk::ImageIOBase::IOPixelType fixedPixelType;
  itk::ImageIOBase::IOComponentType fixedComponentType;
  unsigned int movingDimensions = 0;
  itk::ImageIOBase::IOPixelType movingPixelType;
  itk::ImageIOBase::IOComponentType movingComponentType;
  unsigned int dimensions = 0;
  itk::ImageIOBase::IOComponentType componentType;
 
  try
    {
    GetImageType( fixedImage, fixedPixelType, fixedComponentType, fixedDimensions ); 
    GetImageType( movingImage, movingPixelType, movingComponentType, movingDimensions ); 
    dimensions = fixedDimensions;
    if( movingDimensions > dimensions )
      {
      dimensions = movingDimensions;
      }
    componentType = fixedComponentType;
    if( movingComponentType > componentType )
      {
      componentType = movingComponentType;
      }
    if( dimensions < 2 || dimensions > 3 )
      {
      std::cerr << "ERROR: Only 2 and 3 dimensional images supported."
                << std::endl;
      return EXIT_FAILURE;
      }

    switch( componentType )
      {
      case itk::ImageIOBase::UCHAR:
#ifdef SUPPORT_2D_IMAGES
        if(dimensions == 2)
          {
          return DoIt<2, unsigned char>( argc, argv );
          }
        else
#endif
          {
          return DoIt<3, unsigned char>( argc, argv );
          }
        break;
      case itk::ImageIOBase::CHAR:
#ifdef SUPPORT_2D_IMAGES
        if(dimensions == 2)
          {
          return DoIt<2, char>( argc, argv );
          }
        else
#endif
          {
          return DoIt<3, char>( argc, argv );
          }
        break;
      case itk::ImageIOBase::SHORT:
#ifdef SUPPORT_2D_IMAGES
        if(dimensions == 2)
          {
          return DoIt<2, short>( argc, argv );
          }
        else
#endif
          {
          return DoIt<3, short>( argc, argv );
          }
        break;
      case itk::ImageIOBase::USHORT:
#ifdef SUPPORT_2D_IMAGES
        if(dimensions == 2)
          {
          return DoIt<2, unsigned short>( argc, argv );
          }
        else
#endif
          {
          return DoIt<3, unsigned short>( argc, argv );
          }
        break;
      case itk::ImageIOBase::UINT:
      case itk::ImageIOBase::INT:
      case itk::ImageIOBase::ULONG:
      case itk::ImageIOBase::LONG:
      case itk::ImageIOBase::FLOAT:
      case itk::ImageIOBase::DOUBLE:
#ifdef SUPPORT_2D_IMAGES
        if(dimensions == 2)
          {
          return DoIt<2, float>( argc, argv );
          }
        else
#endif
          {
          return DoIt<3, float>( argc, argv );
          }
        break;
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
        std::cerr << "ERROR: unknown component type" << std::endl;
        return EXIT_FAILURE;
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

