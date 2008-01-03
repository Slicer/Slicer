/*=========================================================================

Program:   ReconstructTensor
Module:    $RCSfile: GEDicom2NRRD.cxx,v $
Language:  C++
Date:      $Date: 2007/01/03 02:06:07 $
Version:   $Revision: 1.2 $

Copyright (c) General Electric Global Research. All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

*** 
This program converts Diffusion weighted MR images in GE Dicom format into
NRRD format.

Assumptions:

1) Space is always right-anterior-superior (scanner space or world space).
   This is the ABSOLUTE space in which all coordinates are represented.
   This is also the space for NRRD header.
2) Dicom data are arranged in volume interleaving order.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

#include "itkXMLFilterWatcher.h"

#include "itkNrrdImageIO.h"
#include "itkOrientedImage.h"
#include "itkImageSeriesReader.h"
#include "itkMetaDataDictionary.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"
#include "itkRecursiveGaussianImageFilter.h"


#include "itkImageFileWriter.h"
#include "itkRawImageIO.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkNumericSeriesFileNames.h"

#include "itkScalarImageToListAdaptor.h"
#include "itkListSampleToHistogramGenerator.h"

#include "itkImage.h"
#include "vnl/vnl_math.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix_fixed.h"
#include "vnl/algo/vnl_svd.h"

#include "itkVectorImage.h"
#include "gdcmDictSet.h"   // access to dictionary
#include "gdcmGlobal.h"   // access to dictionary


#include "GEDicom2NRRDCLP.h"

typedef vnl_vector_fixed<double, 3> VectorType;
typedef itk::Vector<float, 3> OutputVectorType;

int main(int argc, char* argv[])
{
  
  PARSE_ARGS;

  // add private dictionary
  gdcm::Global::GetDicts()->GetDefaultPubDict()->AddDict(PrivateDictionary.c_str());

  // check if the file name is valid
  std::string nhdrname = outputFileName;
  std::string dataname;
  {
    int i;
    i = nhdrname.find(".nhdr");
    if (i == std::string::npos)
      {
      std::cerr << "Output file must be a nrrd header file.\n";
      std::cerr << "Version:   $Revision: 1.2 $" << std::endl;
      return EXIT_FAILURE;
      }
    dataname = nhdrname.substr(0, i) + ".raw";
  }

  typedef short PixelValueType;
  typedef itk::OrientedImage< PixelValueType, 3 > VolumeType;
  typedef itk::ImageSeriesReader< VolumeType >
    ReaderType;
  typedef itk::GDCMImageIO
    ImageIOType;
  typedef itk::GDCMSeriesFileNames
    InputNamesGeneratorType;

//////////////////////////////////////////////////  
// 1) Read the input series as an array of slices
  ImageIOType::Pointer gdcmIO = ImageIOType::New();
  gdcmIO->LoadPrivateTagsOn();

  InputNamesGeneratorType::Pointer inputNames = InputNamesGeneratorType::New();
  inputNames->SetInputDirectory( inputDicom.c_str() );

  const ReaderType::FileNamesContainer & filenames = 
                            inputNames->GetInputFileNames();

  ReaderType::Pointer reader = ReaderType::New();

  reader->SetImageIO( gdcmIO );
  reader->SetFileNames( filenames );
  try
    {
    reader->Update();
    }
  catch (itk::ExceptionObject &excp)
    {
    std::cerr << "Exception thrown while reading the series" << std::endl;
    std::cerr << excp << std::endl;
    return EXIT_FAILURE;
    }
/////////////////////////////////////////////////////
// 2) Analyze the DICOM header to determine the 
//    number of gradient directions, gradient
//    vectors, and form volume based on these info

  ReaderType::DictionaryArrayRawPointer inputDict = reader->GetMetaDataDictionaryArray();

//   /// for debug. to check what tags have been exposed
//   itk::MetaDataDictionary & debugDict = gdcmIO->GetMetaDataDictionary();
//   std::vector<std::string> allKeys = debugDict.GetKeys();
//   for (int k = 0; k < allKeys.size(); k++)
//     {
//     std::cout << allKeys[k] << std::endl;
//     }

  
  int nSlice = inputDict->size();   
  std::string tag;

  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0028|0010", tag );
  int nRows = atoi( tag.c_str() );

  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0028|0011", tag );
  int nCols = atoi( tag.c_str() );

  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0028|0030", tag );
  float xRes;
  float yRes;
  sscanf( tag.c_str(), "%f\\%f", &xRes, &yRes );

  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0020|0032", tag );
  float xOrigin;
  float yOrigin;
  float zOrigin;
  sscanf( tag.c_str(), "%f\\%f\\%f", &xOrigin, &yOrigin, &zOrigin );

  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0018|0050", tag );
  float sliceThickness = atof( tag.c_str() );
   
  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0018|0088", tag );
  float sliceSpacing = atof( tag.c_str() );

  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0020|1041", tag );  
  float maxSliceLocation = atof( tag.c_str() );
       
  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[1], "0020|1041", tag );  
  float minSliceLocation = atof( tag.c_str() );

  for (int k = 0; k < nSlice; k++)
  {
    tag.clear();
    itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0020|1041",  tag);
    float sliceLocation = atof( tag.c_str() );
    
    if (sliceLocation > maxSliceLocation)
    {
      maxSliceLocation = sliceLocation;
    }
    
    if (sliceLocation < minSliceLocation)
    {
      minSliceLocation = sliceLocation;
    }
  }    

  // check ImageOrientationPatient and figure out Slice direction in right-hand coordinate system.
  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0020|0037", tag );
  float xRow, yRow, zRow, xCol, yCol, zCol, xSlice, ySlice, zSlice;
  sscanf( tag.c_str(), "%f\\%f\\%f\\%f\\%f\\%f", &xRow, &yRow, &zRow, &xCol, &yCol, &zCol );
  // Cross product
  xSlice = (yRow*zCol-zRow*yCol)*sliceSpacing;
  ySlice = (zRow*xCol-xRow*zCol)*sliceSpacing;
  zSlice = (xRow*yCol-yRow*xCol)*sliceSpacing;

  bool rightHanded = true;
  {
  float x0, y0, z0;
  float x1, y1, z1;
  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0020|0032", tag );
  sscanf( tag.c_str(), "%f\\%f\\%f", &x0, &y0, &z0 );
  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[1], "0020|0032", tag );
  sscanf( tag.c_str(), "%f\\%f\\%f", &x1, &y1, &z1 );
  x1 -= x0; y1 -= y0; z1 -= z0;
  x0 = x1*xSlice + y1*ySlice + z1*zSlice;
  if (x0 < 0)
    {
    rightHanded = false;
    }
  }
  
  if (!rightHanded)
    {
    xSlice = -xSlice;
    ySlice = -ySlice;
    zSlice = -zSlice;
    }

  int nSliceInVolume = static_cast<int> ((maxSliceLocation - minSliceLocation) / sliceSpacing + 1.5 ); 
                          // .5 is for rounding up, 1 is for adding one slice at one end.
  int nVolume = nSlice/nSliceInVolume;
  
  
  // read in gradient vectors and determin nBaseline and nMeasurement
  float bValue;
  int nBaseline = 0;
  
  for (int k = 0; k < nSlice; k += nSliceInVolume)
    {
    tag.clear();
    itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0043|1039",  tag);
    float b = atof( tag.c_str() );
    if (b == 0)
      {
      nBaseline ++;
      }
    else
      {
      bValue = b;
      }
    }    
  
  int nMeasurement = nVolume - nBaseline;

  // extract gradient vectors and form DWImages
  vnl_vector_fixed<double, 3> vect3d;
  std::vector< vnl_vector_fixed<double, 3> > DiffusionVectors;
  std::vector< int > idVolume( nMeasurement );
  int count = 0;
  
  for (int k = 0; k < nSlice; k += nSliceInVolume)
    {
    tag.clear();
    itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0043|1039",  tag);
    float b = atof( tag.c_str() );
    if (b == 0)
      {
      continue;
      }
    
    idVolume[count] = k/nSliceInVolume;
    count ++;

    tag.clear();
    itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0019|10bb",  tag);
    vect3d[0] = atof( tag.c_str() );

    tag.clear();
    itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0019|10bc",  tag);
    vect3d[1] = atof( tag.c_str() );

    tag.clear();
    itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0019|10bd",  tag);
    vect3d[2] = atof( tag.c_str() );

    DiffusionVectors.push_back(vect3d);      
    }
  
  // zReverse for gradient directions
    
  for (int k = 0; k < nMeasurement; k++)
  {
    std::cout << idVolume[k] << "\t"<< DiffusionVectors[k] << std::endl;
  }

///////////////////////////////////////////////
// write volumes in raw format
  itk::ImageFileWriter< VolumeType >::Pointer rawWriter = itk::ImageFileWriter< VolumeType >::New();
  itk::RawImageIO<PixelValueType, 3>::Pointer rawIO = itk::RawImageIO<PixelValueType, 3>::New();
  std::string rawFileName = outputDir + "\\" + dataname;
  rawWriter->SetFileName( rawFileName.c_str() );
  rawWriter->SetInput( reader->GetOutput() );
  rawWriter->SetImageIO( rawIO );
  rawIO->SetByteOrderToLittleEndian();
  try
    {
    rawWriter->Update();
    }
  catch (itk::ExceptionObject &excp)
    {
    std::cerr << "Exception thrown while reading the series" << std::endl;
    std::cerr << excp << std::endl;
    return EXIT_FAILURE;
    }

//////////////////////////////////////////////
// write header file
// This part follows a DWI NRRD file in NRRD format 5. 
// There should be a better way using itkNRRDImageIO.

  std::ofstream header;
  std::string headerFileName = outputDir + "\\" + outputFileName;

  header.open (headerFileName.c_str());
  header << "NRRD0005" << std::endl;

  header << "content: exists(" << dataname << ",0)" << std::endl;
  header << "type: short" << std::endl;
  header << "dimension: 4" << std::endl;

  // need to check
  header << "space: right-anterior-superior" << std::endl;

  header << "sizes: " << nRows << " " << nCols << " " << nSliceInVolume << " " << nVolume << std::endl;
  header << "thicknesses:  NaN  NaN " << sliceSpacing << " NaN" << std::endl;
  
  // need to check
  header << "space directions: " 
    << "(" << xRow << ","<< yRow << ","<< zRow << ") "
    << "(" << xCol << ","<< yCol << ","<< zCol << ") "
    << "(" << xSlice << ","<< ySlice << ","<< zSlice << ") none" << std::endl;

  header << "centerings: cell cell cell ???" << std::endl;
  header << "kinds: space space space list" << std::endl;
  header << "endian: little" << std::endl;
  header << "encoding: raw" << std::endl;
  header << "space units: \"mm\" \"mm\" \"mm\"" << std::endl;
  header << "space origin: "
    <<"(" << xOrigin << ","<< yOrigin << ","<< zOrigin << ") " << std::endl;
  header << "data file: " << dataname << std::endl;

  // Need to check
  header << "measurement frame: (1,0,0) (0,1,0) (0,0,1)" << std::endl;

  header << "modality:=DWMRI" << std::endl;
  header << "DWMRI_b-value:=" << bValue << std::endl;
  header << "DWMRI_gradient_0000:=0  0  0" << std::endl;
  header << "DWMRI_NEX_0000:=" << nBaseline << std::endl;

  // need to check
  for (int k = nBaseline; k < nVolume; k++)
    {
    header << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << k << ":=" 
      << DiffusionVectors[k-nBaseline][0] << "   " << DiffusionVectors[k-nBaseline][1] << "   " << DiffusionVectors[k-nBaseline][2]
      << std::endl;
    }
  header.close();
     
  return EXIT_SUCCESS;  
}

