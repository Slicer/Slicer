#include <iostream>
#include "itkSimpleDiffeomorphicRegistration.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include <string>
#include <itkWarpImageFilter.h>

int main(int argc, char **argv)
{
  typedef itk::Image< float, 3 >                   ImageType;
  typedef itk::Image< itk::Vector< float, 3 >, 3 > TDeformationField;
  typedef itk::ImageFileReader< ImageType >        ReadType;

  ReadType::Pointer FileReader1 = ReadType::New();
  ReadType::Pointer FileReader2 = ReadType::New();
  ReadType::Pointer FileReader3 = ReadType::New();
  if ( argc != 4 )
    {
    std::cout << "ERROR:  Wrong number of arguments given!" << std::endl;
    return -1;
    }
  FileReader1->SetFileName(argv[1]);
  FileReader2->SetFileName(argv[2]);

  FileReader1->Update();
  FileReader2->Update();

  itkSimpleDiffeomorphicRegistration *m_Registor =
    new itkSimpleDiffeomorphicRegistration;
  m_Registor->SetFixedImage( FileReader1->GetOutput() );
  m_Registor->SetMovingImage( FileReader2->GetOutput() );
  m_Registor->SetDeformedImageName(argv[3]);
  m_Registor->Update();
  return 0;
  //  delete m_Registor;
}
