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

#include "itkImageToImageRegistrationHelper.h"

// Description:
// Get the PixelType and ComponentType from fileName
void GetImageType (std::string fileName,
                   itk::ImageIOBase::IOPixelType &pixelType,
                   itk::ImageIOBase::IOComponentType &componentType)
{
  typedef itk::Image<short, 3> ImageType;
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

  typedef typename itk::OrientedImage<T, 3> ImageType;

  typedef typename itk::ImageToImageRegistrationHelper< ImageType >  RegerType;

  typename RegerType::Pointer reger = RegerType::New();

  reger->SetReportProgress( true );

  reger->LoadFixedImage( fixedImage );
  reger->LoadMovingImage( movingImage );

  if( loadParameters.size() > 1)
    {
    reger->LoadParameters( loadParameters );
    }

  if( loadTransform.size() > 1 )
    {
    reger->LoadTransform( loadTransform );
    }

  if( initialization == "None" )
    {
    reger->SetInitialMethodEnum( RegerType::INIT_WITH_NONE );
    reger->SetEnableInitialRegistration( false );
    }
  else if( initialization == "ImageCenters")
    {
    reger->SetInitialMethodEnum( RegerType::INIT_WITH_IMAGE_CENTERS );
    reger->SetEnableInitialRegistration( true );
    }
  else if( initialization == "SecondMoments")
    {
    reger->SetInitialMethodEnum( RegerType::INIT_WITH_SECOND_MOMENTS );
    reger->SetEnableInitialRegistration( true );
    }
  else //if( initialization == "CentersOfMass")
    {
    reger->SetInitialMethodEnum( RegerType::INIT_WITH_CENTERS_OF_MASS );
    reger->SetEnableInitialRegistration( true );
    }

  if( registration == "None" )
    {
    reger->SetEnableRigidRegistration( false );
    reger->SetEnableAffineRegistration( false );
    reger->SetEnableBSplineRegistration( false );
    }
  else if( registration == "Rigid" )
    {
    reger->SetEnableRigidRegistration( true );
    reger->SetEnableAffineRegistration( false );
    reger->SetEnableBSplineRegistration( false );
    }
  else if( registration == "Affine" )
    {
    reger->SetEnableRigidRegistration( true );
    reger->SetEnableAffineRegistration( true );
    reger->SetEnableBSplineRegistration( false );
    }
  else //if( registration == "BSpline" )
    {
    reger->SetEnableRigidRegistration( true );
    reger->SetEnableAffineRegistration( true );
    reger->SetEnableBSplineRegistration( true );
    }

  if( metric == "NormCorr" )
    {
    reger->SetRigidMetricMethodEnum( RegerType::OptimizedRegistrationMethodType::NORMALIZED_CORRELATION_METRIC );
    }
  else if( metric == "MeanSqrd" )
    {
    reger->SetRigidMetricMethodEnum( RegerType::OptimizedRegistrationMethodType::MEAN_SQUARED_ERROR_METRIC );
    }
  else // if( metric == "MattesMI" )
    {
    reger->SetRigidMetricMethodEnum( RegerType::OptimizedRegistrationMethodType::MATTES_MI_METRIC );
    }

  reger->SetMinimizeMemory( minimizeMemory );

  reger->SetRigidMaxIterations( rigidMaxIterations );
  reger->SetAffineMaxIterations( affineMaxIterations );
  reger->SetBSplineMaxIterations( bsplineMaxIterations );

  reger->SetRigidSamplingRatio( rigidSamplingRatio );
  reger->SetAffineSamplingRatio( affineSamplingRatio );
  reger->SetBSplineSamplingRatio( bsplineSamplingRatio );

  reger->SetExpectedOffsetPixelMagnitude( expectedOffset );
  reger->SetExpectedRotationMagnitude( expectedRotation );
  reger->SetExpectedScaleMagnitude( expectedScale );
  reger->SetExpectedSkewMagnitude( expectedSkew );

  reger->SetBSplineControlPointPixelSpacing( controlPointSpacing );

  reger->Update();

  typename ImageType::ConstPointer resultImage;
  if(useWindowedSinc)
    {
    resultImage = reger->ResampleImage( RegerType::OptimizedRegistrationMethodType::SINC_INTERPOLATION );
    reger->SaveImage( resampledImage, resultImage );
    }
  else
    {
    resultImage = reger->ResampleImage();
    reger->SaveImage( resampledImage, reger->ResampleImage() );
    }

  if(differenceImage.size() > 1)
    {
    typedef itk::OrientedImage< float, 3 > OutputImageType;
    typedef itk::SubtractImageFilter< ImageType, ImageType, OutputImageType > FilterType;
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

  return EXIT_SUCCESS;

}

int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType pixelType;
  itk::ImageIOBase::IOComponentType componentType;
 
  try
    {
    GetImageType (fixedImage, pixelType, componentType); 

    switch (componentType)
      {
      case itk::ImageIOBase::UCHAR:
        return DoIt<unsigned char>( argc, argv );
        break;
      case itk::ImageIOBase::CHAR:
        return DoIt<char>( argc, argv );
        break;
      case itk::ImageIOBase::USHORT:
        return DoIt<unsigned short>( argc, argv );
        break;
      case itk::ImageIOBase::SHORT:
        return DoIt<short>( argc, argv );
        break;
      case itk::ImageIOBase::UINT:
      case itk::ImageIOBase::INT:
      case itk::ImageIOBase::ULONG:
      case itk::ImageIOBase::LONG:
      case itk::ImageIOBase::FLOAT:
        return DoIt<float>( argc, argv );
        break;
      case itk::ImageIOBase::DOUBLE:
        return DoIt<double>( argc, argv );
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

