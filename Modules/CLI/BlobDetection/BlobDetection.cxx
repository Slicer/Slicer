/*=========================================================================

This plugin is based on the Insight Toolkit example
ImageReadDicomWrite. It has been modified for GenerateCLP style
command line processing and additional features have been added.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkHessianToObjectnessMeasureImageFilter.h"
#include "itkMultiScaleHessianBasedMeasureImageFilter.h"

#include "BlobDetectionCLP.h"
#include "itkPluginUtilities.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

template <class Tin>
int DoIt( int argc, char * argv[])
{
  PARSE_ARGS;

  typedef Tin     InputPixelType;

  typedef itk::Image<InputPixelType, 3>                           InputImageType;
  typedef itk::Image<float, 3>                                    OutputImageType;

  typedef itk::ImageFileReader<InputImageType>                    ReaderType;
  typedef itk::ImageFileWriter<OutputImageType>                   WriterType;

  typename InputImageType::Pointer image;
  typename ReaderType::Pointer  reader = ReaderType::New();

  typedef itk::SymmetricSecondRankTensor<float, 3>                HessianPixelType;
  typedef itk::Image<HessianPixelType, 3>                         HessianImageType;
  typedef itk::HessianToObjectnessMeasureImageFilter<HessianImageType, OutputImageType> ObjectnessFilterType;
  typedef itk::MultiScaleHessianBasedMeasureImageFilter<InputImageType, HessianImageType, OutputImageType>  MultiScaleEnhancementFilterType;

  typename ReaderType::Pointer reader1 = ReaderType::New();
  itk::PluginFilterWatcher watchReader1(reader1, "Read Volume", CLPProcessInformation);

  reader1->SetFileName( InputVolume.c_str() );
  reader1->Update();

  typename ObjectnessFilterType::Pointer objectnessFilter =
      ObjectnessFilterType::New();

  objectnessFilter->SetScaleObjectnessMeasure(false);
  objectnessFilter->SetBrightObject(DetectBrightObjects);
  objectnessFilter->SetAlpha(alpha < 0.0 ? 0.5 : alpha);
  objectnessFilter->SetBeta(beta < 0.0 ? 0.5 : beta);
  objectnessFilter->SetGamma(gamma < 0.0 ? 5.0 : gamma);
  objectnessFilter->SetObjectDimension(0);

  typename InputImageType::SpacingType spacing = reader1->GetOutput()->GetSpacing();

  float sx = spacing[0];
  float sy = spacing[1];
  float sz = spacing[2];

  float minScale = sx < sy ? sx : sy;
  minScale = minScale < sz ? minScale : sz;

  if (minScale <= 0.0)
  {
    std::cerr << "!!! minimum image spacing can not be zero or negative !!!" << std::endl;
    return EXIT_FAILURE;
  }

  int autoSteps = (scale - minScale) / minScale;

  autoSteps = autoSteps < 1 ? 1 : autoSteps;

  typename MultiScaleEnhancementFilterType::Pointer multiScaleEnhancementFilter =
      MultiScaleEnhancementFilterType::New();
  itk::PluginFilterWatcher watchFilter(multiScaleEnhancementFilter,   "Processing", CLPProcessInformation);

  multiScaleEnhancementFilter->SetInput(reader1->GetOutput());
  multiScaleEnhancementFilter->SetSigmaMinimum(minScale);
  multiScaleEnhancementFilter->SetSigmaMaximum(scale < minScale ? minScale : scale);
  multiScaleEnhancementFilter->SetNumberOfSigmaSteps(steps <= 0 ? autoSteps : steps);
  multiScaleEnhancementFilter->SetGenerateScalesOutput(false);
  multiScaleEnhancementFilter->SetHessianToMeasureFilter(objectnessFilter);

  typename WriterType::Pointer writer = WriterType::New();
  itk::PluginFilterWatcher watchWriter(writer,
                                       "Write Volume",
                                       CLPProcessInformation);
  writer->SetFileName( OutputVolume.c_str() );
  writer->SetInput( multiScaleEnhancementFilter->GetOutput() );
  writer->Update();

  return EXIT_SUCCESS;
}

} // end of anonymous namespace

int main( int argc, char* argv[] )
{
  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    itk::GetImageType(InputVolume, pixelType, componentType);

    // This filter handles all types on input, but only produces
    // signed types

    switch( componentType )
      {
      case itk::ImageIOBase::UCHAR:
        return DoIt<unsigned char>( argc, argv);
        break;
      case itk::ImageIOBase::CHAR:
        return DoIt<char>( argc, argv);
        break;
      case itk::ImageIOBase::USHORT:
        return DoIt<unsigned short>( argc, argv);
        break;
      case itk::ImageIOBase::SHORT:
        return DoIt<short>( argc, argv);
        break;
      case itk::ImageIOBase::UINT:
        return DoIt<unsigned int>( argc, argv);
        break;
      case itk::ImageIOBase::INT:
        return DoIt<int>( argc, argv);
        break;
      case itk::ImageIOBase::ULONG:
        return DoIt<unsigned long>( argc, argv);
        break;
      case itk::ImageIOBase::LONG:
        return DoIt<long>( argc, argv);
        break;
      case itk::ImageIOBase::FLOAT:
        return DoIt<float>( argc, argv);
        break;
      case itk::ImageIOBase::DOUBLE:
        return DoIt<double>( argc, argv);
        break;
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
        std::cout << "unknown component type" << std::endl;
        break;
      }
    }
  catch( itk::ExceptionObject & excep )
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
