/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "itkOrientedImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkOrientImageFilter.h"
#include "itkChangeInformationImageFilter.h"
#include "itkSpatialOrientation.h"

#include "itkPluginUtilities.h"
#include "OrientImageCLP.h"

//
// Description: Map from string description to SpatialOrientation
void CreateOrientationMap(
  std::map<std::string,itk::SpatialOrientation::ValidCoordinateOrientationFlags> &orientationMap)
{
  orientationMap["Axial"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI;
  orientationMap["Coronal"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RSA;
  orientationMap["Sagittal"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ASL;
  orientationMap["RIP"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RIP;
  orientationMap["LIP"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LIP;
  orientationMap["RSP"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RSP;
  orientationMap["LSP"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LSP;
  orientationMap["RIA"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RIA;
  orientationMap["LIA"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LIA;
  orientationMap["RSA"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RSA;
  orientationMap["LSA"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LSA;
  orientationMap["IRP"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IRP;
  orientationMap["ILP"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ILP;
  orientationMap["SRP"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SRP;
  orientationMap["SLP"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SLP;
  orientationMap["IRA"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IRA;
  orientationMap["ILA"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ILA;
  orientationMap["SRA"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SRA;
  orientationMap["SLA"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SLA;
  orientationMap["RPI"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RPI;
  orientationMap["LPI"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LPI;
  orientationMap["RAI"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI;
  orientationMap["LAI"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LAI;
  orientationMap["RPS"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RPS;
  orientationMap["LPS"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LPS;
  orientationMap["RAS"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAS;
  orientationMap["LAS"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_LAS;
  orientationMap["PRI"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PRI;
  orientationMap["PLI"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PLI;
  orientationMap["ARI"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ARI;
  orientationMap["ALI"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ALI;
  orientationMap["PRS"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PRS;
  orientationMap["PLS"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PLS;
  orientationMap["ARS"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ARS;
  orientationMap["ALS"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ALS;
  orientationMap["IPR"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IPR;
  orientationMap["SPR"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SPR;
  orientationMap["IAR"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IAR;
  orientationMap["SAR"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SAR;
  orientationMap["IPL"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IPL;
  orientationMap["SPL"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SPL;
  orientationMap["IAL"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_IAL;
  orientationMap["SAL"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_SAL;
  orientationMap["PIR"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PIR;
  orientationMap["PSR"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PSR;
  orientationMap["AIR"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_AIR;
  orientationMap["ASR"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ASR;
  orientationMap["PIL"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PIL;
  orientationMap["PSL"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_PSL;
  orientationMap["AIL"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_AIL;
  orientationMap["ASL"] = itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ASL;
}

//
// Description: A templated procedure to execute the algorithm
template<class T> int DoIt( int argc, char * argv[], T )
{

  PARSE_ARGS;

  typedef    T       PixelType;

  typedef itk::OrientedImage< PixelType,  3 >   ImageType;

  typedef itk::ImageFileReader< ImageType >  ReaderType;
  typedef itk::ImageFileWriter< ImageType > WriterType;
  typedef itk::ChangeInformationImageFilter< ImageType > ChangeType;

  typedef itk::OrientImageFilter<
    ImageType, ImageType >  FilterType;

  // Read the input volume
  typename ReaderType::Pointer reader1 = ReaderType::New();
  itk::PluginFilterWatcher watchReader1(reader1, "Read Volume 2",
                                        CLPProcessInformation);
  
  reader1->SetFileName( inputVolume1.c_str() );
  reader1->Update();

  // change the orientation of the volume
  typename FilterType::Pointer filter = FilterType::New();
  itk::PluginFilterWatcher watchFilter(filter,
                                       "Orient image",
                                       CLPProcessInformation);

  std::map<std::string,itk::SpatialOrientation::ValidCoordinateOrientationFlags> orientationMap;
  CreateOrientationMap(orientationMap);
  std::map<std::string, itk::SpatialOrientation::ValidCoordinateOrientationFlags>::iterator o = orientationMap.find(orientation);
  
  filter->SetInput( reader1->GetOutput() );
  filter->UseImageDirectionOn();
  filter->SetDesiredCoordinateOrientation((*o).second);
  filter->Update();

  // Compute a new origin for the volume such that the output aligns
  // with the input. Center the two volumes, then compute the offset
  // that moves the output center to the input center. Then appl that
  // offset to the output volume origin

  typename ImageType::PointType newOrigin;
  itk::AlignVolumeCenters<ImageType>(reader1->GetOutput(), filter->GetOutput(), newOrigin);

  // Now change the origin of the output volume
  typename ChangeType::Pointer change = ChangeType::New();
  change->SetInput( filter->GetOutput() );
  change->ChangeOriginOn();
  change->SetOutputOrigin(newOrigin);
  change->Update();

  typename WriterType::Pointer writer = WriterType::New();
  itk::PluginFilterWatcher watchWriter(writer,
                                       "Write Volume",
                                       CLPProcessInformation);
  writer->SetFileName( outputVolume.c_str() );
  writer->SetInput( change->GetOutput() );
  writer->Update();
  std::cout << "Input origin is: " << reader1->GetOutput()->GetOrigin() << std::endl;
  std::cout << "Output origin is: " << change->GetOutput()->GetOrigin() <<
  std::endl;

  return EXIT_SUCCESS;
}

int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    itk::GetImageType (inputVolume1, pixelType, componentType);

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
