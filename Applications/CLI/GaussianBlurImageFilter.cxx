#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkSmoothingRecursiveGaussianImageFilter.h"

#include "GaussianBlurImageFilterCLP.h"

namespace itk
{
  // Description:
  // Get the PixelType and ComponentType from fileName
  void GetImageType (std::string fileName,
                     ImageIOBase::IOPixelType &pixelType,
                     ImageIOBase::IOComponentType &componentType)
    {
      typedef itk::Image<unsigned char, 3> ImageType;
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
                      std::vector<ImageIOBase::IOPixelType> &pixelTypes,
                      std::vector<ImageIOBase::IOComponentType> &componentTypes)
    {
    pixelTypes.clear();
    componentTypes.clear();

    // For each file, find the pixel and component type
    for (std::vector<std::string>::size_type i = 0; i < fileNames.size(); i++)
      {
      ImageIOBase::IOPixelType pixelType;
      ImageIOBase::IOComponentType componentType;

      GetImageType (fileNames[i],
                    pixelType,
                    componentType);
      pixelTypes.push_back(pixelType);  
      componentTypes.push_back(componentType);  
      }
    }
  
} // end namespace itk



template<class T> int DoIt( int argc, char * argv[], T )
{
  PARSE_ARGS;

  typedef    T       InputPixelType;
  typedef    T       OutputPixelType;

  typedef itk::Image< InputPixelType,  3 >   InputImageType;
  typedef itk::Image< OutputPixelType, 3 >   OutputImageType;

  typedef itk::ImageFileReader< InputImageType >  ReaderType;
  typedef itk::ImageFileWriter< OutputImageType > WriterType;

  typedef itk::SmoothingRecursiveGaussianImageFilter<
    InputImageType, OutputImageType >  FilterType;

  typename ReaderType::Pointer reader = ReaderType::New();
  
  reader->SetFileName( inputVolume.c_str() );

  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInput( reader->GetOutput() );
  filter->SetSigma( sigma );

  typename WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( outputVolume.c_str() );
  writer->SetInput( filter->GetOutput() );
  writer->Update();

  return EXIT_SUCCESS;
}

int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    itk::GetImageType (inputVolume, pixelType, componentType);

    // This filter handles all types on input, but only produces
    // signed types
    switch (componentType)
      {
      case itk::ImageIOBase::UCHAR:
      case itk::ImageIOBase::CHAR:
        return DoIt( argc, argv, static_cast<char>(0));
        break;
      case itk::ImageIOBase::USHORT:
      case itk::ImageIOBase::SHORT:
        return DoIt( argc, argv, static_cast<short>(0));
        break;
      case itk::ImageIOBase::UINT:
      case itk::ImageIOBase::INT:
        return DoIt( argc, argv, static_cast<int>(0));
        break;
      case itk::ImageIOBase::ULONG:
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
