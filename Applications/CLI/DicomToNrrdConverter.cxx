/*=========================================================================

Program:   ReconstructTensor
Module:    $RCSfile: DicomToNrrdConverter.cxx,v $
Language:  C++
Date:      $Date: 2007/01/03 02:06:07 $
Version:   $Revision: 1.2 $

Copyright (c) General Electric Global Research. All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

*** 
This program converts Diffusion weighted MR images in Dicom format into
NRRD format.

Assumptions:

1) Space is always right-anterior-superior (scanner space or world space).
This is the ABSOLUTE space in which all coordinates are represented.
This is also the space for NRRD header.
2) For GE data, Dicom data are arranged in volume interleaving order.
3) For Siemens data, images are arranged in mosaic form.

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
#include "gdcmDictSet.h"        // access to dictionary
#include "gdcmDict.h"           // access to dictionary
#include "gdcmDictEntry.h"      // access to dictionary
#include "gdcmGlobal.h"         // access to dictionary

#include "DicomToNrrdConverterCLP.h"

// relevant GE private tags
const gdcm::DictEntry GEDictBValue( 0x0043, 0x1039, "IS", "1", "B Value of diffusion weighting" );
const gdcm::DictEntry GEDictXGradient( 0x0019, 0x10bb, "DS", "1", "X component of gradient direction" );
const gdcm::DictEntry GEDictYGradient( 0x0019, 0x10bc, "DS", "1", "Y component of gradient direction" );
const gdcm::DictEntry GEDictZGradient( 0x0019, 0x10bd, "DS", "1", "Z component of gradient direction" );

// relevant Siemens private tags
const gdcm::DictEntry SiemensMosiacParameters( 0x0051, 0x100b, "IS", "1", "Mosiac Matrix Size" );
const gdcm::DictEntry SiemensDictNMosiac( 0x0019, 0x100a, "US", "1", "Number of Images In Mosaic" );       
const gdcm::DictEntry SiemensDictBValue( 0x0019, 0x100c, "IS", "1", "B Value of diffusion weighting" );       
const gdcm::DictEntry SiemensDictDiffusionDirection( 0x0019, 0x100e, "FD", "3", "Diffusion Gradient Direction" );       
const gdcm::DictEntry SiemensDictDiffusionMatrix( 0x0019, 0x1027, "FD", "6", "Diffusion Matrix" );       
const gdcm::DictEntry SiemensDictShadowInfo( 0x0029, 0x1010, "LT", "1", "Siemens DWI Info" );       

// add relevant private tags from other vendors

typedef vnl_vector_fixed<double, 3> VectorType;
typedef itk::Vector<float, 3> OutputVectorType;

typedef short PixelValueType;
typedef itk::OrientedImage< PixelValueType, 3 > VolumeType;
typedef itk::ImageSeriesReader< VolumeType > ReaderType;
typedef itk::GDCMImageIO ImageIOType;
typedef itk::GDCMSeriesFileNames InputNamesGeneratorType;

void InsertUnique( std::vector<float> & vec, float value )
{
  int n = vec.size();
  if (n == 0)
    {
    vec.push_back( value );
    return;
    }

  for (int k = 0; k < n ; k++)
    {
    if (vec[k] == value)
      {
      return;
      }
    }

  // if we get here, it means value is not in vec.
  vec.push_back( value );
  return;
  
}

int main(int argc, char* argv[])
{

  PARSE_ARGS;

  bool SliceOrderIS = true;
  std::string vendor;
  bool SliceMosaic = false;

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

  //////////////////////////////////////////////////  
  // 0a) read one slice and figure out vendor
  ImageIOType::Pointer gdcmIO = ImageIOType::New();
  gdcmIO->LoadPrivateTagsOn();
  gdcmIO->SetMaxSizeLoadEntry( 65535 );

  InputNamesGeneratorType::Pointer inputNames = InputNamesGeneratorType::New();
  inputNames->SetInputDirectory( inputDicom.c_str() );

  const ReaderType::FileNamesContainer & filenames = 
    inputNames->GetInputFileNames();

  typedef itk::OrientedImage< PixelValueType, 2 > SliceType;
  typedef itk::ImageFileReader< SliceType > SliceReaderType;

  SliceReaderType::Pointer sReader = SliceReaderType::New();
  sReader->SetImageIO( gdcmIO );
  sReader->SetFileName( filenames[0] );
  try
    {
    sReader->Update();
    }
  catch (itk::ExceptionObject &excp)
    {
    std::cerr << "Exception thrown while reading the first file in the series" << std::endl;
    std::cerr << excp << std::endl;
    return EXIT_FAILURE;
    }

  // check the tag 0008|0070 for vendor information
  itk::MetaDataDictionary sliceDict = sReader->GetMetaDataDictionary();

  itk::ExposeMetaData<std::string> ( sliceDict, "0008|0070", vendor );
  std::cout << vendor << std::endl;

  std::string ImageType;
  itk::ExposeMetaData<std::string> ( sliceDict, "0008|0008", ImageType );
  std::cout << ImageType << std::endl;

  //////////////////////////////////////////////////////////////////
  //  0b) Add private dictionary
  if ( vendor.find("GE") != std::string::npos )
    {
    // for GE data
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(GEDictBValue);
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(GEDictXGradient);
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(GEDictYGradient);
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(GEDictZGradient);
    }
  else if( vendor.find("SIEMENS") != std::string::npos )
    {
    // for Siemens data
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensMosiacParameters);
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictNMosiac);
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictBValue);
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictDiffusionDirection);
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictDiffusionMatrix);
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(SiemensDictShadowInfo);

    if ( ImageType.find("MOSAIC") != std::string::npos )
      {
      std::cout << "Siemens Mosaic format\n";
      SliceMosaic = true;
      }
    else
      {
      std::cout << "Siemens split format\n";
      SliceMosaic = false;
      }

    }
  else if( vendor.find("PHILIPS") != std::string::npos )
    {
    // for philips data
    }
  else
    {
    std::cerr << "Unrecognized vendor.\n" << std::endl;
    }

  //////////////////////////////////////////////////  
  // 1) Read the input series as an array of slices
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
  //    vectors, and form volume based on this info

  ReaderType::DictionaryArrayRawPointer inputDict = reader->GetMetaDataDictionaryArray();

  // load in all public tags
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

  // figure out how many slices are there in a volume, each unique
  // SliceLocation represent one slice 
  std::vector<float> sliceLocations(0);
  for (int k = 0; k < nSlice; k++)
    {
    tag.clear();
    itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0020|1041",  tag);
    float sliceLocation = atof( tag.c_str() );
    InsertUnique( sliceLocations, sliceLocation );
    }    
  
  
  
  // check ImageOrientationPatient and figure out slice direction in
  // L-P-I (right-handed) system.
  // In Dicom, the coordinate frame is L-P by default. Look at
  // http://medical.nema.org/dicom/2007/07_03pu.pdf ,  page 301
  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0020|0037", tag );
  float xRow, yRow, zRow, xCol, yCol, zCol, xSlice, ySlice, zSlice, orthoSliceSpacing;
  sscanf( tag.c_str(), "%f\\%f\\%f\\%f\\%f\\%f", &xRow, &yRow, &zRow, &xCol, &yCol, &zCol );
  std::cout << "ImageOrientationPatient (0020:0037): ";
  std::cout << xRow << ", " << yRow << ", " << zRow << "; ";
  std::cout << xCol << ", " << yCol << ", " << zCol << "\n";
  
  // In Dicom, the measurement frame is L-P by default. Look at
  // http://medical.nema.org/dicom/2007/07_03pu.pdf ,  page 301, in
  // order to make this compatible with Slicer's RAS frame, we
  // multiply the direction cosines by the negatives of the resolution
  // (resolution is required by nrrd format). Direction cosine is not
  // affacted since the resulting frame is still a right-handed frame.
  xRow = -xRow;
  yRow = -yRow;

  xCol = -xCol;
  yCol = -yCol;

  // Cross product, this gives I-axis direction
  xSlice = (yRow*zCol-zRow*yCol)*sliceSpacing;
  ySlice = (zRow*xCol-xRow*zCol)*sliceSpacing;
  zSlice = (xRow*yCol-yRow*xCol)*sliceSpacing;

  xRow *= xRes;
  yRow *= xRes;
  zRow *= xRes;

  xCol *= yRes;
  yCol *= yRes;
  zCol *= yRes;

  // figure out slice order
  if ( vendor.find("GE") != std::string::npos ||
       (vendor.find("SIEMENS") != std::string::npos && !SliceMosaic) )
    {
    float x0, y0, z0;
    float x1, y1, z1;
    tag.clear();
    itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0020|0032", tag );
    sscanf( tag.c_str(), "%f\\%f\\%f", &x0, &y0, &z0 );
    std::cout << "Slice 0: " << tag << std::endl;
    tag.clear();

    // assume volume interleaving, i.e. the second dicom file stores
    // the second slice in the same volume as the first dicom file
    itk::ExposeMetaData<std::string> ( *(*inputDict)[1], "0020|0032", tag );
    sscanf( tag.c_str(), "%f\\%f\\%f", &x1, &y1, &z1 );
    std::cout << "Slice 1: " << tag << std::endl;
    x1 -= x0; y1 -= y0; z1 -= z0;
    x0 = x1*xSlice + y1*ySlice + z1*zSlice;
    if (x0 < 0)
      {
      SliceOrderIS = false;
      }
    }
  else if ( vendor.find("SIEMENS") != std::string::npos && SliceMosaic )
    {
      SliceOrderIS = false;

      // for siemens mosaic image, figure out mosaic slice order from 0029|1010

      tag.clear();
      itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0029|1010", tag );

      int atPosition = tag.find( "SliceNormalVector" );
      if ( atPosition != std::string::npos)
      {

        std::string infoAsString = tag.substr( atPosition, tag.size()-atPosition+1 );
        const char * infoAsCharPtr = infoAsString.c_str();

        std::string fieldName = "SliceNormalVector";
        int vm = *(infoAsCharPtr+64);
        std::string vr = infoAsString.substr( 68, 4 );
        int syngodt = *(infoAsCharPtr+72);
        int nItems = *(infoAsCharPtr+76);
        int localDummy = *(infoAsCharPtr+80);

        int offset = 84;
        float sliceNorm[3];
        for (int k = 0; k < vm; k++)
        {
          int itemLength = *(infoAsCharPtr+offset+4);
          int strideSize = ceil(static_cast<double>(itemLength)/4) * 4;
          std::string valueString = infoAsString.substr( offset+16, itemLength );
          sliceNorm[k] = atof( valueString.c_str() );
          offset += 16+strideSize;
        }

        if ( sliceNorm[2] > 0 )
        {
          SliceOrderIS = true;        
        }
      }
  }
  else
  {
  }
  
  if ( SliceOrderIS )
    {
    std::cout << "Slice order is IS\n";
    }
  else
    {
    std::cout << "Slice order is SI\n";
    }


  if (!SliceOrderIS)
    {
    xSlice = -xSlice;
    ySlice = -ySlice;
    zSlice = -zSlice;
    }

  std::cout << "Row: " << xRow << ", " << yRow << ", " << zRow << std::endl;
  std::cout << "Col: " << xCol << ", " << yCol << ", " << zCol << std::endl;
  std::cout << "Sli: " << xSlice << ", " << ySlice << ", " << zSlice << std::endl;
    
  orthoSliceSpacing = fabs(zSlice);
  
  int nSliceInVolume;
  int nVolume;

  std::vector<float> bValues(0);
  int nBaseline = 0;

  std::vector< vnl_vector_fixed<double, 3> > DiffusionVectors;
  ////////////////////////////////////////////////////////////
  // vendor dependent tags.
  // read in gradient vectors and determin nBaseline and nMeasurement

  if ( vendor.find("GE") != std::string::npos )
    {
    nSliceInVolume = sliceLocations.size();
    nVolume = nSlice/nSliceInVolume;

    // assume volume interleaving
    std::cout << "Number of Slices: " << nSlice << std::endl;
    std::cout << "Number of Volume: " << nVolume << std::endl;
    std::cout << "Number of Slices in each volume: " << nSliceInVolume << std::endl;

    for (int k = 0; k < nSlice; k += nSliceInVolume)
      {
      tag.clear();
      itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0043|1039",  tag);
      float b = atof( tag.c_str() );
      bValues.push_back(b);

      vnl_vector_fixed<double, 3> vect3d;
      if (b == 0)
        {
        vect3d.fill( 0 );
        DiffusionVectors.push_back(vect3d);      
        continue;
        }

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
    }
  else if ( vendor.find("SIEMENS") != std::string::npos && !SliceMosaic )
    {
    std::cout << orthoSliceSpacing << std::endl;
    
    nSliceInVolume = sliceLocations.size();
    nVolume = nSlice/nSliceInVolume;

    std::cout << "Number of Slices: " << nSlice << std::endl;
    std::cout << "Number of Volume: " << nVolume << std::endl;
    std::cout << "Number of Slices in each volume: " << nSliceInVolume << std::endl;
    
    for (int k = 0; k < nSlice; k += nSliceInVolume)
      {
      tag.clear();
      itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0019|100c",  tag);
      float b = atof( tag.c_str() );
      bValues.push_back(b);

      vnl_vector_fixed<double, 3> vect3d;
      if (b == 0)
        {
        vect3d.fill( 0 );
        DiffusionVectors.push_back(vect3d);      
        continue;
        }

      tag.clear();
      itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0019|100e",  tag);
      memcpy( &vect3d[0], tag.c_str()+0, 8 );
      memcpy( &vect3d[1], tag.c_str()+8, 8 );
      memcpy( &vect3d[2], tag.c_str()+16, 8 );
    
      DiffusionVectors.push_back(vect3d);

      std::cout << "Image#: " << k << " BV: " << b << " GD: " << vect3d << std::endl;
      
      }
    }
  else if ( vendor.find("SIEMENS") != std::string::npos && SliceMosaic )
    {
    // each slice is a volume in mosiac form

    std::cout << "Data in Siemens Mosaic Format\n";

    nVolume = nSlice;
    nSliceInVolume = 1;

    std::cout << "Number of Slices: " << nSlice << std::endl;
    std::cout << "Number of Volume: " << nVolume << std::endl;
    std::cout << "Number of Slices in each volume: " << nSliceInVolume << std::endl;
    
    for ( int k = 0; k < nSlice; k += nSliceInVolume )
      {
      tag.clear();
      itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0019|100c",  tag);
      
      float b = atof( tag.c_str() );
      bValues.push_back(b);

      vnl_vector_fixed<double, 3> vect3d;
      if (b == 0)
        {
        vect3d.fill( 0 );
        DiffusionVectors.push_back(vect3d);      
        continue;
        }

      tag.clear();
      itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0019|100e",  tag);
      
      memcpy( &vect3d[0], tag.c_str()+0, 8 );
      memcpy( &vect3d[1], tag.c_str()+8, 8 );
      memcpy( &vect3d[2], tag.c_str()+16, 8 );
    
      DiffusionVectors.push_back(vect3d);      
      std::cout << "Image#: " << k << " BV: " << b << " GD: " << vect3d << std::endl;
      }
      
    }
  else
    {
    }
  // transform gradient directions into RAS frame 

  for (int k = 0; k < nVolume; k++)
    {
    if ( !SliceOrderIS )
      {
      DiffusionVectors[k][2] = -DiffusionVectors[k][2];  // I -> S
      }
    }

  ///////////////////////////////////////////////
  // write volumes in raw format
  itk::ImageFileWriter< VolumeType >::Pointer rawWriter = itk::ImageFileWriter< VolumeType >::New();
  itk::RawImageIO<PixelValueType, 3>::Pointer rawIO = itk::RawImageIO<PixelValueType, 3>::New();
  std::string rawFileName = outputDir + "/" + dataname;
  rawWriter->SetFileName( rawFileName.c_str() );
  rawWriter->SetImageIO( rawIO );
  rawIO->SetByteOrderToLittleEndian();

  if ( vendor.find("GE") != std::string::npos ||
       (vendor.find("SIEMENS") != std::string::npos && !SliceMosaic) )
    {
    rawWriter->SetInput( reader->GetOutput() );
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
    }
  else if ( vendor.find("SIEMENS") != std::string::npos && SliceMosaic)
    {
    // de-mosaic
    tag.clear();
    itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0051|100b",  tag);
    int mMosaic;   // number of raws in each mosaic block;
    int nMosaic;   // number of columns in each mosaic block
    sscanf( tag.c_str(), "%dp*%ds", &mMosaic, &nMosaic);

    mMosaic = nRows/mMosaic;    // number of block rows in one dicom slice
    nMosaic = nCols/nMosaic;    // number of block columns in one
                                // dicom slice
    nRows /= mMosaic;
    nCols /= nMosaic;

    tag.clear();
    itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0019|100a",  tag);
    nSliceInVolume = atoi( tag.c_str() );

    // center the volume since the image position patient given in the
    // dicom header was useless
    xOrigin = -(nRows*xRow + nCols*xCol + nSliceInVolume*xSlice)/2.0;
    yOrigin = -(nRows*yRow + nCols*yCol + nSliceInVolume*ySlice)/2.0;
    zOrigin = -(nRows*zRow + nCols*zCol + nSliceInVolume*zSlice)/2.0;
    
    VolumeType::Pointer img = reader->GetOutput();

    VolumeType::RegionType region = img->GetLargestPossibleRegion();
    VolumeType::SizeType size = region.GetSize();

    VolumeType::SizeType dmSize = size;
    dmSize[0] /= mMosaic;
    dmSize[1] /= nMosaic;
    dmSize[2] *= nSliceInVolume;

    region.SetSize( dmSize );
    VolumeType::Pointer dmImage = VolumeType::New();
    dmImage->CopyInformation( img );
    dmImage->SetRegions( region );
    dmImage->Allocate();

    VolumeType::RegionType dmRegion = dmImage->GetLargestPossibleRegion();
    dmRegion.SetSize(2, 1);
    region.SetSize(0, dmSize[0]);
    region.SetSize(1, dmSize[1]);
    region.SetSize(2, 1);

    int rawMosaic = 0;
    int colMosaic = 0;
    int slcMosaic = 0;

    for (int k = 0; k < dmSize[2]; k++)
      {
      dmRegion.SetIndex(2, k);
      itk::ImageRegionIteratorWithIndex<VolumeType> dmIt( dmImage, dmRegion );

      // figure out the mosaic region for this slice
      int sliceIndex = k;
      
      int nBlockPerSlice = mMosaic*nMosaic;
      int slcMosaic = sliceIndex/(nSliceInVolume);
      sliceIndex -= slcMosaic*nSliceInVolume;
      int colMosaic = sliceIndex/mMosaic;
      int rawMosaic = sliceIndex - mMosaic*colMosaic;
      region.SetIndex( 0, rawMosaic*dmSize[0] );
      region.SetIndex( 1, colMosaic*dmSize[1] );
      region.SetIndex( 2, slcMosaic );

      itk::ImageRegionConstIteratorWithIndex<VolumeType> imIt( img, region );

      for ( dmIt.GoToBegin(), imIt.GoToBegin(); !dmIt.IsAtEnd(); ++dmIt, ++imIt)
        {
        dmIt.Set( imIt.Get() );
        }
      }
    rawWriter->SetInput( dmImage );
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
    }
  else
    {
    }

  //////////////////////////////////////////////
  // write header file
  // This part follows a DWI NRRD file in NRRD format 5. 
  // There should be a better way using itkNRRDImageIO.

  std::ofstream header;
  std::string headerFileName = outputDir + "/" + outputFileName;

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
  float bValue = 0;
  for (int k = 0; k < nVolume; k++)
    {
    if (bValues[k] > 0)
      {
      bValue = bValues[k];
      break;
      }
    }
  
  header << "DWMRI_b-value:=" << bValue << std::endl;

  //  header << "DWMRI_gradient_0000:=0  0  0" << std::endl;
  //  header << "DWMRI_NEX_0000:=" << nBaseline << std::endl;
  //  need to check
  for (int k = 0; k < nVolume; k++)
    {
    header << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << k << ":=" 
           << DiffusionVectors[k-nBaseline][0] << "   " << DiffusionVectors[k-nBaseline][1] << "   " << DiffusionVectors[k-nBaseline][2]
           << std::endl;
    }
  header.close();

  return EXIT_SUCCESS;  
}



