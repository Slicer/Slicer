#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDiffusionTensor3D.h"

#include "itkPluginFilterWatcher.h"



#include "DiffusionTensorTestCLP.h"

int main( int argc, char * argv[] )
{
  typedef itk::DiffusionTensor3D<float> TensorType;
  typedef itk::Image<TensorType, 3> TensorImageType;
  typedef itk::ImageFileReader<TensorImageType> TensorReaderType;
  typedef itk::ImageFileWriter<TensorImageType> TensorWriterType;

  PARSE_ARGS;

  TensorReaderType::Pointer reader = TensorReaderType::New();
  TensorWriterType::Pointer writer = TensorWriterType::New();

  reader->SetFileName( inputVolume.c_str() );
  writer->SetFileName( outputVolume.c_str() );

  writer->SetInput( reader->GetOutput() );

  writer->Update();
}
