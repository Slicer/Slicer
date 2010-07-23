#include <itksys/SystemTools.hxx>
#include <iostream>
#include <stdlib.h>

#include <itkImage.h>
#include <itkAffineTransform.h>
#include <itkLinearInterpolateImageFunction.h>
// #include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkResampleImageFilter.h>
#include <itkTransformFileReader.h>
#include <itkTransformFileWriter.h>
#include "itkIO.h"
#include "ReadMask.h"
#include <math.h>

#include "itkTestMain.h"

#ifndef M_PI
#  define M_PI 3.1415926
#endif
#ifndef M_TWOPI
#  define M_TWOPI ( 2.0 * M_PI )
#endif
inline double DEGREES(double x)
{
  double rval = x * ( M_PI / 180 );

  return rval;
}

static bool keepOutputs(false);

//
// typedefs
typedef itk::Image< unsigned char, 3 > ImageType;
typedef itk::AffineTransform< double,
                              3 >                        AffineTransformType;
typedef itk::LinearInterpolateImageFunction< ImageType, double > InterpolatorType;
// typedef itk::NearestNeighborInterpolateImageFunction<ImageType,double>
//  InterpolatorType;
typedef itk::ResampleImageFilter< ImageType,
                                  ImageType >         ResampleImageFilter;

//
// given two images, compute the RMS error between them.
static int
LocalCompareImages(const std::string & imageAname, const std::string & imageBname)
{
  return RegressionTestImage(imageAname.c_str(), imageBname.c_str(), 1, 0, 0, 0);
}

//
// apply an affine transform to an image, and
// return the transformed image
static ImageType::Pointer
Resample(ImageType::Pointer & inputImage,
         AffineTransformType::Pointer & transform)
{
  InterpolatorType::Pointer interp = InterpolatorType::New();

  interp->SetInputImage(inputImage);

  ResampleImageFilter::Pointer resample = ResampleImageFilter::New();
  resample->SetInput(inputImage);
  resample->SetTransform(transform);
  resample->SetInterpolator(interp);
  resample->SetOutputParametersFromImage(inputImage);
  resample->Update();
  ImageType::Pointer returnval = resample->GetOutput();
  return returnval;
}

static void WriteTransform(AffineTransformType::Pointer & MyTransform,
                           const std::string & TransformFilename)
{
  typedef itk::TransformFileWriter TransformWriterType;
  TransformWriterType::Pointer affineWriter =  TransformWriterType::New();

  affineWriter->SetFileName( TransformFilename.c_str() );
  affineWriter->SetInput(MyTransform);
  affineWriter->Update();
}

//
// build the command line and invoke BRAINSFit
// to register the image, and write out the registered
// image.
static void
Register(const std::string & registrationProgram,
         const std::string & fixedVolumeName,
         const std::string & fixedBinaryVolumeName,
         const std::string & movingVolumeName,
         const std::string & movingBinaryVolumeName,
         const std::string & outputVolumeName,
         const std::string & outputTransformName,
         const std::string & transformType,
         AffineTransformType::InputPointType & CenterOfRotation,
         AffineTransformType::OutputPointType & TransformedCenter,
         bool OriginsProvided,
         const std::string & InitTransformName)
{
  std::string cmdLine(registrationProgram);

  cmdLine +=  // " --forceImageOrientation Axial"
             " --numberOfIterations 2500"
             " --numberOfSamples 24000" // test is on small image, speed things
                                        // up
             " --translationScale 250"
             " --minimumStepSize 0.01"
             " --outputVolumePixelType uchar";
  cmdLine += " --transformType ";
  cmdLine += transformType;

  if ( OriginsProvided )
    {
    cmdLine += " --explicitOrigins";
    char buf[1024];
    sprintf(buf, " --fixedVolumeOrigin %f,%f,%f --movingVolumeOrigin %f,%f,%f ",
            CenterOfRotation[0], CenterOfRotation[1], CenterOfRotation[2],
            TransformedCenter[0], TransformedCenter[1], TransformedCenter[2]);
    cmdLine += buf;
    }
  if ( InitTransformName != "" )
    {
    cmdLine += " --initialTransform ";
    cmdLine += InitTransformName;
    }
  cmdLine += " --fixedVolume ";
  cmdLine += fixedVolumeName;
  if ( fixedBinaryVolumeName != "" )
    {
    cmdLine += " --fixedBinaryVolume ";
    cmdLine += fixedBinaryVolumeName;
    }
  cmdLine += " --movingVolume ";
  cmdLine += movingVolumeName;
  if ( movingBinaryVolumeName != "" )
    {
    cmdLine += " --movingBinaryVolume ";
    cmdLine += movingBinaryVolumeName;
    }
  cmdLine += " --outputVolume ";
  cmdLine += outputVolumeName;
  cmdLine += " --outputTransform ";
  cmdLine += outputTransformName;
  std::cerr << "Running registration, command = \""
            << cmdLine << "\"" << std::endl;
  system( cmdLine.c_str() );
}

