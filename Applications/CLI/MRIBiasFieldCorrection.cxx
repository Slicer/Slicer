#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"

#include "itkMRIBiasFieldCorrectionFilter.h"
#include "itkBinaryThresholdImageFilter.h"

#include "itkPluginUtilities.h"
#include "MRIBiasFieldCorrectionCLP.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()

namespace {

template<class T> int DoIt( int argc, char * argv[], T )
{

  PARSE_ARGS;

  typedef    float   InputPixelType;
  typedef    T       OutputPixelType;

  typedef itk::Image< InputPixelType,  3 >   InputImageType;
  typedef itk::Image< OutputPixelType, 3 >   OutputImageType;

  typedef itk::ImageFileReader< InputImageType >  ReaderType;
  typedef itk::ImageFileWriter< OutputImageType > WriterType;

  typename ReaderType::Pointer reader = ReaderType::New();
    itk::PluginFilterWatcher watchReader(reader, "Read Volume",
      CLPProcessInformation);

  typename WriterType::Pointer writer = WriterType::New();

  itk::Array<double> m_tissue(2);

  reader->SetFileName( inputVolume.c_str() );
  writer->SetFileName( outputVolume.c_str() );

  if (useMask == "yes")
  {
  typedef itk::MRIBiasFieldCorrectionFilter<
    InputImageType, InputImageType, InputImageType>  FilterType;
  typename FilterType::Pointer filter = FilterType::New();

  filter->SetInput( reader->GetOutput() );
  filter->SetInputMask( reader->GetOutput() );
  filter->SetOutputMask( reader->GetOutput() );
  m_tissue(0) = meanTissue1;
  m_tissue(1) = meanTissue2;
  filter->SetTissueClassStatistics(m_tissue,m_tissue);

  typedef itk::CastImageFilter<InputImageType, OutputImageType> CastType;
  typename CastType::Pointer cast = CastType::New();
  cast->SetInput( filter->GetOutput());

  writer->SetInput( cast->GetOutput() );
  writer->Update();
  }
  else
  {
  typedef itk::MRIBiasFieldCorrectionFilter<
    InputImageType, InputImageType, InputImageType >  FilterType;
  typename FilterType::Pointer filter = FilterType::New();

  filter->SetInput( reader->GetOutput() );
  m_tissue(0) = meanTissue1;
  m_tissue(1) = meanTissue2;
  filter->SetTissueClassStatistics(m_tissue,m_tissue);

  typedef itk::CastImageFilter<InputImageType, OutputImageType> CastType;
  typename CastType::Pointer cast = CastType::New();
  cast->SetInput( filter->GetOutput());

  writer->SetInput( cast->GetOutput() );
  writer->Update();
  }

  return EXIT_SUCCESS;
}

} // end of anonymous namespace


int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    itk::GetImageType (inputVolume, pixelType, componentType);

    // This filter handles all types

    switch (componentType)
      {
      case itk::ImageIOBase::UCHAR:
        return DoIt( argc, argv, static_cast<unsigned char>(0));
        break;
      case itk::ImageIOBase::CHAR:
        return DoIt( argc, argv, static_cast<char>(0));
        break;
      case itk::ImageIOBase::USHORT:
        return DoIt( argc, argv, static_cast<unsigned short>(0));
        break;
      case itk::ImageIOBase::SHORT:
        return DoIt( argc, argv, static_cast<short>(0));
        break;
      case itk::ImageIOBase::UINT:
        return DoIt( argc, argv, static_cast<unsigned int>(0));
        break;
      case itk::ImageIOBase::INT:
        return DoIt( argc, argv, static_cast<int>(0));
        break;
      case itk::ImageIOBase::ULONG:
        return DoIt( argc, argv, static_cast<unsigned long>(0));
        break;
      case itk::ImageIOBase::LONG:
        return DoIt( argc, argv, static_cast<long>(0));
        break;
      case itk::ImageIOBase::FLOAT:
        return DoIt( argc, argv, static_cast<float>(0));
        break;
      case itk::ImageIOBase::DOUBLE:
        return DoIt( argc, argv, static_cast<double>(0));
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
