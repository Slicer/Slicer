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

1) Uses left-posterior-superior (Dicom default) as default space for philips and siemens.
This is the default space for NRRD header.
2) For GE data, Dicom data are arranged in volume interleaving order.
3) For Siemens data, images are arranged in mosaic form.
4) For oblique collected Philips data, the measurement frame for the
gradient directions is the same as the ImageOrientationPatient

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
#include <algorithm> //Needed for trasforom to convert string tolower

#include "itkXMLFilterWatcher.h"

#include "itkNrrdImageIO.h"
#include "itkImage.h"
#include "itkImageSeriesReader.h"
#include "itkMetaDataDictionary.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"
#include "itkRecursiveGaussianImageFilter.h"


#include "itkImageFileWriter.h"
#include "itkRawImageIO.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkNumericSeriesFileNames.h"

//#include "itkScalarImageToListAdaptor.h"
//#include "itkListSampleToHistogramGenerator.h"
//
#include "itksys/Base64.h"

#include "itkImage.h"
#include "vnl/vnl_math.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix_fixed.h"
#include "vnl/algo/vnl_svd.h"

#include "itkVectorImage.h"
#include "gdcmFile.h"
#include "gdcmGlobal.h"
#include "gdcmUtil.h"
#include "gdcmDebug.h"
#include "gdcmTS.h"
#include "gdcmValEntry.h"
#include "gdcmBinEntry.h"
#include "gdcmSeqEntry.h"
#include "gdcmRLEFramesInfo.h"
#include "gdcmJPEGFragmentsInfo.h"
#include "gdcmSQItem.h"

#include "gdcmSeqEntry.h"
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
  const gdcm::DictEntry PhilipsDictBValue              ( 0x2001, 0x1003, "FL", "1", "B Value of diffusion weighting" );
  const gdcm::DictEntry PhilipsDictDiffusionDirection  ( 0x2001, 0x1004, "CS", "1", "Diffusion Gradient Direction" );
  const gdcm::DictEntry PhilipsDictDiffusionDirectionRL( 0x2005, 0x10b0, "FL", "4", "Diffusion Direction R/L" );
  const gdcm::DictEntry PhilipsDictDiffusionDirectionAP( 0x2005, 0x10b1, "FL", "4", "Diffusion Direction A/P" );
  const gdcm::DictEntry PhilipsDictDiffusionDirectionFH( 0x2005, 0x10b2, "FL", "4", "Diffusion Direction F/H" );

#if 0
  //Defined in gdcm dicomV3.dic
  // Tags defined in Supplement 49
  // 0018 9075 CS 1 Diffusion Directionality
  // 0018 9076 SQ 1 Diffusion Gradient Direction Sequence
  // 0018 9087 FD 1 Diffusion b-value
  // 0018 9089 FD 3 Diffusion Gradient Orientation
  // 0018 9117 SQ 1 MR Diffusion Sequence
  // 0018 9147 CS 1 Diffusion Anisotropy Type
  const gdcm::DictEntry Supplement49DictDiffusionDirection                ( 0x0018, 0x9075, "CS", "1", "Diffusion Directionality");
  const gdcm::DictEntry Supplement49DictDiffusionGradientDirectionSequence( 0x0018, 0x9076, "SQ", "1", "Diffusion Gradient Direction Sequence" );
  const gdcm::DictEntry Supplement49DictDiffusionBValue                   ( 0x0018, 0x9087, "FD", "1", "Diffusion b-value" );
  const gdcm::DictEntry Supplement49DictDiffusionGradientOrientation      ( 0x0018, 0x9089, "FD", "3", "Diffusion Graident Orientation" );
  const gdcm::DictEntry Supplement49DictMRDiffusionSequence               ( 0x0018, 0x9117, "SQ", "1", "MR Diffusion Sequence" );
  const gdcm::DictEntry Supplement49DictDiffusionAnisotropyType           ( 0x0018, 0x9147, "CS", "1", "Diffusion Anisotropy Type" );
#endif

  // add relevant private tags from other vendors

  typedef vnl_vector_fixed<double, 3> VectorType;
  typedef itk::Vector<float, 3> OutputVectorType;

  typedef short PixelValueType;
  typedef itk::Image< PixelValueType, 3 > VolumeType;
  typedef itk::ImageSeriesReader< VolumeType > ReaderType;
  typedef itk::GDCMImageIO ImageIOType;
  typedef itk::GDCMSeriesFileNames InputNamesGeneratorType;


  static bool ExtractBinValEntry( gdcm::File * const header, const uint16_t group, const uint16_t elem, std::string& tag )
    {
    tag.clear();
     
    if ( header->GetBinEntry(group, elem) )
      {
      gdcm::BinEntry* binEntry = header->GetBinEntry(group, elem);
      const unsigned int binLength = binEntry->GetFullLength();
      tag.resize( binLength );
      uint8_t const * const tagString = binEntry->GetBinArea();

      for (unsigned int n = 0; n < binLength; n++)
        {
        tag[n] = *(tagString+n);
        }
      return true;
      }
    else if ( header->GetValEntry(group, elem) )
      {
      tag = header->GetValEntry(group, elem)->GetValue();
      return true;
      }
    return false;
    }

  static unsigned int ExtractSiemensDiffusionInformation( const std::string tagString, const std::string nameString, std::vector<double>& valueArray )
    {
    ::size_t atPosition = tagString.find( nameString );
      while( true )  // skip nameString inside a quotation 
        {
          std::string nextChar = tagString.substr( atPosition+nameString.size(), 1 );
          std::cout << nextChar << std::endl;
          if (nextChar.c_str()[0] == 0 )
            {
              break;
            }
          else
            {
              atPosition = tagString.find( nameString, atPosition+2 );
            }
        }

    if ( atPosition == std::string::npos)
      {
      return 0;
      }
    else
      {
      std::string infoAsString = tagString.substr( atPosition, tagString.size()-atPosition+1 );
      const char * infoAsCharPtr = infoAsString.c_str();

      unsigned int vm = *(infoAsCharPtr+64);
        {
        std::string vr = infoAsString.substr( 68, 2 );
        int syngodt = *(infoAsCharPtr+72);
        int nItems = *(infoAsCharPtr+76);
        int localDummy = *(infoAsCharPtr+80);

        //std::cout << "\tName String: " << nameString << std::endl;
        //std::cout << "\tVR: " << vr << std::endl;
        //std::cout << "\tVM: " << vm << std::endl;
        //std::cout << "Local String: " << infoAsString.substr(0,80) << std::endl;

        /* This hack is required for some Siemens VB15 Data */
        if ( ( nameString == "DiffusionGradientDirection" ) && (vr != "FD") )
          {
          bool loop = true;
          while ( loop )
            {
            atPosition = tagString.find( nameString, atPosition+26 );
            if ( atPosition == std::string::npos)
              {
              //std::cout << "\tFailed to find DiffusionGradientDirection Tag - returning" << vm << std::endl;
              return 0;
              }
            infoAsString = tagString.substr( atPosition, tagString.size()-atPosition+1 );
            infoAsCharPtr = infoAsString.c_str();
            //std::cout << "\tOffset to new position" << std::endl;
            //std::cout << "\tNew Local String: " << infoAsString.substr(0,80) << std::endl;
            vm = *(infoAsCharPtr+64);
            vr = infoAsString.substr( 68, 2 );
            if (vr == "FD") loop = false;
            syngodt = *(infoAsCharPtr+72);
            nItems = *(infoAsCharPtr+76);
            localDummy = *(infoAsCharPtr+80);
            //std::cout << "\tVR: " << vr << std::endl;
            //std::cout << "\tVM: " << vm << std::endl;
            }
          }
        else
          {
          //std::cout << "\tUsing initial position" << std::endl;
          }
        //std::cout << "\tArray Length: " << vm << std::endl;
        }

      unsigned int offset = 84;
      for (unsigned int k = 0; k < vm; k++)
        {
        const int itemLength = *(infoAsCharPtr+offset+4);
        const int strideSize = static_cast<int> (ceil(static_cast<double>(itemLength)/4) * 4);
        if ( infoAsString.length() < offset + 16 + itemLength )
          { // data not available or incomplete
          return 0;
          }
        const std::string valueString = infoAsString.substr( offset+16, itemLength );
        valueArray.push_back( atof(valueString.c_str()) );
        offset += 16+strideSize;
        }
      return vm;
      }
    }

  int isSystemBigEndian(void)
    {
    union {
      uint32_t i;
      char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1; 
    }

  std::string endianAwareSwap(const std::string toBeSwapped, const bool swapByteOrder)
    {
    // If system and files agree we're done.
    // Otherwise memcpy and reverse the bytes
    if (swapByteOrder)
      {
      // Swap bytes
      std::string tmp = toBeSwapped; 
      reverse(tmp.begin(),tmp.end());
      return tmp;
      }
    else
      {
      return toBeSwapped;
      }
    }

} // end of anonymous namespace


