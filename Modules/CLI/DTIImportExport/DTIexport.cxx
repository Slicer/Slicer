#include <iostream>
#include "DTIexportCLP.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDiffusionTensor3D.h"

int main(int argc, char * argv [])
{

  PARSE_ARGS;


  typedef itk::DiffusionTensor3D<float> PixelType;
  typedef itk::Image< PixelType, 3 >   ImageType;
  typedef itk::ImageFileReader< ImageType  >  ReaderType;
  typedef itk::ImageFileWriter< ImageType >  WriterType;

  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  ReaderType::Pointer readerImport = ReaderType::New();
  WriterType::Pointer writerImport = WriterType::New();

  ReaderType::Pointer readerExport = ReaderType::New();
  WriterType::Pointer writerExport = WriterType::New();

  readerExport->SetFileName(inputTensor);
  writerExport->SetFileName(outputFile);

  try
    {
      writerExport->SetInput(readerExport->GetOutput());
      // writerExport->UseCompressionOn();
      writerExport->Update();
      }
  catch (itk::ExceptionObject &excep)
    {
      std::cerr << excep << std::endl;
      return EXIT_FAILURE;
    }

  std::cout << "DTI Volume successfully written on disk" << std::endl;
  return EXIT_SUCCESS;
}