//
// put a prefix on the filename part of a filename
static const std::string
PrependName(const std::string & name, const std::string & prefix)
{
  std::string rval(prefix);

  rval += itksys::SystemTools::GetFilenameName(name);
  return rval;
}

//
// generate a rotation transform
static AffineTransformType::Pointer
SimpleRotation(const double imageCenter[3])
{
  AffineTransformType::Pointer          transform = AffineTransformType::New();
  AffineTransformType::OutputVectorType translation1;

  translation1[0] = -imageCenter[0];
  translation1[1] = -imageCenter[1];
  translation1[2] = -imageCenter[2];
  transform->Translate(translation1);

  AffineTransformType::OutputVectorType rotationAxis;
  rotationAxis[0] = 0.0; rotationAxis[1] = 0.0; rotationAxis[2] = 1.0;
  transform->Rotate3D( rotationAxis, DEGREES(6.0) );
  rotationAxis[0] = 1.0;                        rotationAxis[2] = 0.0;
  transform->Rotate3D( rotationAxis, DEGREES(-5.0) );
  rotationAxis[0] = 0.0; rotationAxis[1] = 1.0;
  transform->Rotate3D( rotationAxis, DEGREES(4.0) );
  AffineTransformType::OutputVectorType translation2;
  translation2[0] = imageCenter[0];
  translation2[1] = imageCenter[1];
  translation2[2] = imageCenter[2];
  transform->Translate(translation2);
  return transform;
}

//
// generate a translation transform
static AffineTransformType::Pointer
Translate(const double xoffset, const double yoffset, const double zoffset)
{
  AffineTransformType::Pointer transform =
    AffineTransformType::New();
  AffineTransformType::OutputVectorType offset;

  offset[0] = xoffset;
  offset[1] = yoffset;
  offset[2] = zoffset;
  transform->Translate(offset);
  return transform;
}

//
// generate a scale transorm
static AffineTransformType::Pointer
Scale(const double xScale, const double yScale, const double zScale)
{
  AffineTransformType::Pointer transform =
    AffineTransformType::New();
  AffineTransformType::OutputVectorType scale;

  scale[0] = xScale;
  scale[1] = yScale;
  scale[2] = zScale;
  transform->Scale(scale);
  return transform;
}

