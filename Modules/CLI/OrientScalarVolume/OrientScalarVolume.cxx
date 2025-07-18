/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or https://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "itkImageFileWriter.h"

#include "itkOrientImageFilter.h"
#include "itkChangeInformationImageFilter.h"

#include "itkPluginUtilities.h"
#include "OrientScalarVolumeCLP.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
namespace
{
// Description: Map from string description to CoordinateOrientationCode
void CreateOrientationMap(std::map<std::string, itk::SpatialOrientationEnums::ValidCoordinateOrientations>& orientationMap)
{
  using CoordinateOrientationCode = itk::SpatialOrientationEnums::ValidCoordinateOrientations;

  orientationMap["Axial"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_RAI;
  orientationMap["Coronal"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_RSA;
  orientationMap["Sagittal"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_ASL;
  orientationMap["RIP"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_RIP;
  orientationMap["LIP"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_LIP;
  orientationMap["RSP"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_RSP;
  orientationMap["LSP"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_LSP;
  orientationMap["RIA"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_RIA;
  orientationMap["LIA"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_LIA;
  orientationMap["RSA"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_RSA;
  orientationMap["LSA"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_LSA;
  orientationMap["IRP"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_IRP;
  orientationMap["ILP"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_ILP;
  orientationMap["SRP"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_SRP;
  orientationMap["SLP"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_SLP;
  orientationMap["IRA"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_IRA;
  orientationMap["ILA"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_ILA;
  orientationMap["SRA"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_SRA;
  orientationMap["SLA"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_SLA;
  orientationMap["RPI"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_RPI;
  orientationMap["LPI"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_LPI;
  orientationMap["RAI"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_RAI;
  orientationMap["LAI"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_LAI;
  orientationMap["RPS"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_RPS;
  orientationMap["LPS"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_LPS;
  orientationMap["RAS"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_RAS;
  orientationMap["LAS"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_LAS;
  orientationMap["PRI"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_PRI;
  orientationMap["PLI"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_PLI;
  orientationMap["ARI"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_ARI;
  orientationMap["ALI"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_ALI;
  orientationMap["PRS"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_PRS;
  orientationMap["PLS"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_PLS;
  orientationMap["ARS"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_ARS;
  orientationMap["ALS"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_ALS;
  orientationMap["IPR"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_IPR;
  orientationMap["SPR"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_SPR;
  orientationMap["IAR"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_IAR;
  orientationMap["SAR"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_SAR;
  orientationMap["IPL"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_IPL;
  orientationMap["SPL"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_SPL;
  orientationMap["IAL"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_IAL;
  orientationMap["SAL"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_SAL;
  orientationMap["PIR"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_PIR;
  orientationMap["PSR"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_PSR;
  orientationMap["AIR"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_AIR;
  orientationMap["ASR"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_ASR;
  orientationMap["PIL"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_PIL;
  orientationMap["PSL"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_PSL;
  orientationMap["AIL"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_AIL;
  orientationMap["ASL"] = CoordinateOrientationCode::ITK_COORDINATE_ORIENTATION_ASL;
}

//
// Description: A templated procedure to execute the algorithm
template <class T>
int DoIt(int argc, char* argv[], T)
{

  PARSE_ARGS;

  typedef T PixelType;

  typedef itk::Image<PixelType, 3> ImageType;

  typedef itk::ImageFileReader<ImageType> ReaderType;
  typedef itk::ImageFileWriter<ImageType> WriterType;
  typedef itk::ChangeInformationImageFilter<ImageType> ChangeType;

  typedef itk::OrientImageFilter<ImageType, ImageType> FilterType;

  // Read the input volume
  const typename ReaderType::Pointer reader1 = ReaderType::New();
  const itk::PluginFilterWatcher watchReader1(reader1, "Read Volume 2", CLPProcessInformation);

  reader1->SetFileName(inputVolume1.c_str());
  reader1->Update();

  // change the orientation of the volume
  const typename FilterType::Pointer filter = FilterType::New();
  const itk::PluginFilterWatcher watchFilter(filter, "Orient image", CLPProcessInformation);

  std::map<std::string, itk::SpatialOrientationEnums::ValidCoordinateOrientations> orientationMap;
  CreateOrientationMap(orientationMap);
  const std::map<std::string, itk::SpatialOrientationEnums::ValidCoordinateOrientations>::iterator o = orientationMap.find(orientation);

  filter->SetInput(reader1->GetOutput());
  filter->UseImageDirectionOn();
  filter->SetDesiredCoordinateOrientation(o->second);
  filter->Update();

  // Compute a new origin for the volume such that the output aligns
  // with the input. Center the two volumes, then compute the offset
  // that moves the output center to the input center. Then appl that
  // offset to the output volume origin

  typename ImageType::PointType newOrigin;
  itk::AlignVolumeCenters<ImageType>(reader1->GetOutput(), filter->GetOutput(), newOrigin);

  // Now change the origin of the output volume
  const typename ChangeType::Pointer change = ChangeType::New();
  change->SetInput(filter->GetOutput());
  change->ChangeOriginOn();
  change->SetOutputOrigin(newOrigin);
  change->Update();

  const typename WriterType::Pointer writer = WriterType::New();
  const itk::PluginFilterWatcher watchWriter(writer, "Write Volume", CLPProcessInformation);
  writer->SetFileName(outputVolume.c_str());
  writer->SetInput(change->GetOutput());
  writer->Update();
  std::cout << "Input origin is: " << reader1->GetOutput()->GetOrigin() << std::endl;
  std::cout << "Output origin is: " << change->GetOutput()->GetOrigin() << std::endl;

  return EXIT_SUCCESS;
}

} // end of anonymous namespace

int main(int argc, char* argv[])
{
  PARSE_ARGS;

  itk::IOPixelEnum pixelType;
  itk::IOComponentEnum componentType;

  try
  {
    itk::GetImageType(inputVolume1, pixelType, componentType);

    // this filter produces the image of the same type as the input
    switch (componentType)
    {
      case itk::IOComponentEnum::UCHAR: return DoIt(argc, argv, static_cast<unsigned char>(0)); break;
      case itk::IOComponentEnum::CHAR: return DoIt(argc, argv, static_cast<char>(0)); break;
      case itk::IOComponentEnum::USHORT: return DoIt(argc, argv, static_cast<unsigned short>(0)); break;
      case itk::IOComponentEnum::SHORT: return DoIt(argc, argv, static_cast<short>(0)); break;
      case itk::IOComponentEnum::UINT: return DoIt(argc, argv, static_cast<unsigned int>(0)); break;
      case itk::IOComponentEnum::INT: return DoIt(argc, argv, static_cast<int>(0)); break;
      case itk::IOComponentEnum::ULONG: return DoIt(argc, argv, static_cast<unsigned long>(0)); break;
      case itk::IOComponentEnum::LONG: return DoIt(argc, argv, static_cast<long>(0)); break;
      case itk::IOComponentEnum::FLOAT: return DoIt(argc, argv, static_cast<float>(0)); break;
      case itk::IOComponentEnum::DOUBLE: return DoIt(argc, argv, static_cast<double>(0)); break;
      case itk::IOComponentEnum::UNKNOWNCOMPONENTTYPE:
      default: std::cout << "unknown component type" << std::endl; break;
    }
  }
  catch (itk::ExceptionObject& excep)
  {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
