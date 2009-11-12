/*=========================================================================

Module:    $RCSfile: DicomToNrrdConverter.cxx,v $
Language:  C++
Date:      $Date: 2007/01/03 02:06:07 $
Version:   $Revision: 1.2 $

This work is part of the National Alliance for Medical Image 
Computing (NAMIC), funded by the National Institutes of Health 
through the NIH Roadmap for Medical Research, Grant U54 EB005149.

See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

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
#include "gdcmFile.h"           // access to dictionary
#include "gdcmDocEntry.h"       //internal of gdcm
#include "gdcmBinEntry.h"       //internal of gdcm
#include "gdcmValEntry.h"       //internal of gdcm
#include "gdcmDictEntry.h"      // access to dictionary
#include "gdcmGlobal.h"         // access to dictionary

#include "itksys/RegularExpression.hxx"
#include "itksys/Directory.hxx"
#include "itksys/SystemTools.hxx"

#include "DicomToNrrdConverterCLP.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace {

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
const gdcm::DictEntry SiemensDictShadowInfo( 0x0029, 0x1010, "OB", "1", "Siemens DWI Info" );       

// relevant Philips private tags
const gdcm::DictEntry PhilipsDictBValue( 0x2001, 0x1003, "FL", "1", "B Value of diffusion weighting" );
const gdcm::DictEntry PhilipsDictDiffusionDirection( 0x2001, 0x1004, "CS", "1", "Diffusion Gradient Direction" );
const gdcm::DictEntry PhilipsDictDiffusionDirectionRL( 0x2005, 0x10b0, "FL", "4", "Diffusion Direction R/L" );
const gdcm::DictEntry PhilipsDictDiffusionDirectionAP( 0x2005, 0x10b1, "FL", "4", "Diffusion Direction A/P" );
const gdcm::DictEntry PhilipsDictDiffusionDirectionFH( 0x2005, 0x10b2, "FL", "4", "Diffusion Direction F/H" );

// add relevant private tags from other vendors

typedef vnl_vector_fixed<double, 3> VectorType;
typedef itk::Vector<float, 3> OutputVectorType;

typedef short PixelValueType;
typedef itk::OrientedImage< PixelValueType, 3 > VolumeType;
typedef itk::ImageSeriesReader< VolumeType > ReaderType;
typedef itk::GDCMImageIO ImageIOType;
typedef itk::GDCMSeriesFileNames InputNamesGeneratorType;

int ExtractSiemensDiffusionInformation( std::string tagString, std::string nameString, std::vector<double>& valueArray )
{
  ::size_t atPosition = tagString.find( nameString );
  if ( atPosition == std::string::npos)
  {
    return 0;
  }
  else
  {
    std::string infoAsString = tagString.substr( atPosition, tagString.size()-atPosition+1 );
    const char * infoAsCharPtr = infoAsString.c_str();

    int vm = *(infoAsCharPtr+64);
    std::string vr = infoAsString.substr( 68, 4 );
//    int syngodt = *(infoAsCharPtr+72);
//    int nItems = *(infoAsCharPtr+76);
//    int localDummy = *(infoAsCharPtr+80);

    int offset = 84;
    for (int k = 0; k < vm; k++)
    {
      int itemLength = *(infoAsCharPtr+offset+4);
      int strideSize = static_cast<int> (ceil(static_cast<double>(itemLength)/4) * 4);
      std::string valueString = infoAsString.substr( offset+16, itemLength );
      valueArray.push_back( atof(valueString.c_str()) );
      offset += 16+strideSize;
    }
    return vm;
  }
}

void InsertUnique( std::vector<float> & vec, float value )
{
  ::size_t n = vec.size();
  if (n == 0)
  {
    vec.push_back( value );
    return;
  }

  for (::size_t k = 0; k < n ; k++)
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

} // end of anonymous namespace


int main(int argc, char* argv[])
{

  PARSE_ARGS;

  bool SliceOrderIS = true;
  std::string vendor;
  bool SliceMosaic = false;
  bool SingleSeries = true;

  // check if the file name is valid
  std::string nhdrname = outputVolume;
  std::string dataname;
  {
  ::size_t i;
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
  InputNamesGeneratorType::Pointer inputNames = InputNamesGeneratorType::New();
  inputNames->SetInputDirectory( inputDicomDirectory.c_str() );

  ReaderType::FileNamesContainer filenames;

  const ReaderType::FileNamesContainer & filenamesInSeries = 
    inputNames->GetInputFileNames();

  // If there are just one file in the serires returned above, it is obvious that the series is 
  // not complete. There is no way to put diffusion weighted images in one file, even for mosaic 
  // format. We, then, need to find all files in that directory and treate them as a single series
  // of diffusion weighted image.

  if ( filenamesInSeries.size() > 1 ) 
  { 
    ::size_t nFiles = filenamesInSeries.size(); 
    filenames.resize( 0 ); 
    for (::size_t k = 0; k < nFiles; k++) 
    { 
      filenames.push_back( filenamesInSeries[k] ); 
    } 
  } 
  else 
  { 
    std::cout << "gdcm returned just one file. \n"; 
    SingleSeries = false; 
    filenames.resize( 0 ); 
    itksys::Directory directory; 
    directory.Load( itksys::SystemTools::CollapseFullPath(inputDicomDirectory.c_str()).c_str() ); 
    ImageIOType::Pointer gdcmIOTest = ImageIOType::New();

    // for each patient directory 
    for ( unsigned int k = 0; k < directory.GetNumberOfFiles(); k++) 
    { 
      std::string subdirectory( inputDicomDirectory.c_str() ); 
      subdirectory = subdirectory + "/" + directory.GetFile(k); 

      std::string sqDir( directory.GetFile(k) ); 
      if (sqDir.length() == 1 && directory.GetFile(k)[0] == '.')   // skip self 
      { 
        continue; 
      } 
      else if (sqDir.length() == 2 && sqDir.find( ".." ) != std::string::npos)    // skip parent  
      { 
        continue; 
      } 
      else if (!itksys::SystemTools::FileIsDirectory( subdirectory.c_str() ))     // load only files 
      { 
        if ( gdcmIOTest->CanReadFile(subdirectory.c_str()) ) 
        { 
          filenames.push_back( subdirectory ); 
        } 
      } 
    }
    for (::size_t k = 0; k < filenames.size(); k++)
    {
      std::cout << "itksys file names: " << filenames[k] << std::endl;
    }
  }



  typedef itk::OrientedImage< PixelValueType, 2 > SliceType;
  typedef itk::ImageFileReader< SliceType > SliceReaderType;

  ImageIOType::Pointer gdcmIO = ImageIOType::New();
  gdcmIO->LoadPrivateTagsOn();
  gdcmIO->SetMaxSizeLoadEntry( 65536 );

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
    std::cout << "Image acquired using a GE scanner\n";
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
    std::cout << "Image acquired using a Siemens scanner\n";
    
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
  else if ( ( vendor.find("PHILIPS") != std::string::npos ) || 
    ( vendor.find("Philips") != std::string::npos ) )
    {
    // for philips data
    std::cout << "Image acquired using a Philips scanner\n";
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(PhilipsDictBValue);
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(PhilipsDictDiffusionDirection);
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(PhilipsDictDiffusionDirectionRL);
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(PhilipsDictDiffusionDirectionAP);
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(PhilipsDictDiffusionDirectionFH);
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
  // number of rows is the number of pixels in the second dimension 
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0028|0010", tag );
  int nRows = atoi( tag.c_str() );

  tag.clear();
  // number of columns is the number of pixels in the first dimension 
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
//  float sliceThickness = atof( tag.c_str() );

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

  int mMosaic = 0;   // number of raws in each mosaic block;
  int nMosaic = 0;   // number of columns in each mosaic block
  int nSliceInVolume = 0;
  int nVolume = 0;

  // figure out slice order and mosaic arrangement.
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
    std::cout << "Siemens SliceMosaic......" << std::endl;

    SliceOrderIS = false;

    // for siemens mosaic image, figure out mosaic slice order from 0029|1010
    tag.clear();
    gdcm::File *header0 = new gdcm::File;
    gdcm::BinEntry* binEntry;

    header0->SetMaxSizeLoadEntry(65536);
    header0->SetFileName( filenames[0] );
    header0->SetLoadMode( gdcm::LD_ALL );
    header0->Load();

    // copy information stored in 0029,1010 into a string for parsing
    gdcm::DocEntry* docEntry = header0->GetFirstEntry();
    while(docEntry)
    {
      if ( docEntry->GetKey() == "0029|1010"  )
      {
        binEntry = dynamic_cast<gdcm::BinEntry*> ( docEntry );
        int binLength = binEntry->GetFullLength();
        tag.resize( binLength );
        uint8_t * tagString = binEntry->GetBinArea();

        for (int k = 0; k < binLength; k++)
        {
          tag[k] = *(tagString+k);
        }
        break;
      }
      docEntry = header0->GetNextEntry();
    }

    // parse SliceNormalVector from 0029,1010 tag
    std::vector<double> valueArray(0);
    int nItems = ExtractSiemensDiffusionInformation(tag, "SliceNormalVector", valueArray);
    if (nItems != 3)  // did not find enough information
    {
      std::cout << "Warning: Cannot find complete information on SliceNormalVector in 0029|1010\n";
      std::cout << "         Slice order may be wrong.\n";
    }
    else if (valueArray[2] > 0)
    {
      SliceOrderIS = true;
    }

    // parse NumberOfImagesInMosaic from 0029,1010 tag
    valueArray.resize(0);
    nItems = ExtractSiemensDiffusionInformation(tag, "NumberOfImagesInMosaic", valueArray);
    if (nItems == 0)  // did not find enough information
    {
      std::cout << "Warning: Cannot find complete information on NumberOfImagesInMosaic in 0029|1010\n";
      std::cout << "         Resulting image may contain empty slices.\n";
    }
    else 
    {
      nSliceInVolume = static_cast<int>(valueArray[0]);
      mMosaic = static_cast<int> (ceil(sqrt(valueArray[0])));
      nMosaic = mMosaic;
    }
    std::cout << "Mosaic in " << mMosaic << " X " << nMosaic << " blocks (total number of blocks = " << valueArray[0] << ").\n"; 
  }
  else if ( vendor.find("Philips") != std::string::npos )
  {
    //SliceOrderIS = true;

    nSliceInVolume = sliceLocations.size();
    nVolume = nSlice/nSliceInVolume;

    float x0, y0, z0;
    float x1, y1, z1;
    tag.clear();
    itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0020|0032", tag );
    sscanf( tag.c_str(), "%f\\%f\\%f", &x0, &y0, &z0 );
    std::cout << "Slice 0: " << tag << std::endl;
    tag.clear();

    // b-value volume interleaving
    itk::ExposeMetaData<std::string> ( *(*inputDict)[nVolume], "0020|0032", tag );
    sscanf( tag.c_str(), "%f\\%f\\%f", &x1, &y1, &z1 );
    std::cout << "Slice 1: " << tag << std::endl;
    x1 -= x0; y1 -= y0; z1 -= z0;
    x0 = x1*xSlice + y1*ySlice + z1*zSlice;

    // VAM - This needs more investigation - 
    // Should we default to false and chnage based on slice order
    if (x0 < 0)
    {
      SliceOrderIS = false;
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


  int nIgnoreVolume = 0; // Used for Philips Trace like images
  std::vector<int> useVolume;

  std::vector<float> bValues(0);
  float maxBvalue = 0;
  int nBaseline = 0;

  // DiffusionVectorsOrig is only of debug purposes.
  std::vector< vnl_vector_fixed<double, 3> > DiffusionVectors;
  std::vector< vnl_vector_fixed<double, 3> > DiffusionVectorsOrig;
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

    for (int k = 0; k < nSlice; k +=  nSliceInVolume)
      {
      gdcm::File *header0 = new gdcm::File;
      gdcm::BinEntry* binEntry;
      gdcm::ValEntry* valEntry;
      
      header0->SetMaxSizeLoadEntry(65536);
      header0->SetFileName( filenames[k] );
      header0->SetLoadMode( gdcm::LD_ALL );
      header0->Load();

      // parsing bvalue and gradient directions
      int nValueParsed = 0;
      vnl_vector_fixed<double, 3> vect3d;
      vect3d.fill( 0 );
      float b = 0;
      
      gdcm::DocEntry* docEntry = header0->GetFirstEntry();
      
      while(docEntry)
        {     
        if ( docEntry->GetKey() == "0043|1039"  )
          {
            std::string geVR = docEntry->GetVR();
            if (geVR.find("UN") != std::string::npos)
            {
              binEntry = dynamic_cast<gdcm::BinEntry*> ( docEntry );
              int binLength = binEntry->GetFullLength();
              tag.resize( binLength );
              uint8_t * tagString = binEntry->GetBinArea();

              for (int n = 0; n < binLength; n++)
              {
                tag[n] = *(tagString+n);
              }
            }
            else if (geVR.find("DS") != std::string::npos ||
              geVR.find("IS") != std::string::npos )
            {
              valEntry = dynamic_cast<gdcm::ValEntry*> ( docEntry );
              tag = valEntry->GetValue();
            }
            b = atof( tag.c_str() );
            nValueParsed ++;
          }
        else if ( docEntry->GetKey() == "0019|10bb"  )
          {
            std::string geVR = docEntry->GetVR();
            if (geVR.find("UN") != std::string::npos)
            {
              binEntry = dynamic_cast<gdcm::BinEntry*> ( docEntry );
              int binLength = binEntry->GetFullLength();
              tag.resize( binLength );
              uint8_t * tagString = binEntry->GetBinArea();

              for (int n = 0; n < binLength; n++)
              {
                tag[n] = *(tagString+n);
              }
            }
            else if (geVR.find("DS") != std::string::npos ||
              geVR.find("IS") != std::string::npos )
            {
              valEntry = dynamic_cast<gdcm::ValEntry*> ( docEntry );
              tag = valEntry->GetValue();
            }
            vect3d[0] = atof( tag.c_str() );
            nValueParsed ++;
          }
        else if ( docEntry->GetKey() == "0019|10bc"  )
          {
            std::string geVR = docEntry->GetVR();
            if (geVR.find("UN") != std::string::npos)  // software version before 14.0
            {
              binEntry = dynamic_cast<gdcm::BinEntry*> ( docEntry );
              int binLength = binEntry->GetFullLength();
              tag.resize( binLength );
              uint8_t * tagString = binEntry->GetBinArea();

              for (int n = 0; n < binLength; n++)
              {
                tag[n] = *(tagString+n);
              }
            }
            else if (geVR.find("DS") != std::string::npos || // for software version 15.0
              geVR.find("IS") != std::string::npos)          // for software version 20.0
            {
              valEntry = dynamic_cast<gdcm::ValEntry*> ( docEntry );
              tag = valEntry->GetValue();
            }
            vect3d[1] = atof( tag.c_str() );
            nValueParsed ++;
          }
        else if ( docEntry->GetKey() == "0019|10bd"  )
          {
            std::string geVR = docEntry->GetVR();
            if (geVR.find("UN") != std::string::npos)
            {
              binEntry = dynamic_cast<gdcm::BinEntry*> ( docEntry );
              int binLength = binEntry->GetFullLength();
              tag.resize( binLength );
              uint8_t * tagString = binEntry->GetBinArea();

              for (int n = 0; n < binLength; n++)
              {
                tag[n] = *(tagString+n);
              }
            }
            else if (geVR.find("DS") != std::string::npos ||
              geVR.find("IS") != std::string::npos )
            {
              valEntry = dynamic_cast<gdcm::ValEntry*> ( docEntry );
              tag = valEntry->GetValue();
            }
            vect3d[2] = atof( tag.c_str() );
            nValueParsed ++;
          }

        if (nValueParsed == 4)
          {
          break;
          }
        docEntry = header0->GetNextEntry();
        }

      bValues.push_back( b );
      if (b == 0)
        {
        vect3d.fill( 0 );
        DiffusionVectors.push_back(vect3d);      
        DiffusionVectorsOrig.push_back(vect3d);      
        }
      else
        {
        DiffusionVectorsOrig.push_back(vect3d);      
        vect3d.normalize();
        DiffusionVectors.push_back(vect3d);
        }
      
      }
  }
  else if ( vendor.find("Philips") != std::string::npos )
  {
    //nSliceInVolume = sliceLocations.size();
    //nVolume = nSlice/nSliceInVolume;

    // assume volume interleaving
    std::cout << "Number of Slices: " << nSlice << std::endl;
    std::cout << "Number of Volumes: " << nVolume << std::endl;
    std::cout << "Number of Slices in each volume: " << nSliceInVolume << std::endl;

    for (int k = 0; k < nVolume; k +=1 /*nSliceInVolume*/)
    {
      tag.clear();
      bool exist = itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "2001|1003",  tag);
      float b = atof( tag.c_str() );
      memcpy(&b, tag.c_str(), 4);
      bValues.push_back(b);

      tag.clear();
      itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "2001|1004",  tag);

      vnl_vector_fixed<double, 3> vect3d;
      if (!exist || b == 0)
      {
        vect3d.fill( 0 );
        DiffusionVectors.push_back(vect3d);      
        DiffusionVectorsOrig.push_back(vect3d);   
        useVolume.push_back(1);
        continue;
      }          
      else if ( (tag.find("I") != std::string::npos) && (b != 0) )
      {
        // Ignore the Trace like image
        nIgnoreVolume++;
        useVolume.push_back(0);
        continue;
      }
      else
      {
        useVolume.push_back(1);
      }  
      tag.clear();
      vect3d.fill( 0 );

      float value;
      exist = itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "2005|10b0",  tag);
      memcpy(&value, tag.c_str(), 4); 
      vect3d[0] = value;
      tag.clear();

      exist = itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "2005|10b1",  tag);
      memcpy(&value, tag.c_str(), 4); 
      vect3d[1] = value;
      tag.clear();

      exist = itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "2005|10b2",  tag);
      memcpy(&value, tag.c_str(), 4); 
      vect3d[2] = value;
      tag.clear();

      DiffusionVectorsOrig.push_back(vect3d);      
      vect3d.normalize();
      DiffusionVectors.push_back(vect3d);      
    }
  }
  else if ( vendor.find("SIEMENS") != std::string::npos )
  {

    int nStride = 1;
    if ( !SliceMosaic )
    {
      std::cout << orthoSliceSpacing << std::endl;
      nSliceInVolume = sliceLocations.size();
      nVolume = nSlice/nSliceInVolume;
      std::cout << "Number of Slices: " << nSlice << std::endl;
      std::cout << "Number of Volume: " << nVolume << std::endl;
      std::cout << "Number of Slices in each volume: " << nSliceInVolume << std::endl;
      nStride = nSliceInVolume;
    }
    else
    {
      std::cout << "Data in Siemens Mosaic Format\n";
      nVolume = nSlice;
      std::cout << "Number of Volume: " << nVolume << std::endl;
      std::cout << "Number of Slices in each volume: " << nSliceInVolume << std::endl;
      nStride = 1;
    }


    for (int k = 0; k < nSlice; k += nStride )
    {

      gdcm::File *header0 = new gdcm::File;
      gdcm::BinEntry* binEntry;

      header0->SetMaxSizeLoadEntry(65536);
      header0->SetFileName( filenames[k] );
      header0->SetLoadMode( gdcm::LD_ALL );
      header0->Load();

      // copy information stored in 0029,1010 into a string for parsing
      gdcm::DocEntry* docEntry = header0->GetFirstEntry();
      while(docEntry)
      {
        if ( docEntry->GetKey() == "0029|1010"  )
        {
          binEntry = dynamic_cast<gdcm::BinEntry*> ( docEntry );
          int binLength = binEntry->GetFullLength();
          tag.resize( binLength );
          uint8_t * tagString = binEntry->GetBinArea();

          for (int n = 0; n < binLength; n++)
          {
            tag[n] = *(tagString+n);
          }
          break;
        }
        docEntry = header0->GetNextEntry();
      }

      // parse B_value from 0029,1010 tag
      std::vector<double> valueArray(0);
      vnl_vector_fixed<double, 3> vect3d;
      int nItems = ExtractSiemensDiffusionInformation(tag, "B_value", valueArray);
      std::cout << "Number of Items for Bvalue : " << nItems << std::endl;
      std::cout << "Bvalue : " << valueArray[0] << std::endl;

      if (nItems != 1)   // did not find enough information
      {
        std::cout << "Warning: Cannot find complete information on B_value in 0029|1010\n";
        bValues.push_back( 0.0 );
        vect3d.fill( 0.0 );
        DiffusionVectors.push_back(vect3d);      
        DiffusionVectorsOrig.push_back(vect3d);    
        continue;
      }
      else 
      {
        bValues.push_back( valueArray[0] );
      }

      // parse DiffusionGradientDirection from 0029,1010 tag
      valueArray.resize(0);
      nItems = ExtractSiemensDiffusionInformation(tag, "DiffusionGradientDirection", valueArray);
      std::cout << "Number of Directions : " << nItems << std::endl;
      std::cout << "   Directions 0: " << valueArray[0] << std::endl;
      std::cout << "   Directions 1: " << valueArray[1] << std::endl;
      std::cout << "   Directions 2: " << valueArray[2] << std::endl;
      if (nItems != 3)  // did not find enough information
      {
        std::cout << "Warning: Cannot find complete information on DiffusionGradientDirection in 0029|1010\n";
        vect3d.fill( 0 );
        DiffusionVectors.push_back(vect3d);      
        DiffusionVectorsOrig.push_back(vect3d);      
      }
      else 
      {
        vect3d[0] = valueArray[0];
        vect3d[1] = valueArray[1];
        vect3d[2] = valueArray[2];
        DiffusionVectorsOrig.push_back(vect3d);      
        vect3d.normalize();
        DiffusionVectors.push_back(vect3d);      
        int p = bValues.size();
        std::cout << "Image#: " << k << " BV: " << bValues[p-1] << " GD: " << DiffusionVectors[p-1] << std::endl;
      }
    }
  }
  else
  {
  }

  // transform gradient directions into RAS frame 
  for (int k = 0; k < nVolume; k++)
  {
    if ( !SliceOrderIS || SliceMosaic )
    {
      DiffusionVectors[k][2] = -DiffusionVectors[k][2];  // I -> S
      DiffusionVectorsOrig[k][2] = -DiffusionVectorsOrig[k][2];  // I -> S
    }
  }

  ///////////////////////////////////////////////
  // write volumes in raw format
  itk::ImageFileWriter< VolumeType >::Pointer rawWriter = itk::ImageFileWriter< VolumeType >::New();
  itk::RawImageIO<PixelValueType, 3>::Pointer rawIO = itk::RawImageIO<PixelValueType, 3>::New();
  //std::string rawFileName = outputDir + "/" + dataname;
  rawWriter->SetFileName( dataname.c_str() );
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
    nRows /= mMosaic;
    nCols /= nMosaic;

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

