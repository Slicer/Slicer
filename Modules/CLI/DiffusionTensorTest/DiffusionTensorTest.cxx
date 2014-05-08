#include "itkImageFileWriter.h"
#include "itkDiffusionTensor3D.h"
#include "itkMetaDataObject.h"

#include "itkPluginUtilities.h"

#include "DiffusionTensorTestCLP.h"

int main( int argc, char * argv[] )
{
  typedef itk::DiffusionTensor3D<float>         TensorType;
  typedef itk::Image<TensorType, 3>             TensorImageType;
  typedef itk::ImageFileReader<TensorImageType> TensorReaderType;
  typedef itk::ImageFileWriter<TensorImageType> TensorWriterType;

  PARSE_ARGS;

  TensorReaderType::Pointer reader = TensorReaderType::New();
  TensorWriterType::Pointer writer = TensorWriterType::New();

  reader->SetFileName( inputVolume.c_str() );
  reader->Update();

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  itk::GetImageType(inputVolume, pixelType, componentType);
  std::cout << "Plugin received PixelType: " << pixelType << std::endl;
  std::cout << "Plugin received ComponentType: " << componentType << std::endl;

  typedef itk::MetaDataDictionary DictionaryType;
  const DictionaryType & dictionary = reader->GetMetaDataDictionary();

  DictionaryType::ConstIterator itr = dictionary.Begin();
  DictionaryType::ConstIterator end = dictionary.End();

  while( itr != end )
    {
    if( itr->first == "DWMRI_b-value" )
      {
      std::string bValueString;
      itk::ExposeMetaData<std::string>(dictionary, itr->first, bValueString);
      std::cout << "DWMRI_b-value(string): " << bValueString << std::endl;
      }
    else if( itr->first.find("DWMRI_gradient") != std::string::npos )
      {
      std::string gradientValueString;
      itk::ExposeMetaData<std::string>(dictionary, itr->first, gradientValueString);
      std::cout << "DWMRI_gradient(string): " << gradientValueString << std::endl;
      }
    else if( itr->first.find("NRRD_measurement frame") != std::string::npos )
      {
      std::vector<std::vector<double> > measurementFrameValue(3);
      for( unsigned int i = 0; i < 3; i++ )
        {
        measurementFrameValue[i].resize(3);
        }

      itk::ExposeMetaData<std::vector<std::vector<double> > >(dictionary, itr->first, measurementFrameValue);

      std::cout << itr->first << ": " << std::endl;
      for( unsigned int i = 0; i < 3; i++ )
        {
        for( unsigned int j = 0; j < 3; j++ )
          {
          std::cout << measurementFrameValue[i][j] << " ";
          }
        std::cout << std::endl;
        }

      std::cout << std::endl;
      }
    else
      {
      std::cout << itr->first << std::endl;
      }

    ++itr;
    }

  writer->SetFileName( outputVolume.c_str() );

  writer->SetInput( reader->GetOutput() );

  writer->Update();

  return EXIT_SUCCESS;
}