static int WriteVolume( VolumeType::Pointer img, const std::string fname )
{
  itk::ImageFileWriter< VolumeType >::Pointer imgWriter = itk::ImageFileWriter< VolumeType >::New();

  imgWriter->SetInput( img );
  imgWriter->SetFileName( fname.c_str() );
  try
    {
    imgWriter->Update();
    }
  catch (itk::ExceptionObject &excp)
    {
    std::cerr << "Exception thrown while reading the series" << std::endl;
    std::cerr << excp << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}


int main(int argc, char* argv[])
{
  PARSE_ARGS;


  bool violated=false;
  if (inputDicomDirectory.size() == 0) { violated = true; std::cout << "  --inputDicomDirectory Required! "  << std::endl; }
  if (outputVolume.size() == 0) { violated = true; std::cout << "  --outputVolume Required! "  << std::endl; }
  if (violated) return EXIT_FAILURE;
#if 0 //Defined in gdcm dicomV3.dic
  gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(Supplement49DictDiffusionDirection);
  gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(Supplement49DictDiffusionGradientDirectionSequence);
  gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(Supplement49DictDiffusionBValue);
  gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(Supplement49DictDiffusionGradientOrientation);

  gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(Supplement49DictMRDiffusionSequence);
  gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(Supplement49DictDiffusionAnisotropyType);

  gdcm::DictEntry * dictEntry=gdcm::Global::GetDicts()->GetDefaultPubDict()->GetEntry(0x0018,0x9089);
  dictEntry->Print();

#endif

  bool SliceOrderIS = true;
  bool SliceMosaic = false;
  bool SingleSeries = true;
  bool NrrdFormat = false;

  // check if the file name is valid
  ::size_t slashpos = outputVolume.find( "/" );
  if (slashpos == std::string::npos)
    {
    slashpos = outputVolume.find( "\\" );    // for windows
    }

  std::string nhdrname;
  if (slashpos != std::string::npos)
    {
    nhdrname = outputVolume;
    }
  else
    {
    nhdrname = outputDirectory + "/" + outputVolume;
    }

  std::cout << nhdrname << std::endl;
  std::string dataname;
    {
    const ::size_t i = nhdrname.find(".nhdr");
    const ::size_t j = nhdrname.find(".nrrd");
    if (i == std::string::npos && j == std::string::npos)
      {
      // not a valid nrrd extension
      std::cerr << "Warning: Output file does not have extension .nhdr or .nrrd." << std::endl;
      std::cerr << "         Extension .nrrd added." << std::endl;
      nhdrname = nhdrname + ".nrrd";
      NrrdFormat = true;
      }
    else if( i != std::string::npos )
      {
      // user specified .nhdr
      dataname = nhdrname.substr(0, i) + ".raw";
      NrrdFormat = false;
      }
    else
      {
      // user specified .nrrd
      NrrdFormat = true;
      }
    }

  //////////////////////////////////////////////////
  // 0a) read one slice and figure out vendor
  InputNamesGeneratorType::Pointer inputNames = InputNamesGeneratorType::New();
  inputNames->SetLoadSequences( true );
  inputNames->SetLoadPrivateTags( true );
  inputNames->SetInputDirectory( inputDicomDirectory.c_str() );

  ReaderType::FileNamesContainer filenames;

  const ReaderType::FileNamesContainer & filenamesInSeries =
    inputNames->GetInputFileNames();

  if ( filenamesInSeries.size() < 1 ) 
    {
    std::cout << "Error: no DICOM files found in inputDirectory: " << inputDicomDirectory << std::endl;
    return EXIT_FAILURE;
    }

  //HACK:  This is not true.  The Philips scanner has the ability to write a multi-frame single file for DTI data.
  //
  // If there are just one file in the serires returned above, it is obvious that the series is
  // not complete. There is no way to put diffusion weighted images in one file, even for mosaic
  // format. We, then, need to find all files in that directory and treate them as a single series
  // of diffusion weighted image.
  if ( filenamesInSeries.size() > 1 )
    {
    const ::size_t nFiles = filenamesInSeries.size();
    filenames.resize( 0 );
    for (::size_t k = 0; k < nFiles; k++)
      {
      filenames.push_back( filenamesInSeries[k] );
      }
    }
  else
    {
    std::cout << "gdcm returned just one file. " << std::endl;
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

      const std::string sqDir( directory.GetFile(k) );
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

  gdcm::File * headerLite = new gdcm::File;
  headerLite->SetFileName( filenames[0] );
  headerLite->SetMaxSizeLoadEntry( 65535 );
  headerLite->SetLoadMode( gdcm::LD_NOSEQ );
  headerLite->Load();

  //
  std::vector<int> ignorePhilipsSliceMultiFrame;

  // check the tag 0008|0070 for vendor information
  std::string vendor;
  ExtractBinValEntry( headerLite, 0x0008, 0x0070, vendor );
  for (unsigned int k = 0; k < vendor.size(); k++)
    {
    vendor[k] =  toupper( vendor[k] );
    }

  // check the tag 0008|0060 for modlity information
  std::string modality;
  ExtractBinValEntry( headerLite, 0x0008, 0x0060, modality );
  for (unsigned int k = 0; k < modality.size(); k++)
    {
    modality[k] =  toupper( modality[k] );
    }
  if (  modality.find("PT") != std::string::npos
    || modality.find("ST") != std::string::npos )
    {
    typedef itk::Image<float, 3> USVolumeType;
    itk::ImageSeriesReader<USVolumeType>::Pointer seriesReader =
      itk::ImageSeriesReader<USVolumeType>::New();
    seriesReader->SetFileNames( filenamesInSeries );

    itk::ImageFileWriter<USVolumeType>::Pointer nrrdImageWriter =
      itk::ImageFileWriter<USVolumeType>::New();

    nrrdImageWriter->SetFileName( nhdrname );
    nrrdImageWriter->SetInput( seriesReader->GetOutput() );
    try
      {
      nrrdImageWriter->Update();
      }
    catch( itk::ExceptionObject & err )
      {
      std::cerr << "ExceptionObject caught !" << std::endl;
      std::cerr << err << std::endl;
      return EXIT_FAILURE;
      }
    return EXIT_SUCCESS;
    }


  std::string ImageType;
  ExtractBinValEntry( headerLite, 0x0008, 0x0008, ImageType );

  delete(headerLite);

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
    std::cout << "Image acquired using a GE scanner" << std::endl;
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
    std::cout << "Image acquired using a Siemens scanner" << std::endl;

    if ( ImageType.find("MOSAIC") != std::string::npos )
      {
      std::cout << "Siemens Mosaic format" << std::endl;
      SliceMosaic = true;
      }
    else
      {
      std::cout << "Siemens split format" << std::endl;
      SliceMosaic = false;
      }
    }
  else if ( ( vendor.find("PHILIPS") != std::string::npos ) )
    {
    // for philips data
    std::cout << "Image acquired using a Philips scanner" << std::endl;
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(PhilipsDictBValue);
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(PhilipsDictDiffusionDirection);
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(PhilipsDictDiffusionDirectionRL);
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(PhilipsDictDiffusionDirectionAP);
    gdcm::Global::GetDicts()->GetDefaultPubDict()->AddEntry(PhilipsDictDiffusionDirectionFH);
    }
  else
    {
    std::cerr << "Unrecognized vendor." << std::endl;
    }


  //////////////////////////////////////////////////
  // 1) Read the input series as an array of slices
  ReaderType::Pointer reader = ReaderType::New();
  itk::GDCMImageIO::Pointer gdcmIO = itk::GDCMImageIO::New();
  reader->SetImageIO( gdcmIO );
  reader->SetFileNames( filenames );
  const unsigned int nSlice = filenames.size();
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
  VolumeType::Pointer dmImage = VolumeType::New();

  //////////////////////////////////////////////////
  // 1-A) Read the input dicom headers
  std::vector<gdcm::File *> allHeaders( filenames.size() );
  for (unsigned int k = 0; k < filenames.size(); k ++)
    {
    allHeaders[k] = new gdcm::File;
    allHeaders[k]->SetFileName( filenames[k] );
    allHeaders[k]->SetMaxSizeLoadEntry( 65535 );
    allHeaders[k]->SetLoadMode( gdcm::LD_NOSEQ );
    allHeaders[k]->Load();
    }

  /////////////////////////////////////////////////////
  // 2) Analyze the DICOM header to determine the
  //    number of gradient directions, gradient
  //    vectors, and form volume based on this info


  // load in all public tags
  std::string tag;

  // Get the transfer syntax telling us how binary data is encoded
  // in the image
  std::string transferSyntaxName = allHeaders[0]->GetTransferSyntaxName();
  std::transform(transferSyntaxName.begin(), transferSyntaxName.end(), transferSyntaxName.begin(), ::tolower);
  bool swapByteOrder = false;

  if ( transferSyntaxName.find("acr-nema") != std::string::npos ) 
    {
    std::cout << "WARNING: Unknown transfer syntax, assuming little endian.  This may cause strange results, " <<
      "such as bizarre b-values and directions." << std::endl;
    transferSyntaxName = "little";
    }
  if ( ( ( transferSyntaxName.find("little") != std::string::npos ) && isSystemBigEndian() ) ||
    ( ( transferSyntaxName.find("big") != std::string::npos ) && !isSystemBigEndian() ) )
    {
    // Need to swap binary byte order
    swapByteOrder = true;
    }
  else if ( ( ( transferSyntaxName.find("big") != std::string::npos ) && isSystemBigEndian() )
    || ( ( transferSyntaxName.find("little") != std::string::npos ) && !isSystemBigEndian()) )
    {
    // File and system match, do nothing.
    swapByteOrder = false;
    }
  else
    {
    // Something is screwed up, error out
    std::cout << "ERROR: Unknown transfer syntax found " << transferSyntaxName << std::endl;
    std::cout << "isSystemBigEndian=" << isSystemBigEndian() << std::endl;
    return EXIT_FAILURE;
    }

  // number of rows is the number of pixels in the second dimension
  ExtractBinValEntry( allHeaders[0], 0x0028, 0x0010, tag );
  int nRows = atoi( tag.c_str() );

  // number of columns is the number of pixels in the first dimension
  ExtractBinValEntry( allHeaders[0], 0x0028, 0x0011, tag );
  int nCols = atoi( tag.c_str() );

  ExtractBinValEntry( allHeaders[0], 0x0028, 0x0030, tag );
  float xRes;
  float yRes;
  sscanf( tag.c_str(), "%f\\%f", &xRes, &yRes );

  ExtractBinValEntry( allHeaders[0], 0x0020, 0x0032, tag );
  itk::Vector<double,3> ImageOrigin;
  sscanf( tag.c_str(), "%lf\\%lf\\%lf", &(ImageOrigin[0]), &(ImageOrigin[1]), &(ImageOrigin[2]) );

  ExtractBinValEntry( allHeaders[0], 0x0018, 0x0088, tag );
  float sliceSpacing = atof( tag.c_str() );

  //Make a hash of the sliceLocations in order to get the correct count.  This is more reliable since SliceLocation may not be available.
  std::map<std::string,int> sliceLocations;
  std::vector<int> sliceLocationIndicator;
  std::vector<std::string> sliceLocationStrings;
  sliceLocationIndicator.resize( nSlice );
  for (unsigned int k = 0; k < nSlice; k++)
    {
    ExtractBinValEntry( allHeaders[k], 0x0020, 0x0032, tag );
    sliceLocationStrings.push_back( tag );
    sliceLocations[tag]++;
    }

  for (unsigned int k = 0; k < nSlice; k++)
    {
    std::map<std::string,int>::iterator it = sliceLocations.find( sliceLocationStrings[k] );
    sliceLocationIndicator[k] = distance( sliceLocations.begin(), it );
    // std::cout << k << ": " << filenames[k] << " -- " << sliceLocationStrings[k] << " -- " << sliceLocationIndicator[k] << std::endl;
    }

  unsigned int numberOfSlicesPerVolume=sliceLocations.size();
  std::cout << "=================== numberOfSlicesPerVolume:" << numberOfSlicesPerVolume << std::endl;

  if ( nSlice >= 2)
    {
    if (sliceLocationIndicator[0] == sliceLocationIndicator[1])
      {
      std::cout << "Dicom images are ordered in a slice interleaving way." << std::endl;
      // reorder slices into a volume interleaving manner
      int Ns = numberOfSlicesPerVolume;
      int Nv = nSlice / Ns; // do we need to do error check here

      VolumeType::RegionType R = reader->GetOutput()->GetLargestPossibleRegion();
      R.SetSize(2,1);
      std::vector<VolumeType::PixelType> v(nSlice);
      std::vector<VolumeType::PixelType> w(nSlice);

      itk::ImageRegionIteratorWithIndex<VolumeType> I( reader->GetOutput(), R );
      for (I.GoToBegin(); !I.IsAtEnd(); ++I)
        {
        VolumeType::IndexType idx = I.GetIndex();

        // extract all values in one "column"
        for (unsigned int k = 0; k < nSlice; k++)
          {
          idx[2] = k;
          v[k] = reader->GetOutput()->GetPixel( idx );
          }

        // permute
        for (int k = 0; k < Nv; k++)
          {
          for (int m = 0; m < Ns; m++)
            {
            w[k*Ns+m] = v[m*Nv+k];
            }
          }

        // put things back in order
        for (unsigned int k = 0; k < nSlice; k++)
          {
          idx[2] = k;
          reader->GetOutput()->SetPixel( idx, w[k] );
          }
        }
      }
    else
      {
      std::cout << "Dicom images are ordered in a volume interleaving way." << std::endl;
      }
    }

  itk::Matrix<double,3,3> MeasurementFrame;
  MeasurementFrame.SetIdentity();

  // check ImageOrientationPatient and figure out slice direction in
  // L-P-I (right-handed) system.
  // In Dicom, the coordinate frame is L-P by default. Look at
  // http://medical.nema.org/dicom/2007/07_03pu.pdf ,  page 301
  ExtractBinValEntry( allHeaders[0], 0x0020, 0x0037, tag );
  itk::Matrix<double,3,3> LPSDirCos;
  sscanf( tag.c_str(), "%lf\\%lf\\%lf\\%lf\\%lf\\%lf",
    &(LPSDirCos[0][0]), &(LPSDirCos[1][0]), &(LPSDirCos[2][0]),
    &(LPSDirCos[0][1]), &(LPSDirCos[1][1]), &(LPSDirCos[2][1]) );
  // Cross product, this gives I-axis direction
  LPSDirCos[0][2] = (LPSDirCos[1][0]*LPSDirCos[2][1]-LPSDirCos[2][0]*LPSDirCos[1][1]);
  LPSDirCos[1][2] = (LPSDirCos[2][0]*LPSDirCos[0][1]-LPSDirCos[0][0]*LPSDirCos[2][1]);
  LPSDirCos[2][2] = (LPSDirCos[0][0]*LPSDirCos[1][1]-LPSDirCos[1][0]*LPSDirCos[0][1]);

  std::cout << "ImageOrientationPatient (0020:0037): ";
  std::cout << "LPS Orientation Matrix" << std::endl;
  std::cout << LPSDirCos << std::endl;

  itk::Matrix<double,3,3> SpacingMatrix;
  SpacingMatrix.Fill(0.0);
  SpacingMatrix[0][0]=xRes;
  SpacingMatrix[1][1]=yRes;
  SpacingMatrix[2][2]=sliceSpacing;
  std::cout << "SpacingMatrix" << std::endl;
  std::cout << SpacingMatrix << std::endl;

  itk::Matrix<double,3,3> OrientationMatrix;
  OrientationMatrix.SetIdentity();

  itk::Matrix<double,3,3> NRRDSpaceDirection;
  std::string nrrdSpaceDefinition="left-posterior-superior";;
  NRRDSpaceDirection=LPSDirCos*OrientationMatrix*SpacingMatrix;

  std::cout << "NRRDSpaceDirection" << std::endl;
  std::cout << NRRDSpaceDirection << std::endl;

  unsigned int mMosaic = 0;   // number of raws in each mosaic block;
  unsigned int nMosaic = 0;   // number of columns in each mosaic block
  unsigned int nSliceInVolume = 0;
  unsigned int nVolume = 0;

  // figure out slice order and mosaic arrangement.
  if ( vendor.find("GE") != std::string::npos ||
    (vendor.find("SIEMENS") != std::string::npos && !SliceMosaic) )
    {
    if(vendor.find("GE") != std::string::npos)
      {
      MeasurementFrame=LPSDirCos;
      }
    else //SIEMENS data assumes a measurement frame that is the identity matrix.
      {
      MeasurementFrame.SetIdentity();
      }
    // has the measurement frame represented as an identity matrix.
    ExtractBinValEntry( allHeaders[0], 0x0020, 0x0032, tag );
    float x0, y0, z0;
    sscanf( tag.c_str(), "%f\\%f\\%f", &x0, &y0, &z0 );
    std::cout << "Slice 0: " << tag << std::endl;

    // assume volume interleaving, i.e. the second dicom file stores
    // the second slice in the same volume as the first dicom file
    ExtractBinValEntry( allHeaders[1], 0x0020, 0x0032, tag );
    float x1, y1, z1;
    sscanf( tag.c_str(), "%f\\%f\\%f", &x1, &y1, &z1 );
    std::cout << "Slice 1: " << tag << std::endl;
    x1 -= x0; y1 -= y0; z1 -= z0;
    x0 = x1*(NRRDSpaceDirection[0][2]) + y1*(NRRDSpaceDirection[1][2]) + z1*(NRRDSpaceDirection[2][2]);
    if (x0 < 0)
      {
      SliceOrderIS = false;
      }
    }
  else if ( vendor.find("SIEMENS") != std::string::npos && SliceMosaic )
    {
    MeasurementFrame.SetIdentity(); //The DICOM version of SIEMENS that uses private tags
    // has the measurement frame represented as an identity matrix.
    std::cout << "Siemens SliceMosaic......" << std::endl;

    SliceOrderIS = false;

    // for siemens mosaic image, figure out mosaic slice order from 0029|1010
    // copy information stored in 0029,1010 into a string for parsing
    ExtractBinValEntry( allHeaders[0], 0x0029, 0x1010, tag );

    // parse SliceNormalVector from 0029,1010 tag
    std::vector<double> valueArray(0);
    int nItems = ExtractSiemensDiffusionInformation(tag, "SliceNormalVector", valueArray);
    if (nItems != 3)  // did not find enough information
      {
      std::cout << "Warning: Cannot find complete information on SliceNormalVector in 0029|1010" << std::endl;
      std::cout << "         Slice order may be wrong." << std::endl;
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
      std::cout << "Warning: Cannot find complete information on NumberOfImagesInMosaic in 0029|1010" << std:: endl;
      std::cout << "         Resulting image may contain empty slices." << std::endl;
      }
    else
      {
      nSliceInVolume = static_cast<int>(valueArray[0]);
      mMosaic = static_cast<int> (ceil(sqrt(valueArray[0])));
      nMosaic = mMosaic;
      }
    std::cout << "Mosaic in " << mMosaic << " X " << nMosaic << " blocks (total number of blocks = " << valueArray[0] << ")." << std::endl;
    }
  else if ( vendor.find("PHILIPS") != std::string::npos
    && nSlice > 1) // so this is not a philips multi-frame single dicom file
    {
    MeasurementFrame=LPSDirCos; //Philips oblique scans list the gradients with respect to the ImagePatientOrientation.
    SliceOrderIS = true;

    nSliceInVolume = numberOfSlicesPerVolume;
    nVolume = nSlice/nSliceInVolume;

    float x0, y0, z0;
    float x1, y1, z1;
    ExtractBinValEntry( allHeaders[0], 0x0020, 0x0032, tag );
    sscanf( tag.c_str(), "%f\\%f\\%f", &x0, &y0, &z0 );
    std::cout << "Slice 0: " << tag << std::endl;

    // b-value volume interleaving
    ExtractBinValEntry( allHeaders[nVolume], 0x0020, 0x0032, tag );
    sscanf( tag.c_str(), "%f\\%f\\%f", &x1, &y1, &z1 );
    std::cout << "Slice 1: " << tag << std::endl;
    x1 -= x0; y1 -= y0; z1 -= z0;
    x0 = x1*(NRRDSpaceDirection[0][2]) + y1*(NRRDSpaceDirection[1][2]) + z1*(NRRDSpaceDirection[2][2]);

    // VAM - This needs more investigation -
    // Should we default to false and change based on slice order
    if (x0 < 0)
      {
      SliceOrderIS = false;
      }
    }
  else if ( vendor.find("PHILIPS") != std::string::npos
    && nSlice == 1)
    {
    // special handling for philips multi-frame dicom later.
    }
  else
    {
    std::cout << " Warning: vendor type not valid" << std::endl;
    // treate the dicom series as an ordinary image and write a straight nrrd file.
    WriteVolume( reader->GetOutput(), nhdrname );
    return EXIT_SUCCESS;
    }

  if ( SliceOrderIS )
    {
    std::cout << "Slice order is IS" << std::endl;
    }
  else
    {
    std::cout << "Slice order is SI" << std::endl;
    (NRRDSpaceDirection[0][2]) = -(NRRDSpaceDirection[0][2]);
    (NRRDSpaceDirection[1][2]) = -(NRRDSpaceDirection[1][2]);
    (NRRDSpaceDirection[2][2]) = -(NRRDSpaceDirection[2][2]);
    }

  std::cout << "Row: " << (NRRDSpaceDirection[0][0]) << ", " << (NRRDSpaceDirection[1][0]) << ", " << (NRRDSpaceDirection[2][0]) << std::endl;
  std::cout << "Col: " << (NRRDSpaceDirection[0][1]) << ", " << (NRRDSpaceDirection[1][1]) << ", " << (NRRDSpaceDirection[2][1]) << std::endl;
  std::cout << "Sli: " << (NRRDSpaceDirection[0][2]) << ", " << (NRRDSpaceDirection[1][2]) << ", " << (NRRDSpaceDirection[2][2]) << std::endl;

  const float orthoSliceSpacing = fabs((NRRDSpaceDirection[2][2]));

  int nIgnoreVolume = 0; // Used for Philips Trace like images
  std::vector<int> useVolume;

  std::vector<float> bValues(0);
  float maxBvalue = 0;
  int nBaseline = 0;

  // UnmodifiedDiffusionVectorsInDicomLPSCoordinateSystem is only of debug purposes.
  std::vector< vnl_vector_fixed<double, 3> > DiffusionVectors;
  std::vector< vnl_vector_fixed<double, 3> > UnmodifiedDiffusionVectorsInDicomLPSCoordinateSystem;
  std::vector< unsigned int>  bad_gradient_indices;
  ////////////////////////////////////////////////////////////
  // vendor dependent tags.
  // read in gradient vectors and determin nBaseline and nMeasurement

  if ( vendor.find("GE") != std::string::npos )
    {
    nSliceInVolume = numberOfSlicesPerVolume;
    nVolume = nSlice/nSliceInVolume;

    // assume volume interleaving
    std::cout << "Number of Slices: " << nSlice << std::endl;
    std::cout << "Number of Volume: " << nVolume << std::endl;
    std::cout << "Number of Slices in each volume: " << nSliceInVolume << std::endl;

    for (unsigned int k = 0; k < nSlice; k += nSliceInVolume)
      {
      // parsing bvalue and gradient directions
      vnl_vector_fixed<double, 3> vect3d;
      vect3d.fill( 0 );
      float b = 0;

      ExtractBinValEntry( allHeaders[k], 0x0043, 0x1039, tag );
      b = atof( tag.c_str() );

      ExtractBinValEntry( allHeaders[k], 0x0019, 0x10bb, tag );
      vect3d[0] = -atof( tag.c_str() );

      ExtractBinValEntry( allHeaders[k], 0x0019, 0x10bc, tag );
      vect3d[1] = -atof( tag.c_str() );

      ExtractBinValEntry( allHeaders[k], 0x0019, 0x10bd, tag );
      vect3d[2] = atof( tag.c_str() );

      bValues.push_back( b );
      if (b == 0)
        {
        vect3d.fill( 0 );
        UnmodifiedDiffusionVectorsInDicomLPSCoordinateSystem.push_back(vect3d);
        DiffusionVectors.push_back(vect3d);
        }
      else
        {
        UnmodifiedDiffusionVectorsInDicomLPSCoordinateSystem.push_back(vect3d);
        // vect3d.normalize();
        DiffusionVectors.push_back(vect3d);
        }

      std::cout << "B-value: " << b <<
        "; diffusion direction: " << vect3d[0] << ", " << vect3d[1] << ", " << vect3d[2] << std::endl;
      }
    }
  else if ( vendor.find("PHILIPS") != std::string::npos && nSlice > 1 )
    {
    // assume volume interleaving
    std::cout << "Number of Slices: " << nSlice << std::endl;
    std::cout << "Number of Volumes: " << nVolume << std::endl;
    std::cout << "Number of Slices in each volume: " << nSliceInVolume << std::endl;

    std::string tmpString = "";
    //NOTE:  Philips interleaves the directions, so the all gradient directions can be
    //determined in the first "nVolume" slices which represents the first slice from each
    //of the gradient volumes.
    for (unsigned int k = 0; k < nVolume; k++ /*nSliceInVolume*/)
      {
      const bool useSuppplement49Definitions = ExtractBinValEntry( allHeaders[k], 0x0018, 0x9075, tag );
      std::string DiffusionDirectionality(tag);//This is either NONE or DIRECTIONAL (or ISOTROPIC or "" if the tag is not found)

      bool B0FieldFound = false;
      float b=0.0;
      if (useSuppplement49Definitions == true )
        {
        B0FieldFound=ExtractBinValEntry( allHeaders[k], 0x0018, 0x9087, tag );;
        double temp_b=0.0;
        tmpString = endianAwareSwap(tag,swapByteOrder);
        //memcpy(&temp_b,tag.c_str(),8);//This seems very strange to me, it should have been a binary value.
        memcpy(&temp_b,tmpString.c_str(),8);//Since we did the endianAwareSwap memcpy is endian safe
        b=temp_b;
        }
      else
        {
        B0FieldFound=ExtractBinValEntry( allHeaders[k], 0x2001, 0x1003, tag );
        //HJJ -- VAM -- HACK:  This indicates that this field is store in binaryj format.
        //it seems that this would not work consistently across differnt endedness machines.
        //std::cout << "tag=" << tag << std::endl;
        tmpString = endianAwareSwap(tag,swapByteOrder);
        //memcpy(&b, tag.c_str(), 4);
        memcpy(&b, tmpString.c_str(), 4);
        //endianAwareMemCpy(&b, tag.c_str(), 4,swapByteOrder);
        //std::cout << "b=" << b << std::endl;

        ExtractBinValEntry( allHeaders[k], 0x2001, 0x1004, tag );

        if((tag.find("I") != std::string::npos) && (b != 0) )
          {
          DiffusionDirectionality="ISOTROPIC";
          }
        // char const * const temp=tag.c_str();
        }

      vnl_vector_fixed<double, 3> vect3d;
      vect3d.fill( 0 );
      //std::cout << "HACK: " << "DiffusionDirectionality=" << DiffusionDirectionality << ", k= " <<  k << std::endl;
      //std::cout << "HACK: " << "B0FieldFound=" << B0FieldFound << ", b=" << b << ", DiffusionDirectionality=" << DiffusionDirectionality << std::endl;
      
      if ( DiffusionDirectionality.find("ISOTROPIC") != std::string::npos )
        { //Deal with images that are to be ignored
        //std::cout << " SKIPPING ISOTROPIC Diffusion. " << std::endl;
        //std::cout << "HACK: IGNORE IMAGEFILE:   " << k << " of " << filenames.size() << " " << filenames[k] << std::endl;
        // Ignore the Trace like image
        nIgnoreVolume++;
        useVolume.push_back(0);
        continue;
        }
      else if (( !B0FieldFound || b == 0 ) || ( DiffusionDirectionality.find("NONE") != std::string::npos )  )
        { //Deal with b0 images
        bValues.push_back(b);
        UnmodifiedDiffusionVectorsInDicomLPSCoordinateSystem.push_back(vect3d);
        DiffusionVectors.push_back(vect3d);
        useVolume.push_back(1);
        continue;
        }
      else if (DiffusionDirectionality.find("DIRECTIONAL") != std::string::npos || ( DiffusionDirectionality == "" ))
        { //Deal with gradient direction images
        bValues.push_back(b);
        //std::cout << "HACK: GRADIENT IMAGEFILE: " << k << " of " << filenames.size() << " " << filenames[k] << std::endl;
        useVolume.push_back(1);
        tag.clear();
        if (useSuppplement49Definitions == true )
          {
          //Use alternate method to get value out of a sequence header (Some Phillips Data).
          const bool exposedSuccedded=ExtractBinValEntry( allHeaders[k], 0x0018, 0x9089, tag );
          if(exposedSuccedded == false)
            {
            //std::cout << "Looking for  0018|9089 in sequence 0018,9076" << std::endl;

            gdcm::SeqEntry * DiffusionSeqEntry=allHeaders[k]->GetSeqEntry(0x0018,0x9076);
            if(DiffusionSeqEntry == NULL)
              {
              std::cout << "ERROR:  0018|9089 could not be found in Seq Entry 0018|9076" << std::endl;
              return EXIT_FAILURE;
              }
            else
              {
              const unsigned int n=DiffusionSeqEntry->GetNumberOfSQItems();
              if( n == 0 )
                {
                std::cout << "ERROR:  Sequence entry 0018|9076 has no items." << std::endl;
                return EXIT_FAILURE;
                }
              else
                {
                bool found0018_9089=false;
                gdcm::SQItem *item1=DiffusionSeqEntry->GetFirstSQItem();
                while(item1 != NULL && found0018_9089==false)
                  {
                  if( item1 == NULL )
                    {
                    std::cout << "ERROR:  First item not found for 0018|9076." << std::endl;
                    return EXIT_FAILURE;
                    }
                  else
                    {
                    gdcm::DocEntry *p=item1->GetDocEntry(0x0018,0x9089);
                    if(p==NULL)
                      {
                      item1=DiffusionSeqEntry->GetNextSQItem();
                      continue;
                      }
                    else
                      {
                      gdcm::ContentEntry *entry=dynamic_cast<gdcm::ContentEntry *>(p);
                      if(entry ==NULL)
                        {
                        std::cout << "ERROR:  DocEntry for 0018|9089 is not the correct type." << std::endl;
                        return EXIT_FAILURE;
                        }
                      else
                        {
                        found0018_9089=true;
                        const std::string gradientDirection(entry->GetValue());
                        //double x[3];
                        //memcpy(x,gradientDirection.c_str(),24);
                        //std::cout << "################################# [" << x[0] << ","<< x[1] << "," << x[2] << "]" << std::endl;
                        memcpy(&(vect3d[0]),gradientDirection.c_str(),24);
                        }
                      }
                    }
                  item1=DiffusionSeqEntry->GetNextSQItem();
                  }
                if(found0018_9089 == false)
                  {
                  std::cout << "ERROR:  0018|9076 sequence not found." << std::endl;
                  return EXIT_FAILURE;
                  }
                }
              }
            }
          else
            {
            itksysBase64_Decode((const unsigned char *)(tag.c_str()), 8*3, (unsigned char *)(&(vect3d[0])),8*3*4);
            std::cout << "===== gradient orientations:" << k << " " << filenames[k] << " (0018,9089) " << " " << *((const double * )tag.c_str()) << "|"<<vect3d << std::endl;
            }
          }
        else
          {
          float value;
          /*const bool b0exist =*/
          ExtractBinValEntry( allHeaders[k], 0x2005, 0x10b0, tag );
          tmpString = endianAwareSwap(tag,swapByteOrder);
          //memcpy(&value, tag.c_str(), 4); //HACK:  VAM -- this does not seem to be endedness compliant.
          memcpy(&value, tmpString.c_str(), 4); //Since we used endianAwareSwap this is endian compliant
          vect3d[0] = value;
          tag.clear();

          /*const bool b1exist =*/
          ExtractBinValEntry( allHeaders[k], 0x2005, 0x10b1, tag );
          tmpString = endianAwareSwap(tag,swapByteOrder);
          //memcpy(&value, tag.c_str(), 4); //HACK:  VAM -- this does not seem to be endedness compliant.
          memcpy(&value, tmpString.c_str(), 4); //Since we used endianAwareSwap this is endian complian
          vect3d[1] = value;
          tag.clear();

          /*const bool b2exist =*/
          ExtractBinValEntry( allHeaders[k], 0x2005, 0x10b2, tag );
          tmpString = endianAwareSwap(tag,swapByteOrder);
          //memcpy(&value, tag.c_str(), 4); //HACK:  VAM -- this does not seem to be endedness compliant.
          memcpy(&value, tmpString.c_str(), 4); //Since we used endianAwareSwap this is endian complian
          vect3d[2] = value;
          }

        UnmodifiedDiffusionVectorsInDicomLPSCoordinateSystem.push_back(vect3d);
        // vect3d.normalize();
        DiffusionVectors.push_back(vect3d);
        }
      else // Have no idea why we'd be here so error out
        {
        std::cout << "ERROR: DiffusionDirectionality was " << DiffusionDirectionality << "  Don't know what to do with that..." << std::endl;
        return EXIT_FAILURE;
        }

      std::cout << "ERROR: DiffusionDirectionality: " << DiffusionDirectionality  << " :B-Value " << b << " :DiffusionOrientation " << vect3d << " :Filename " << filenames[k] << std::endl;
      }
    }
  else if ( vendor.find("SIEMENS") != std::string::npos )
    {

    int nStride = 1;
    if ( !SliceMosaic )
      {
      std::cout << orthoSliceSpacing << std::endl;
      nSliceInVolume = numberOfSlicesPerVolume;
      nVolume = nSlice/nSliceInVolume;
      std::cout << "Number of Slices: " << nSlice << std::endl;
      std::cout << "Number of Volume: " << nVolume << std::endl;
      std::cout << "Number of Slices in each volume: " << nSliceInVolume << std::endl;
      nStride = nSliceInVolume;
      }
    else
      {
      std::cout << "Data in Siemens Mosaic Format" << std::endl;
      nVolume = nSlice;
      std::cout << "Number of Volume: " << nVolume << std::endl;
      std::cout << "Number of Slices in each volume: " << nSliceInVolume << std::endl;
      nStride = 1;
      }

    // JTM - Determine bvalues from all gradients
    double max_bValue = 0.0;
    vnl_vector_fixed<double, 3> vect3d;

    for (unsigned int k = 0; k < nSlice; k += nStride )
      {
      ExtractBinValEntry( allHeaders[k], 0x0029, 0x1010, tag );

      // parse B_value from 0029,1010 tag
      std::vector<double> valueArray(0);
      int nItems = ExtractSiemensDiffusionInformation(tag, "B_value", valueArray);

      if (nItems != 1)   // did not find enough information
        {
        std::cout << "Warning: Cannot find complete information on B_value in 0029|1010" << std::endl;
        bValues.push_back( 0.0 );
        vect3d.fill( 0.0 );
        UnmodifiedDiffusionVectorsInDicomLPSCoordinateSystem.push_back(vect3d);
        DiffusionVectors.push_back(vect3d);
        continue;
        }
      else
        {
        // JTM - Patch from UNC: fill the nhdr header with the gradient directions and 
        // bvalues computed out of the BMatrix
        valueArray.resize(0);
        int nItems = ExtractSiemensDiffusionInformation(tag, "B_matrix", valueArray);
        vnl_matrix_fixed<double, 3, 3> bMatrix;

        if ((useBMatrixGradientDirections) && (nItems == 6))
          {
          std::cout << "=============================================" << std::endl;
          std::cout << "BMatrix calculations..." << std::endl;
          // UNC comments: We get the value of the b-value tag in the header. 
          // We won't use it as is, but just to locate the B0 images.
          // This check must be added, otherwise the bmatrix of the B0 is not
          // read properly (it's not an actual field in the DICOM header of the B0).
          std::vector<double> bval_tmp(0);
          bool b0_image = false;

          // UNC comments: Get the bvalue
          nItems = ExtractSiemensDiffusionInformation(tag, "B_value", bval_tmp);
          if (bval_tmp[0] == 0)
            {
            b0_image = true;
            }

          // UNC comments: The principal eigenvector of the bmatrix is to be extracted as
          // it's the gradient direction and trace of the matrix is the b-value

          double bvalue = 0;

          // UNC comments: Fill out the 3x3 bmatrix with the 6 components read from the 
          // DICOM header.
          bMatrix[0][0] = valueArray[0];
          bMatrix[0][1] = valueArray[1];
          bMatrix[0][2] = valueArray[2];
          bMatrix[1][1] = valueArray[3];
          bMatrix[1][2] = valueArray[4];
          bMatrix[2][2] = valueArray[5];
          bMatrix[1][0] = bMatrix[0][1];
          bMatrix[2][0] = bMatrix[0][2];
          bMatrix[2][1] = bMatrix[1][2];

          // UNC comments: Computing the decomposition
          vnl_svd<double> svd(bMatrix);

          // UNC comments: Extracting the principal eigenvector i.e. the gradient direction
          vect3d[0] = svd.U(0,0);
          vect3d[1] = svd.U(1,0);
          vect3d[2] = svd.U(2,0);

          std::cout << "BMatrix: " << std::endl;
          std::cout << bMatrix[0][0] << std::endl;
          std::cout << bMatrix[0][1] << "\t" << bMatrix[1][1] << std::endl;
          std::cout << bMatrix[0][2] << "\t" << bMatrix[1][2] << "\t" << bMatrix[2][2] << std::endl;

          // UNC comments: The b-value si the trace of the bmatrix
          bvalue = bMatrix[0][0] + bMatrix[1][1] + bMatrix[2][2];
          std::cout << bvalue << std::endl;
          // UNC comments: Even if the bmatrix is null, the svd decomposition set the 1st eigenvector
          // to (1,0,0). So we force the gradient direction to 0 if the bvalue is null
          if((b0_image == true) || (bvalue == 0))
            {
            std::cout << "B0 image detected: gradient direction and bvalue forced to 0" << std::endl;
            vect3d[0] = 0;
            vect3d[1] = 0;
            vect3d[2] = 0;
            std::cout << "Gradient coordinates: " << vect3d[0] << " " << vect3d[1] << " " << vect3d[2] << std::endl;
            bValues.push_back(0);
            }
          else
            {
            std::cout << "Gradient coordinates: " << vect3d[0] << " " << vect3d[1] << " " << vect3d[2] << std::endl;
            bValues.push_back(bvalue);
            }
          DiffusionVectors.push_back(vect3d);
          }
        else
          {
          valueArray.resize(0);
          ExtractSiemensDiffusionInformation(tag, "B_value", valueArray);
          bValues.push_back( valueArray[0] );
          vect3d[0] = 0;
          vect3d[1] = 0;
          vect3d[2] = 0;
          DiffusionVectors.push_back(vect3d);
          }
        }

      if (bValues[k / nStride] > max_bValue)
        {
        max_bValue = bValues[k / nStride];
        }
      }


    // JTM - Create gradient scaling factor, which is determined by the largest b 
    // value in the scan
    std::vector<double> gradient_scaling_factor;

    if(useBMatrixGradientDirections == false)
      {
      for (unsigned int k = 0; k < nSlice; k+=nStride)
        {    
        double scaling_factor = bValues[k / nStride] / max_bValue;
        gradient_scaling_factor.push_back(scaling_factor); 
        }

      for (unsigned int k = 0; k < nSlice; k += nStride )
        {
        std::cout << "=======================================" << std::endl << std::endl;

        ExtractBinValEntry( allHeaders[k], 0x0029, 0x1010, tag );

        std::vector<double> valueArray;
        vnl_vector_fixed<double, 3> vect3d;

        // parse DiffusionGradientDirection from 0029,1010 tag
        valueArray.resize(0);
        int nItems = ExtractSiemensDiffusionInformation(tag, "DiffusionGradientDirection", valueArray);
        std::cout << "Number of Directions : " << nItems << std::endl;
        std::cout << "   Directions 0: " << valueArray[0] << std::endl;
        std::cout << "   Directions 1: " << valueArray[1] << std::endl;
        std::cout << "   Directions 2: " << valueArray[2] << std::endl;
        if (nItems != 3)  // did not find enough information
          {
          std::cout << "Warning: Cannot find complete information on DiffusionGradientDirection in 0029|1010" << std::endl;
          vect3d.fill( 0 );
          UnmodifiedDiffusionVectorsInDicomLPSCoordinateSystem.push_back(vect3d);
          DiffusionVectors.push_back(vect3d);
          }
        else
          {
          double DiffusionVector_magnitude;
          double DiffusionVector_magnitude_difference = 0.0;

          vect3d[0] = valueArray[0];
          vect3d[1] = valueArray[1];
          vect3d[2] = valueArray[2];

          DiffusionVector_magnitude = sqrt((vect3d[0]*vect3d[0]) + (vect3d[1]*vect3d[1]) + (vect3d[2]*vect3d[2]));

          if (gradient_scaling_factor[k / nStride] != 0.0)
            {          
            DiffusionVector_magnitude_difference = fabs(1.0 - (DiffusionVector_magnitude / gradient_scaling_factor[k / nStride]));
            std::cout << "DiffusionVector_magnitude_difference " << DiffusionVector_magnitude_difference << std::endl;
            std::cout << "gradient_scaling_factor " << gradient_scaling_factor[k / nStride] << std::endl;
            std::cout << "DiffusionVector_magnitude " << DiffusionVector_magnitude << std::endl;    
            if ((DiffusionVector_magnitude > 0.0) && (DiffusionVector_magnitude_difference > smallGradientThreshold) && (!useBMatrixGradientDirections))
              {
              std::cout << "ERROR: Gradient vector with unreasonably small magnitude exists." << std::endl;
              std::cout << "Gradient #" << k << " with magnitude " << DiffusionVector_magnitude << std::endl;
              //std::cout << "Please set smallGradientFix flag to either Rescale and/or Remove (please separate with comma) OR useBMatrixGradientDirections to calculate gradient directions from the scanner B Matrix to alleviate this problem." << std::endl;
              std::cout << "Please set useBMatrixGradientDirections to calculate gradient directions from the scanner B Matrix to alleviate this problem." << std::endl;
              return EXIT_FAILURE;
              }  
            }  

          UnmodifiedDiffusionVectorsInDicomLPSCoordinateSystem.push_back(vect3d);
          // vect3d.normalize();
          DiffusionVectors.push_back(vect3d);
          int p = bValues.size();
          std::cout << "Image#: " << k << " BV: " << bValues[p-1] << " GD: " << DiffusionVectors[k] << std::endl;
          }
        }
      }
    }
  else if (vendor.find("PHILIPS") != std::string::npos && nSlice == 1) // multi-frame file, everything is inside
    {

    std::map<std::vector<double>, double> gradientDirectionAndBValue;
    ignorePhilipsSliceMultiFrame.clear();
    sliceLocations.clear();
    gdcm::File *header = new gdcm::File;
    header->SetMaxSizeLoadEntry(65535);
    header->SetFileName( filenames[0] );
    header->SetLoadMode( gdcm::LD_ALL );
    header->Load();
    gdcm::DocEntry* d = header->GetFirstEntry();

    bValues.clear();
    DiffusionVectors.clear();
    useVolume.clear();

    bool visited00280010 = false;
    bool visited00280011 = false;
    bool visited00180088 = false;
    while(d)
      {
      if (d->GetKey() == "0028|0010")
        {
        visited00280010 = true;
        gdcm::ValEntry* v = dynamic_cast<gdcm::ValEntry*> (d);
        d->Print( std::cout );
        nRows = atoi(v->GetValue().c_str());
        }
      else if (d->GetKey() == "0028|0011")
        {
        visited00280011 = true;
        gdcm::ValEntry* v = dynamic_cast<gdcm::ValEntry*> (d);
        d->Print( std::cout );
        nCols = atoi(v->GetValue().c_str());
        }
      else if (d->GetKey() == "0018|0088")
        {
        visited00180088 = true;
        gdcm::ValEntry* v = dynamic_cast<gdcm::ValEntry*> (d);
        d->Print( std::cout );
        sliceSpacing = atof(v->GetValue().c_str());
        }
      else if (d->GetKey() == "5200|9230" && visited00180088 && visited00280011 && visited00280010)
        {
        break;
        }
      d = header->GetNextEntry();
      }

    gdcm::SeqEntry* sq = dynamic_cast<gdcm::SeqEntry*> (d);
    int nItems = sq->GetNumberOfSQItems();
    std::cout << "Total number of slices: " << nItems << std::endl;
    gdcm::SQItem * sqi = sq->GetFirstSQItem();

    // figure out
    // 1. size
    // 2. space directions
    // 3. space origin
    // 4. measurement frame
    // 5. slice order (SI or IS)
    int k = 0;
    while (sqi)
      {

      gdcm::SeqEntry* volEntry;
      gdcm::SQItem * innerSqi;
      gdcm::ValEntry* valEntry;

      if ( k == 0 )
        {
        volEntry = dynamic_cast<gdcm::SeqEntry*>( sqi->GetDocEntry( 0x0020, 0x9116) );
        innerSqi = volEntry->GetFirstSQItem();
        valEntry = dynamic_cast<gdcm::ValEntry*>( innerSqi->GetDocEntry( 0x0020, 0x0037) );
        sscanf( valEntry->GetValue().c_str(), "%lf\\%lf\\%lf\\%lf\\%lf\\%lf",
          &(LPSDirCos[0][0]), &(LPSDirCos[1][0]), &(LPSDirCos[2][0]),
          &(LPSDirCos[0][1]), &(LPSDirCos[1][1]), &(LPSDirCos[2][1]) );
        // Cross product, this gives I-axis direction
        LPSDirCos[0][2] = (LPSDirCos[1][0]*LPSDirCos[2][1]-LPSDirCos[2][0]*LPSDirCos[1][1]);
        LPSDirCos[1][2] = (LPSDirCos[2][0]*LPSDirCos[0][1]-LPSDirCos[0][0]*LPSDirCos[2][1]);
        LPSDirCos[2][2] = (LPSDirCos[0][0]*LPSDirCos[1][1]-LPSDirCos[1][0]*LPSDirCos[0][1]);

        volEntry = dynamic_cast<gdcm::SeqEntry*>( sqi->GetDocEntry( 0x0028, 0x9110) );
        innerSqi = volEntry->GetFirstSQItem();
        valEntry = dynamic_cast<gdcm::ValEntry*>( innerSqi->GetDocEntry( 0x0028, 0x0030) );
        sscanf( valEntry->GetValue().c_str(), "%f\\%f", &xRes, &yRes );
        }

      volEntry = dynamic_cast<gdcm::SeqEntry*>( sqi->GetDocEntry( 0x0020, 0x9113) );
      innerSqi = volEntry->GetFirstSQItem();
      valEntry = dynamic_cast<gdcm::ValEntry*>( innerSqi->GetDocEntry( 0x0020, 0x0032) );
      sliceLocations[valEntry->GetValue()] ++;
      if ( k == 0 )
        {
        sscanf( valEntry->GetValue().c_str(), "%lf\\%lf\\%lf",  &(ImageOrigin[0]), &(ImageOrigin[1]), &(ImageOrigin[2]) );
        }

      // figure out diffusion directions
      volEntry = dynamic_cast<gdcm::SeqEntry*>( sqi->GetDocEntry( 0x0018, 0x9117) );
      innerSqi = volEntry->GetFirstSQItem();
      valEntry = dynamic_cast<gdcm::ValEntry*>( innerSqi->GetDocEntry( 0x0018, 0x9075) );
      std::string dirValue = valEntry->GetValue();

      if ( dirValue.find("ISO") != std::string::npos )
        {
        useVolume.push_back(0);
        ignorePhilipsSliceMultiFrame.push_back( k );
        }
      else if (dirValue.find("NONE") != std::string::npos)
        {
        useVolume.push_back(1);
        std::vector<double> v(3);
        v[0] = 0; v[1] = 0; v[2] = 0;
        unsigned int nOld = gradientDirectionAndBValue.size();
        gradientDirectionAndBValue[v] = 0;
        unsigned int nNew = gradientDirectionAndBValue.size();

        if (nOld != nNew)
          {
          vnl_vector_fixed<double, 3> vect3d;
          vect3d.fill( 0 );
          DiffusionVectors.push_back( vect3d );
          UnmodifiedDiffusionVectorsInDicomLPSCoordinateSystem.push_back(vect3d);
          bValues.push_back( 0 );

          }
        }
      else
        {
        useVolume.push_back(1);
        valEntry = dynamic_cast<gdcm::ValEntry*>( innerSqi->GetDocEntry( 0x0018, 0x9087) );
        std::string dwbValue = valEntry->GetValue();

        volEntry = dynamic_cast<gdcm::SeqEntry*>( innerSqi->GetDocEntry( 0x0018, 0x9076) );
        innerSqi = volEntry->GetFirstSQItem();
        valEntry = dynamic_cast<gdcm::ValEntry*>( innerSqi->GetDocEntry( 0x0018, 0x9089) );
        std::string dwgValue = valEntry->GetValue();
        std::vector<double> v(3);
        v[0] = *(double*)(dwgValue.c_str());
        v[1] = *(double*)(dwgValue.c_str()+8);
        v[2] = *(double*)(dwgValue.c_str()+16);
        unsigned int nOld = gradientDirectionAndBValue.size();
        gradientDirectionAndBValue[v] = *(double*)(dwbValue.c_str());
        unsigned int nNew = gradientDirectionAndBValue.size();

        if (nOld != nNew)
          {
          vnl_vector_fixed<double, 3> vect3d;
          vect3d[0] = v[0]; vect3d[1] = v[1]; vect3d[2] = v[2];
          UnmodifiedDiffusionVectorsInDicomLPSCoordinateSystem.push_back(vect3d);
          // vect3d.normalize();
          DiffusionVectors.push_back( vect3d );

          bValues.push_back( *(double*)(dwbValue.c_str()) );
          }
        }

      sqi = sq->GetNextSQItem();
      k ++;
      }
    numberOfSlicesPerVolume=sliceLocations.size();
    std::cout << "LPS Matrix: " << std::endl << LPSDirCos << std::endl;
    std::cout << "Volume Origin: " << std::endl << ImageOrigin[0] << "," << ImageOrigin[1] << ","  << ImageOrigin[2] << "," << std::endl;
    std::cout << "Number of slices per volume: " << numberOfSlicesPerVolume << std::endl;
    std::cout << "Slice matrix size: " << nRows << " X " << nCols << std::endl;
    std::cout << "Image resolution: " << xRes << ", " << yRes << ", " << sliceSpacing << std::endl;

    SpacingMatrix.Fill(0.0);
    SpacingMatrix[0][0]=xRes;
    SpacingMatrix[1][1]=yRes;
    SpacingMatrix[2][2]=sliceSpacing;
    NRRDSpaceDirection=LPSDirCos*OrientationMatrix*SpacingMatrix;

    MeasurementFrame=LPSDirCos;

    nSliceInVolume = sliceLocations.size();
    nVolume = nItems/nSliceInVolume;
    nIgnoreVolume = ignorePhilipsSliceMultiFrame.size()/nSliceInVolume;

    for( unsigned int k2 = 0; k2 < bValues.size(); k2++ )
      {
      std::cout << k2 << ": direction: " <<  DiffusionVectors[k2][0] << ", " << DiffusionVectors[k2][1] << ", " << DiffusionVectors[k2][2] << ", b-value: " << bValues[k2] << std::endl;
      }

    delete(header);
    }
  else
    {
    std::cout << "ERROR: Unknown scanner vendor " << vendor << std::endl;
    std::cout << "       this dti file format is properly handled." << std::endl;
    return EXIT_FAILURE;
    }

  // Delete allHeaders here?
  for (unsigned int k = 0; k < filenames.size(); k ++)
    {
    delete (allHeaders[k]);
    }

  ///////////////////////////////////////////////
  // write volumes in raw format
  itk::ImageFileWriter< VolumeType >::Pointer rawWriter = itk::ImageFileWriter< VolumeType >::New();
  itk::RawImageIO<PixelValueType, 3>::Pointer rawIO = itk::RawImageIO<PixelValueType, 3>::New();
  //std::string rawFileName = outputDir + "/" + dataname;
  if ( !NrrdFormat )
    {
    rawWriter->SetFileName( dataname.c_str() );
    rawWriter->SetImageIO( rawIO );
    rawIO->SetByteOrderToLittleEndian();
    }

  // imgWriter is used to write out image in case it is not a dicom DWI image
  itk::ImageFileWriter< VolumeType >::Pointer imgWriter = itk::ImageFileWriter< VolumeType >::New();

  ///////////////////////////////////////////////
  // Update the number of volumes based on the
  // number to ignore from the header information
  const unsigned int nUsableVolumes = nVolume-nIgnoreVolume-bad_gradient_indices.size();
  std::cout << "Number of usable volumes: " << nUsableVolumes << std::endl;

  if ( vendor.find("GE") != std::string::npos ||
    (vendor.find("SIEMENS") != std::string::npos && !SliceMosaic) )
    {
    if (nUsableVolumes == 1)
      {
      imgWriter->SetInput( reader->GetOutput() );
      imgWriter->SetFileName( nhdrname.c_str() );
      try
        {
        imgWriter->Update();
        }
      catch (itk::ExceptionObject &excp)
        {
        std::cerr << "Exception thrown while reading the series" << std::endl;
        std::cerr << excp << std::endl;
        return EXIT_FAILURE;
        }
      return EXIT_SUCCESS;
      }
    else
      {
      if ( !NrrdFormat )
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
      }
    }
  else if ( vendor.find("SIEMENS") != std::string::npos && SliceMosaic)
    {
    // de-mosaic
    nRows /= mMosaic;
    nCols /= nMosaic;

    // center the volume since the image position patient given in the
    // dicom header was useless
    ImageOrigin[0] = -(nRows*(NRRDSpaceDirection[0][0]) + nCols*(NRRDSpaceDirection[0][1]) + nSliceInVolume*(NRRDSpaceDirection[0][2]))/2.0;
    ImageOrigin[1] = -(nRows*(NRRDSpaceDirection[1][0]) + nCols*(NRRDSpaceDirection[1][1]) + nSliceInVolume*(NRRDSpaceDirection[1][2]))/2.0;
    ImageOrigin[2] = -(nRows*(NRRDSpaceDirection[2][0]) + nCols*(NRRDSpaceDirection[2][1]) + nSliceInVolume*(NRRDSpaceDirection[2][2]))/2.0;

    VolumeType::Pointer img = reader->GetOutput();

    VolumeType::RegionType region = img->GetLargestPossibleRegion();
    VolumeType::SizeType size = region.GetSize();

    VolumeType::SizeType dmSize = size;
    unsigned int original_slice_number = dmSize[2] * nSliceInVolume;
    dmSize[0] /= mMosaic;
    dmSize[1] /= nMosaic;
    dmSize[2] = nUsableVolumes * nSliceInVolume;

    region.SetSize( dmSize );
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

    bool bad_slice = false;
    unsigned int bad_slice_counter = 0;
    for (unsigned int k = 0; k < original_slice_number; k++)
      {
      for ( unsigned int j = 0; j < bad_gradient_indices.size(); j++)
        {
        unsigned int start_bad_slice_number = bad_gradient_indices[j] * nSliceInVolume;
        unsigned int end_bad_slice_number = start_bad_slice_number + (nSliceInVolume - 1);

        if (k >= start_bad_slice_number && k <= end_bad_slice_number)
          {
          bad_slice = true;
          bad_slice_counter++;
          break;
          }
        else
          {
          bad_slice = false;
          }
        }

      if (bad_slice == false)
        {
        unsigned int new_k = k - bad_slice_counter;

        dmRegion.SetIndex(2, new_k);
        itk::ImageRegionIteratorWithIndex<VolumeType> dmIt( dmImage, dmRegion );

        // figure out the mosaic region for this slice
        int sliceIndex = k;

        //int nBlockPerSlice = mMosaic*nMosaic;
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
      }

    if (nUsableVolumes == 1)
      {
      imgWriter->SetInput( dmImage );
      imgWriter->SetFileName( nhdrname.c_str() );
      try
        {
        imgWriter->Update();
        }
      catch (itk::ExceptionObject &excp)
        {
        std::cerr << "Exception thrown while reading the series" << std::endl;
        std::cerr << excp << std::endl;
        return EXIT_FAILURE;
        }
      return EXIT_SUCCESS;
      }
    else
      {
      if ( !NrrdFormat )
        {
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
      }
    }
  else if (vendor.find("PHILIPS") != std::string::npos)
    {
    VolumeType::Pointer img = reader->GetOutput();

    VolumeType::RegionType region = img->GetLargestPossibleRegion();
    VolumeType::SizeType size = region.GetSize();

    VolumeType::SizeType dmSize = size;
    dmSize[2] = nSliceInVolume * (nUsableVolumes);

    region.SetSize( dmSize );
    dmImage->CopyInformation( img );
    dmImage->SetRegions( region );
    dmImage->Allocate();

    VolumeType::RegionType dmRegion = dmImage->GetLargestPossibleRegion();
    dmRegion.SetSize(2, 1);
    region.SetSize(0, dmSize[0]);
    region.SetSize(1, dmSize[1]);
    region.SetSize(2, 1);

    unsigned int count = 0;
    for (unsigned int i = 0; i < nVolume; i++)
      {
      if (useVolume[i] == 1)
        {
        for (unsigned int k = 0; k < nSliceInVolume; k++)
          {
          dmRegion.SetIndex(0, 0);
          dmRegion.SetIndex(1, 0);
          dmRegion.SetIndex(2, count*(nSliceInVolume)+k);
          itk::ImageRegionIteratorWithIndex<VolumeType> dmIt( dmImage, dmRegion );

          // figure out the region for this slice
          const int sliceIndex = k*nVolume+i;
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
    if (nUsableVolumes == 1)
      {
      imgWriter->SetInput( dmImage );
      imgWriter->SetFileName( nhdrname.c_str() );
      try
        {
        imgWriter->Update();
        }
      catch (itk::ExceptionObject &excp)
        {
        std::cerr << "Exception thrown while reading the series" << std::endl;
        std::cerr << excp << std::endl;
        return EXIT_FAILURE;
        }
      return EXIT_SUCCESS;
      }
    else
      {
      if ( !NrrdFormat )
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
      }
    //Verify sizes
    if( count != bValues.size() )
      {
      std::cout << "ERROR:  bValues are the wrong size." <<  count << " != " << bValues.size() << std::endl;
      return EXIT_FAILURE;
      }
    if( count != DiffusionVectors.size() )
      {
      std::cout << "ERROR:  DiffusionVectors are the wrong size." <<  count << " != " << DiffusionVectors.size() << std::endl;
      return EXIT_FAILURE;
      }
    if( count != UnmodifiedDiffusionVectorsInDicomLPSCoordinateSystem.size() )
      {
      std::cout << "ERROR:  UnmodifiedDiffusionVectorsInDicomLPSCoordinateSystem are the wrong size." <<  count << " != " << UnmodifiedDiffusionVectorsInDicomLPSCoordinateSystem.size() << std::endl;
      return EXIT_FAILURE;
      }
    }
  else
    {
    std::cout << "Warning:  invalid vendor found." << std::endl;
    WriteVolume( reader->GetOutput(), nhdrname );
    return EXIT_SUCCESS;
    }


  const vnl_matrix_fixed<double,3,3> InverseMeasurementFrame= MeasurementFrame.GetInverse();
    {
    //////////////////////////////////////////////
    // write header file
    // This part follows a DWI NRRD file in NRRD format 5.
    // There should be a better way using itkNRRDImageIO.

    std::ofstream header;
    //std::string headerFileName = outputDir + "/" + outputFileName;

    header.open (nhdrname.c_str(), std::ios::out | std::ios::binary);
    header << "NRRD0005" << std::endl;

    if (!NrrdFormat)
      {
      header << "content: exists(" << itksys::SystemTools::GetFilenameName(dataname) << ",0)" << std::endl;
      }
    header << "type: short" << std::endl;
    header << "dimension: 4" << std::endl;

    // need to check
    header << "space: " << nrrdSpaceDefinition << "" << std::endl;
    // in nrrd, size array is the number of pixels in 1st, 2nd, 3rd, ... dimensions
    header << "sizes: " << nCols << " " << nRows << " " << nSliceInVolume << " " << nUsableVolumes << std::endl;
    header << "thicknesses:  NaN  NaN " << sliceSpacing << " NaN" << std::endl;

    // need to check
    header << "space directions: "
      << "(" << (NRRDSpaceDirection[0][0]) << ","<< (NRRDSpaceDirection[1][0]) << ","<< (NRRDSpaceDirection[2][0]) << ") "
      << "(" << (NRRDSpaceDirection[0][1]) << ","<< (NRRDSpaceDirection[1][1]) << ","<< (NRRDSpaceDirection[2][1]) << ") "
      << "(" << (NRRDSpaceDirection[0][2]) << ","<< (NRRDSpaceDirection[1][2]) << ","<< (NRRDSpaceDirection[2][2])
      << ") none" << std::endl;
    header << "centerings: cell cell cell ???" << std::endl;
    header << "kinds: space space space list" << std::endl;

    header << "endian: little" << std::endl;
    header << "encoding: raw" << std::endl;
    header << "space units: \"mm\" \"mm\" \"mm\"" << std::endl;
    header << "space origin: "
      <<"(" << ImageOrigin[0] << ","<< ImageOrigin[1] << ","<< ImageOrigin[2] << ") " << std::endl;
    if (!NrrdFormat)
      {
      header << "data file: " << itksys::SystemTools::GetFilenameName(dataname) << std::endl;
      }

    // For scanners, the measurement frame for the gradient directions is the same as the
    // Excerpt from http://teem.sourceforge.net/nrrd/format.html definition of "measurement frame:"
    // There is also the possibility that a measurement frame
    // should be recorded for an image even though it is storing
    // only scalar values (e.g., a sequence of diffusion-weighted MR
    // images has a measurement frame for the coefficients of
    // the diffusion-sensitizing gradient directions, and
    // the measurement frame field is the logical store
    // this information).
    //
    // It was noticed on oblique Philips DTI scans that the prescribed protocol directions were
    // rotated by the ImageOrientationPatient amount and recorded in the DICOM header.
    // In order to compare two different scans to determine if the same protocol was prosribed,
    // it is necessary to multiply each of the recorded diffusion gradient directions by
    // the inverse of the LPSDirCos.
    if(useIdentityMeaseurementFrame)
      {
      header << "measurement frame: "
        << "(" << 1 << ","<< 0 << ","<< 0 << ") "
        << "(" << 0 << ","<< 1 << ","<< 0 << ") "
        << "(" << 0 << ","<< 0 << ","<< 1 << ")"
        << std::endl;
      }
    else
      {
      header << "measurement frame: "
        << "(" << (MeasurementFrame[0][0]) << ","<< (MeasurementFrame[1][0]) << ","<< (MeasurementFrame[2][0]) << ") "
        << "(" << (MeasurementFrame[0][1]) << ","<< (MeasurementFrame[1][1]) << ","<< (MeasurementFrame[2][1]) << ") "
        << "(" << (MeasurementFrame[0][2]) << ","<< (MeasurementFrame[1][2]) << ","<< (MeasurementFrame[2][2]) << ")"
        << std::endl;
      }

    header << "modality:=DWMRI" << std::endl;
    //  float bValue = 0;
    for (unsigned int k = 0; k < nUsableVolumes; k++)
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

    unsigned int shift_index = 0;
    unsigned int original_volume_number = nUsableVolumes + bad_gradient_indices.size();

    for (unsigned int k = 0; k < original_volume_number; k++)
      {
      float scaleFactor = 0;
      bool print_gradient = true;

      for (unsigned int j = 0; j < bad_gradient_indices.size(); j++)
        {
        if (k == bad_gradient_indices[j])
          {
          shift_index++;
          print_gradient = false;
          continue;
          }
        }

      if (maxBvalue > 0)
        {
        scaleFactor = sqrt( bValues[k]/maxBvalue );
        }
      std::cout << "For Multiple BValues: " << k << " -- " << bValues[k] << " / " << maxBvalue << " = " << scaleFactor << std::endl;

      if (print_gradient == true)
        {
        if(useIdentityMeaseurementFrame)
          {
          vnl_vector_fixed<double,3> RotatedDiffusionVectors=InverseMeasurementFrame*(DiffusionVectors[k-nBaseline]);
          header << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << k << ":="
            << RotatedDiffusionVectors[0] * scaleFactor << "   "
            << RotatedDiffusionVectors[1] * scaleFactor << "   "
            << RotatedDiffusionVectors[2] * scaleFactor << std::endl;
          }
        else
          {
          if(useBMatrixGradientDirections)
            {
            header << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << k << ":="
              << DiffusionVectors[k][0] << "   "
              << DiffusionVectors[k][1] << "   "
              << DiffusionVectors[k][2] << std::endl;
            }
          else
            {
            unsigned int printed_gradient_number = k - shift_index;

            header << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << printed_gradient_number << ":="
              << DiffusionVectors[k-nBaseline][0] * scaleFactor << "   "
              << DiffusionVectors[k-nBaseline][1] * scaleFactor << "   "
              << DiffusionVectors[k-nBaseline][2] * scaleFactor << std::endl;
            }
          }
        }
      else
        {
        std::cout << "Gradient " << k << " was removed and will not be printed in the NRRD header file." << std::endl;
        }

      //std::cout << "Consistent Orientation Checks." << std::endl;
      //std::cout << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << k << ":="
      //  << LPSDirCos.GetInverse()*DiffusionVectors[k-nBaseline] << std::endl;
      }

    // write data in the same file is .nrrd was chosen
    header << std::endl;;
    if (NrrdFormat && SliceMosaic)
      {
      unsigned long nVoxels = dmImage->GetBufferedRegion().GetNumberOfPixels();
      header.write( reinterpret_cast<char *>(dmImage->GetBufferPointer()),
        nVoxels*sizeof(short) );
      }
    else if (NrrdFormat)
      {
      unsigned long nVoxels = reader->GetOutput()->GetBufferedRegion().GetNumberOfPixels();
      header.write( reinterpret_cast<char *>(reader->GetOutput()->GetBufferPointer()),
        nVoxels*sizeof(short) );
      }

    header.close();
    }

  if( writeProtocolGradientsFile == true )
    {
    //////////////////////////////////////////////
    // writeProtocolGradientsFile write protocolGradientsFile file
    // This part follows a DWI NRRD file in NRRD format 5.
    // There should be a better way using itkNRRDImageIO.

    std::ofstream protocolGradientsFile;
    //std::string protocolGradientsFileFileName = outputDir + "/" + outputFileName;

    const std::string protocolGradientsFileName=nhdrname+".txt";
    protocolGradientsFile.open ( protocolGradientsFileName.c_str() );
    protocolGradientsFile << "ImageOrientationPatient (0020|0032): "
      << LPSDirCos[0][0] << "\\" << LPSDirCos[1][0] << "\\" << LPSDirCos[2][0] << "\\"
         << LPSDirCos[0][1] << "\\" << LPSDirCos[1][1] << "\\" << LPSDirCos[2][1] << "\\"
         << std::endl;
    protocolGradientsFile << "==================================" << std::endl;
    protocolGradientsFile << "Direction Cosines: " << std::endl << LPSDirCos << std::endl;
    protocolGradientsFile << "==================================" << std::endl;
    protocolGradientsFile << "MeasurementFrame: " << std::endl << MeasurementFrame << std::endl;
    protocolGradientsFile << "==================================" << std::endl;
    for (unsigned int k = 0; k < nUsableVolumes; k++)
      {
      float scaleFactor = 0;
      if (maxBvalue > 0)
        {
        scaleFactor = sqrt( bValues[k]/maxBvalue );
        }
      protocolGradientsFile << "DWMRI_gradient_" << std::setw(4) << std::setfill('0') << k << "=["
        << DiffusionVectors[k-nBaseline][0] * scaleFactor << ";"
        << DiffusionVectors[k-nBaseline][1] * scaleFactor << ";"
        << DiffusionVectors[k-nBaseline][2] * scaleFactor << "]" <<std::endl;
      }
    protocolGradientsFile << "==================================" << std::endl;
    for (unsigned int k = 0; k < nUsableVolumes; k++)
      {
      float scaleFactor = 0;
      if (maxBvalue > 0)
        {
        scaleFactor = sqrt( bValues[k]/maxBvalue );
        }
      const vnl_vector_fixed<double, 3u> ProtocolGradient= InverseMeasurementFrame*DiffusionVectors[k-nBaseline];
      protocolGradientsFile << "Protocol_gradient_" << std::setw(4) << std::setfill('0') << k << "=["
        << ProtocolGradient[0] * scaleFactor << ";"
        << ProtocolGradient[1] * scaleFactor << ";"
        << ProtocolGradient[2] * scaleFactor << "]" <<std::endl;
      }
    protocolGradientsFile << "==================================" << std::endl;
    protocolGradientsFile.close();
    }
  return EXIT_SUCCESS;
}