//    int rawMosaic = 0;
//    int colMosaic = 0;
    int slcMosaic = 0;

    for (unsigned int k = 0; k < dmSize[2]; k++)
    {
      dmRegion.SetIndex(2, k);
      itk::ImageRegionIteratorWithIndex<VolumeType> dmIt( dmImage, dmRegion );

      // figure out the mosaic region for this slice
      int sliceIndex = k;

//      int nBlockPerSlice = mMosaic*nMosaic;
      slcMosaic = sliceIndex/(nSliceInVolume);
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
      std::cerr << "Exception thrown while writing the series" << std::endl;
      std::cerr << excp << std::endl;
      return EXIT_FAILURE;
    }
  }
  else if (vendor.find("Philips") != std::string::npos)
  {
    VolumeType::Pointer img = reader->GetOutput();

    VolumeType::RegionType region = img->GetLargestPossibleRegion();
    VolumeType::SizeType size = region.GetSize();

    VolumeType::SizeType dmSize = size;
    dmSize[2] = nSliceInVolume * (nVolume-nIgnoreVolume);

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

    int count = 0;
    for (int i = 0; i < nVolume; i++)
    {
      if (useVolume[i] == 1)
      {
        for (int k = 0; k < nSliceInVolume; k++)
        {
          dmRegion.SetIndex(0, 0);
          dmRegion.SetIndex(1, 0);
          dmRegion.SetIndex(2, count*(nSliceInVolume)+k);
          itk::ImageRegionIteratorWithIndex<VolumeType> dmIt( dmImage, dmRegion );

          // figure out the region for this slice
          int sliceIndex = k*nVolume+i;
          region.SetIndex( 0, 0 );
          region.SetIndex( 1, 0 );
          region.SetIndex( 2, sliceIndex );

          itk::ImageRegionConstIteratorWithIndex<VolumeType> imIt( img, region );

          for ( dmIt.GoToBegin(), imIt.GoToBegin(); !dmIt.IsAtEnd(); ++dmIt, ++imIt)
          {
            dmIt.Set( imIt.Get() );
          }
        }
        count++;
      }
    }
    rawWriter->SetInput( dmImage );
    try
    {
      rawWriter->Update();
    }
    catch (itk::ExceptionObject &excp)
    {
      std::cerr << "Exception thrown while writing the series" << std::endl;
      std::cerr << excp << std::endl;
      return EXIT_FAILURE;
    }
  }
  else
  {
  }

  ///////////////////////////////////////////////
  // Update the number of volumes based on the 
  // number to ignore from the header information
  nVolume -= nIgnoreVolume;


  //////////////////////////////////////////////
  // write header file
  // This part follows a DWI NRRD file in NRRD format 5. 
  // There should be a better way using itkNRRDImageIO.

  std::ofstream header;
  //std::string headerFileName = outputDir + "/" + outputFileName;

  header.open (nhdrname.c_str());
  header << "NRRD0005" << std::endl;

  header << "content: exists(" << itksys::SystemTools::GetFilenameName(dataname) << ",0)" << std::endl;
  header << "type: short" << std::endl;
  header << "dimension: 4" << std::endl;

  // need to check
  header << "space: right-anterior-superior" << std::endl;
  // in nrrd, size array is the number of pixels in 1st, 2nd, 3rd, ... dimensions
  header << "sizes: " << nCols << " " << nRows << " " << nSliceInVolume << " " << nVolume << std::endl;
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
  header << "data file: " << itksys::SystemTools::GetFilenameName(dataname) << std::endl;

  // Need to check
  header << "measurement frame: (1,0,0) (0,1,0) (0,0,1)" << std::endl;

  header << "modality:=DWMRI" << std::endl;
//  float bValue = 0;
  for (int k = 0; k < nVolume; k++)
  {
    if (bValues[k] > maxBvalue)
    {
      maxBvalue = bValues[k];
    }
  }

  // this is the norminal BValue, i.e. the largest one.
  header << "DWMRI_b-value:=" << maxBvalue << std::endl;   

  //  the following three lines are for older NRRD format, where 
  //  baseline images are always in the begining.
  //  header << "DWMRI_gradient_0000:=0  0  0" << std::endl;
  //  header << "DWMRI_NEX_0000:=" << nBaseline << std::endl;
  //  need to check
  for (int k = 0; k < nVolume; k++)
  {
    float scaleFactor = 0;
    if (maxBvalue > 0)
    {
      scaleFactor = sqrt( bValues[k]/maxBvalue );
    }
    //std::cout << "For Multiple BValues: " << k << ": " << scaleFactor << std::endl;
    header << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << k << ":=" 
      << DiffusionVectors[k-nBaseline][0] * scaleFactor << "   " 
      << DiffusionVectors[k-nBaseline][1] * scaleFactor << "   " 
      << DiffusionVectors[k-nBaseline][2] * scaleFactor << std::endl;
  }
  header.close();

  return EXIT_SUCCESS;  
}