int BRAINSFitTestDriver(int argc, char **argv)
{
  RegisterBrains2MaskFactory();
  if ( argc != 6 && argc != 7 )
    {
    std::cerr << "BRAINSFitTest "
                 "[-k] <test-class> <source-image> <source-mask> <work-directory> <path-to-RegistrationProgram>"
              << std::endl;
    std::cerr
    << "first argument required to be either [translation|rotation|scale]"
    << std::endl;
    exit(1);
    }
  if ( strcmp(argv[1], "-k") == 0 )
    {
    ++argv; --argc;
    keepOutputs = true;
    }

  std::string RegistrationClass(argv[1]);
  ++argv; --argc; // Reduce for Class
  if ( RegistrationClass == "translation" || RegistrationClass == "rotation"
       || RegistrationClass == "scale" )
    {
    std::cout << "Running with class= " << RegistrationClass << std::endl;
    }
  else
    {
    std::cerr
    << "first argument required to be either [translation|rotation|scale] : "
    << RegistrationClass << std::endl;
    exit(-1);
    }
  std::string startImageName( itksys::SystemTools::CollapseFullPath(argv[1]) );
  std::string startMaskName( itksys::SystemTools::CollapseFullPath(argv[2]) );

  itksys::SystemTools::ChangeDirectory(argv[3]);
  std::string registrationProgram( itksys::SystemTools::CollapseFullPath(
                                     argv[4]) );

  if ( !itksys::SystemTools::FileExists( registrationProgram.c_str() ) )
    {
    std::cerr << "Bad path to BRAINSFit: "
              << registrationProgram << std::endl;
    exit(2);
    }

  ImageType::DirectionType IdentityOrientation;
  IdentityOrientation.SetIdentity();
  // read input image
  ImageType::Pointer startImage =
    itkUtil::ReadImage< ImageType >(startImageName);

  if ( startImage.IsNull() )
    {
    std::cerr << "Can't read test image "
              << startImageName << std::endl;
    exit(3);
    }

  // read input mask
  ImageType::Pointer startMask =
    itkUtil::ReadImage< ImageType >(startMaskName);
  if ( startMask.IsNull() )
    {
    std::cerr << "Can't read test mask "
              << startMaskName << std::endl;
    exit(3);
    }

  //
  // set up a point to be used as an initializer
  // this is one I chose based on the test image,
  // and is at my best guess for the AC point
  AffineTransformType::InputPointType CenterOfRotation;
  CenterOfRotation[0] = 15;
  CenterOfRotation[1] = 36;
  CenterOfRotation[2] = 39;

  AffineTransformType::Pointer transform;

  ImageType::DirectionType Orientation = startImage->GetDirection();
  ImageType::SpacingType   spacing = startImage->GetSpacing();
  ImageType::PointType     origin = startImage->GetOrigin();
  ImageType::SizeType      size =
    startImage->GetLargestPossibleRegion().GetSize();

  unsigned    errors(0);
  std::string blank("");

  // test with translation
  if ( RegistrationClass == "translation" )
    {
    std::cerr << "==========Translation==========" << std::endl;
    transform = Translate(4.0, -3.0, 2.0);

    ImageType::Pointer translatedImage = Resample(startImage, transform);
    std::string        translatedImageName = PrependName(startImageName,
                                                         RegistrationClass + ".");
    itkUtil::WriteImage< ImageType >(translatedImage, translatedImageName);

    ImageType::Pointer translatedMask = Resample(startMask, transform);
    std::string        translatedMaskName = PrependName(startMaskName,
                                                        RegistrationClass + ".");
    itkUtil::WriteImage< ImageType >(translatedMask, translatedMaskName);

    std::string registrationTransformName = RegistrationClass + ".txt";
    std::string registeredImageName = PrependName(startImageName,
                                                  RegistrationClass + "Registered.");
    Register(registrationProgram,
             startImageName,
             startMaskName,
             translatedImageName,
             translatedMaskName,
             registeredImageName,
             registrationTransformName,
             std::string("Affine"),
             CenterOfRotation,
             CenterOfRotation,
             false,
             blank);

    // allow RMS error of 3
    errors += LocalCompareImages(startImageName, registeredImageName);
    if ( !keepOutputs )
      {
      itksys::SystemTools::RemoveFile( translatedImageName.c_str() );
      itksys::SystemTools::RemoveFile( registeredImageName.c_str() );
      itksys::SystemTools::RemoveFile( registrationTransformName.c_str() );
      }
    }

  std::string regTypes[4];
  regTypes[0] = "Affine";
  regTypes[1] = "Rigid";
  regTypes[2] = "ScaleVersor3D";
  regTypes[3] = "ScaleSkewVersor3D";

  // test with rotation
  if ( RegistrationClass == "rotation" )
    {
    std::cerr << "==========Rotation==========" << std::endl;
    double imageCenter[3];
    imageCenter[0] = origin[0] + spacing[0] * size[0] / 2.0;
    imageCenter[1] = origin[1] + spacing[1] * size[1] / 2.0;
    imageCenter[2] = origin[2] + spacing[1] * size[2] / 2.0;

    transform = SimpleRotation(imageCenter);

    ImageType::Pointer rotatedImage = Resample(startImage, transform);

    if ( rotatedImage.IsNull() )
      {
      std::cerr << "Rotate image failed"
                << std::endl;
      exit(4);
      }
    AffineTransformType::OutputPointType transformedCenter =
      transform->TransformPoint(CenterOfRotation);

    std::string rotatedImageName = PrependName(startImageName,
                                               RegistrationClass + ".");
    itkUtil::WriteImage< ImageType >(rotatedImage, rotatedImageName);
    if ( !itksys::SystemTools::FileExists( rotatedImageName.c_str() ) )
      {
      std::cerr << "Writing "
                << rotatedImageName << " failed."
                << std::endl;
      exit(5);
      }

    ImageType::Pointer rotatedMask = Resample(startMask, transform);
    std::string        rotatedMaskName = PrependName(startMaskName,
                                                     RegistrationClass + ".");
    itkUtil::WriteImage< ImageType >(rotatedMask, rotatedMaskName);
    if ( !itksys::SystemTools::FileExists( rotatedMaskName.c_str() ) )
      {
      std::cerr << "Writing "
                << rotatedMaskName << " failed."
                << std::endl;
      exit(5);
      }
    for ( unsigned i = 0; i < 4; i++ )
      {
      std::string registrationTransformName = regTypes[i] + RegistrationClass
                                              + ".txt";
      std::string registeredImageName = PrependName(startImageName,
                                                    RegistrationClass + "Registered.");
      for ( unsigned j = 0; j < 2; j++ )
        {
        std::string regRotName(regTypes[i]);
        if ( j == 0 )
          {
          std::cerr << "Using FixedImageCenter "
                    << CenterOfRotation << " and MovingImageCenter"
                    << transformedCenter << std::endl;
          regRotName += "wCenters.";
          }
        regRotName += RegistrationClass + ".";
        std::cerr << "Registration with";
        if ( j == 1 )
          {
          std::cerr << "out";
          }
        std::cerr << " masks. " << regTypes[i];
        std::cerr << std::endl;
        Register(registrationProgram,
                 startImageName,
                 j == 0 ? startMaskName : blank,
                 rotatedImageName,
                 j == 0 ? rotatedMaskName : blank,
                 registeredImageName,
                 registrationTransformName,
                 regTypes[i],
                 CenterOfRotation,
                 transformedCenter,
                 ( j == 0 ? true : false ),
                 blank);

        if ( !itksys::SystemTools::FileExists( registrationTransformName.c_str() ) )
          {
          std::cerr << "Registration failed" << std::endl;
          exit(6);
          }
        errors += LocalCompareImages(startImageName, registeredImageName);
        }
      if ( !keepOutputs )
        {
        itksys::SystemTools::RemoveFile( registeredImageName.c_str() );
        itksys::SystemTools::RemoveFile( registrationTransformName.c_str() );
        }
      }
    if ( !keepOutputs )
      {
      itksys::SystemTools::RemoveFile( rotatedImageName.c_str() );
      }
    }
  // scale
  if ( RegistrationClass == "scale" )
    {
    std::cerr << "==========Scale==========" << std::endl;
    transform = Scale(0.85, 0.80, 0.75);

    AffineTransformType::Pointer initializerTransform = Scale(0.8, 0.9, 0.85);
    std::string                  InitializerTransformName = RegistrationClass
                                                            + "Init.txt";
    WriteTransform(initializerTransform, InitializerTransformName);

    ImageType::Pointer scaledImage = Resample(startImage, transform);
    std::cerr << "Moving image origin: " << scaledImage->GetOrigin()
              << std::endl;
    std::string scaledImageName = PrependName(startImageName,
                                              RegistrationClass + ".");
    itkUtil::WriteImage< ImageType >(scaledImage, scaledImageName);

    ImageType::Pointer scaledMask = Resample(startMask, transform);
    std::string        scaledMaskName = PrependName(startMaskName,
                                                    RegistrationClass + ".");
    itkUtil::WriteImage< ImageType >(scaledMask, scaledMaskName);
    if ( !itksys::SystemTools::FileExists( scaledMaskName.c_str() ) )
      {
      std::cerr << "Writing "
                << scaledMaskName << " failed."
                << std::endl;
      exit(5);
      }

    // just test last two transform types
    for ( unsigned int i = 0; i < 4; i++ )
      {
      if ( ( i & 1 ) == 0 )
        {
        std::cerr << "Initializing Transform with "
                  << InitializerTransformName << std::endl;
        }
      std::string registrationTransformName = regTypes[i] + RegistrationClass
                                              + ".txt";
      std::string regScaleName = regTypes[i] + RegistrationClass + ".";
      std::string registeredImageName = PrependName(startImageName,
                                                    RegistrationClass + "Registered.");

      Register( registrationProgram,
                startImageName,
                startMaskName,
                scaledImageName,
                scaledMaskName,
                registeredImageName,
                registrationTransformName,
                std::string("Affine"),
                CenterOfRotation,
                CenterOfRotation,
                false,
                ( ( ( i & 1 ) == 0 ) ? InitializerTransformName : blank ) );

      errors += LocalCompareImages(startImageName, registeredImageName);
      if ( !keepOutputs )
        {
        itksys::SystemTools::RemoveFile( registeredImageName.c_str() );
        itksys::SystemTools::RemoveFile( registrationTransformName.c_str() );
        if ( i == 2 )
          {
          itksys::SystemTools::RemoveFile( InitializerTransformName.c_str() );
          }
        }
      }
    if ( !keepOutputs )
      {
      itksys::SystemTools::RemoveFile( scaledImageName.c_str() );
      }
    }
  exit(errors);
}

void RegisterTests()
{
  REGISTER_TEST(BRAINSFitTestDriver);
}
