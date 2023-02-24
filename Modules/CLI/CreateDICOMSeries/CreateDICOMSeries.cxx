/*=========================================================================

This plugin is based on the Insight Toolkit example
ImageReadDicomWrite. It has been modified for GenerateCLP style
command line processing and additional features have been added.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <ctime>

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkExtractImageFilter.h"
#include "itkShiftScaleImageFilter.h"
#include "itkGDCMImageIO.h"
#include "itkMetaDataObject.h"

#include "CreateDICOMSeriesCLP.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

// Create a random string from A-Z characters
std::string GenerateRandomCapitalLetters(int numberOfCharacters)
  {
  std::string id;
  std::string randomId;
  for (int i = 0; i < numberOfCharacters; i++)
    {
    id += static_cast<char>('A' + rand() % 24);
    }
  return id;
  }

template <class Tin>
int DoIt( int argc, char * argv[])
{
  PARSE_ARGS;

  typedef Tin     InputPixelType;

  typedef itk::Image<InputPixelType, 3>                        Image3DType;
  typedef itk::Image<InputPixelType, 2>                        Image2DType;
  typedef itk::ImageFileReader<Image3DType>                    ReaderType;
  typedef itk::ShiftScaleImageFilter<Image3DType, Image3DType> ShiftScaleType;
  typedef itk::ImageFileWriter<Image2DType>                    WriterType;
  typedef itk::GDCMImageIO                                     ImageIOType;

  typename Image3DType::Pointer image;
  typename ReaderType::Pointer  reader = ReaderType::New();

  try
    {
    reader->SetFileName(inputVolume.c_str() );
    reader->Update();
    image = reader->GetOutput();
    }
  catch( itk::ExceptionObject & excp )
    {
    std::cerr << "Exception thrown while reading the image file: " << inputVolume << std::endl;
    std::cerr << excp << std::endl;

    return EXIT_FAILURE;
    }

  typename Image3DType::SpacingType   spacing = image->GetSpacing();
  typename Image3DType::DirectionType oMatrix = image->GetDirection();

  // Shift scale the data if necessary based on the rescale slope and
  // rescale interscept prescribed.
  if( fabs(rescaleIntercept) > itk::NumericTraits<double>::epsilon()
      || fabs(rescaleSlope - 1.0) > itk::NumericTraits<double>::epsilon() )
    {
    reader->ReleaseDataFlagOn();

    typename ShiftScaleType::Pointer shiftScale = ShiftScaleType::New();
    shiftScale->SetInput( reader->GetOutput() );
    shiftScale->SetShift( -rescaleIntercept );

    if( fabs(rescaleSlope) < itk::NumericTraits<double>::epsilon() )
      {
      // too close to zero, ignore
      std::cerr << "Rescale slope too close to zero (" << rescaleSlope
                << "). Using the default value of 1.0" << std::endl;

      rescaleSlope = 1.0;
      }
    shiftScale->SetScale( 1.0 / rescaleSlope );

    shiftScale->Update();
    image = shiftScale->GetOutput();
    }

  typedef itk::MetaDataDictionary DictionaryType;
  unsigned int numberOfSlices = image->GetLargestPossibleRegion().GetSize()[2];

  typename ImageIOType::Pointer gdcmIO = ImageIOType::New();
  DictionaryType       dictionary;

  // Progress
  std::cout << "<filter-start>" << std::endl;
  std::cout << "<filter-name>ImageFileWriter</filter-name>" << std::endl;
  std::cout << "<filter-comment>Creating DICOM</filter-comment>" << std::endl;
  std::cout << "</filter-start>" << std::endl;
  std::cout << std::flush;

  // Initialize the random generator
  srand((unsigned)time(0));

  std::ostringstream value;

  // Get current time (this will be used everywhere where the current date or time is needed)
  std::time_t t = std::time(nullptr);
  std::tm tm = *std::localtime(&t);
  value.str("");
  value << std::put_time(&tm, "%H%M%S");
  std::string timeNow = value.str();
  value.str("");
  value << std::put_time(&tm, "%Y%m%d");
  std::string dateNow = value.str();

  // -----------------------------------------
  // SOP Common tags

  // SOP class UID (required)
  std::string sopClassUID;
  // We use ORIGINAL\PRIMARY so that we simulate real image acquisitions
  std::string imageType;
  if (modality == "MR")
    {
    sopClassUID = "1.2.840.10008.5.1.4.1.1.4";
    imageType = "ORIGINAL\\PRIMARY\\MPR"; // type 1
    }
  else if (modality == "CR")
    {
    sopClassUID = "1.2.840.10008.5.1.4.1.1.1";
    imageType = "ORIGINAL\\PRIMARY\\SINGLE PLANE";
    }
  else if (modality == "NM")
    {
    sopClassUID = "1.2.840.10008.5.1.4.1.1.20";
    imageType = "ORIGINAL\\PRIMARY\\STATIC";
    }
  else if (modality == "US")
    {
    sopClassUID = "1.2.840.10008.5.1.4.1.1.6.1";
    // We could use a generic value, such as "ORIGINAL\\PRIMARY\\ABDDOMINAL\\1",
    // but since Image Type is required, empty if unknown (required but can be empty), we just leave it empty.
    imageType = "";
    }
  else if (modality == "SC")
    {
    // Secondary capture, converted from non-DICOM (scanned drawings, screenshots, etc.)
    sopClassUID = "1.2.840.10008.5.1.4.1.1.7";
    // Value 3 shall identify any Image IOD specific specialization (optional), may be encoded with zero-length
    imageType = "ORIGINAL\\PRIMARY\\";
    }
  else // CT or other
    {
    if (modality != "CT")
      {
      std::cerr << "Unknown modality: " << modality << ". Using CT Image Storage SOP class UID." << std::endl;
      }
    sopClassUID = "1.2.840.10008.5.1.4.1.1.2"; // CT Image Storage
    imageType = "ORIGINAL\\PRIMARY\\AXIAL"; // type 1
    // Rescale Type is set to US (unspecified) in GDCM if the attribute is not set, which would not be optimal for CT,
    // therefore Rescale Type is set to HU (Hounsfield unit) here.
    if (rescaleType.empty())
      {
      rescaleType = "HU";
      }
  }

  // All strings are UTF8-encoded, set the SpecificCharacterSet accordingly.
  itk::EncapsulateMetaData<std::string>(dictionary, "0008|0005", "ISO_IR 192");

  itk::EncapsulateMetaData<std::string>(dictionary, "0008|0016", sopClassUID);
  itk::EncapsulateMetaData<std::string>(dictionary, "0008|0008", imageType);
  itk::EncapsulateMetaData<std::string>(dictionary, "0028|1054", rescaleType);

  // -----------------------------------------
  // Patient tags

  // Patient name (required, empty if unknown)
  itk::EncapsulateMetaData<std::string>(dictionary, "0010|0010", patientName);

  // Patient ID (required, empty if unknown)
  // It is recommended to set it, since there is no such thing as patient UID in DICOM.
  if (patientID=="[random]")
    {
    patientID = GenerateRandomCapitalLetters(6);
    }
  itk::EncapsulateMetaData<std::string>(dictionary, "0010|0020", patientID);

  // Patient's sex (required, empty if unknown). Can be M, F, O.
  if (patientSex == "[unknown]" || patientSex.empty())
    {
    patientSex.clear();
    }
  else if (patientSex != "M" && patientSex != "F" && patientSex != "O")
    {
    std::cerr << "Invalid patientSex: " << patientSex << ". Setting the attribute to empty instead." << std::endl;
    patientSex.clear();
    }
  itk::EncapsulateMetaData<std::string>(dictionary, "0010|0040", patientSex);

  // Patient comments (optional)
  if (!patientComments.empty())
    {
    itk::EncapsulateMetaData<std::string>(dictionary, "0010|4000", patientComments);
    }

  // Patient's birth date (required, empty if unknown)
  itk::EncapsulateMetaData<std::string>(dictionary, "0010|0030", patientBirthDate);

  // -----------------------------------------
  // Study tags

  // Study ID (required, empty if unknown)
  // Study instance UID can be a very long string, therefore it is recommended
  // to set a short human-readable study ID.
  if (studyID == "[random]")
    {
    studyID = GenerateRandomCapitalLetters(4);
    }

  itk::EncapsulateMetaData<std::string>(dictionary, "0020|0010", studyID);

  // Accession Number (required, empty if unknown)
  itk::EncapsulateMetaData<std::string>(dictionary, "0008|0050", std::string(""));

  // Study Date (required, empty if unknown)
  if (!studyDate.empty())
    {
    if (studyDate == "[now]")
      {
      studyDate = dateNow;
      }
    }
  itk::EncapsulateMetaData<std::string>(dictionary, "0008|0020", studyDate);

  // Study Time (required, empty if unknown)
  if (!studyTime.empty())
    {
    if (studyTime == "[now]")
      {
      studyTime = timeNow;
      }
    }
  itk::EncapsulateMetaData<std::string>(dictionary, "0008|0030", studyTime);

  // Referring Physician's Name (required, empty if unknown)
  itk::EncapsulateMetaData<std::string>(dictionary, "0008|0090", std::string(""));

  // Study comments
  // This attribute is retired (it should not be used anymore),
  // but it is useful to be able to add a free-form comment to a study
  // therefore we keep this option.
  if (!studyComments.empty())
    {
    itk::EncapsulateMetaData<std::string>(dictionary, "0032|4000", studyComments);
    }

  // Study description (optional)
  if (!studyDescription.empty())
    {
    itk::EncapsulateMetaData<std::string>(dictionary, "0008|1030", studyDescription);
    }

  // -----------------------------------------
  // Device tags

  // Manufacturer (optional)
  if (!manufacturer.empty())
    {
    itk::EncapsulateMetaData<std::string>(dictionary, "0008|0070", manufacturer);
    }

  // Manufacturer's model name (optional)
  if (!model.empty())
    {
    itk::EncapsulateMetaData<std::string>(dictionary, "0008|1090", model);
    }

  // -----------------------------------------
  // Series tags

  // Patient Position (conditionally required, empty if unknown)
  // HFS (head-first supine), FFS, ...
  itk::EncapsulateMetaData<std::string>(dictionary, "0018|5100", patientPosition);

  // Position Reference Indicator (required, empty if unknown)
  itk::EncapsulateMetaData<std::string>(dictionary, "0020|1040", std::string(""));

  // Modality (required)
  itk::EncapsulateMetaData<std::string>(dictionary, "0008|0060", modality);

  // Series number (required, empty if unknown)
  itk::EncapsulateMetaData<std::string>(dictionary, "0020|0011", seriesNumber);

  // Series description (optional)
  if (!seriesDescription.empty())
    {
    itk::EncapsulateMetaData<std::string>(dictionary, "0008|103e", seriesDescription);
    }

  // Series date and time (optional)
  if (!seriesDate.empty())
    {
    if (seriesDate == "[now]")
      {
      seriesDate = dateNow;
      }
    itk::EncapsulateMetaData<std::string>(dictionary, "0008|0021", seriesDate);
    }
  if (!seriesTime.empty())
    {
    if (seriesTime == "[now]")
      {
      seriesTime = timeNow;
      }
    itk::EncapsulateMetaData<std::string>(dictionary, "0008|0031", seriesTime);
    }

  // Content date and time (required if time sequence)
  if (!contentDate.empty())
    {
    if (contentDate == "[now]")
      {
      contentDate = dateNow;
      }
    itk::EncapsulateMetaData<std::string>(dictionary, "0008|0023", contentDate);
    }
  if (!contentTime.empty())
    {
    if (contentTime == "[now]")
      {
      contentTime = timeNow;
      }
    itk::EncapsulateMetaData<std::string>(dictionary, "0008|0033", contentTime);
    }

  // Set study, series, and frame of reference UIDs
  if (studyInstanceUID.empty() && seriesInstanceUID.empty() && frameOfReferenceUID.empty())
    {
    // no UIDs are specified, so we ask ITK DICOM IO to generate them.
    gdcmIO->SetKeepOriginalUID(false);
    }
  else
    {
    // ITK DICOM IO either sets all UIDs or none of them, so we return with error if not all UIDs are specified
    if (studyInstanceUID.empty() || seriesInstanceUID.empty() || frameOfReferenceUID.empty())
      {
      std::cerr << "If any of UIDs (studyInstanceUID, seriesInstanceUID, and frameOfReferenceUID)"
        << " are specified then all of them must be specified." << std::endl;
      return EXIT_FAILURE;
      }
    gdcmIO->SetKeepOriginalUID(true);
    itk::EncapsulateMetaData<std::string>(dictionary, "0020|000d", studyInstanceUID);
    itk::EncapsulateMetaData<std::string>(dictionary, "0020|000e", seriesInstanceUID);
    itk::EncapsulateMetaData<std::string>(dictionary, "0020|0052", frameOfReferenceUID);
    }

  // -----------------------------------------
  // For each slice

  float progress = 1.0 / (float) numberOfSlices;
  for( unsigned int i = 0; i < numberOfSlices; i++ )
    {
    std::cout << "<filter-progress>"
              << (i + 1) * progress
              << "</filter-progress>"
              << std::endl
              << std::flush;

    std::ostringstream value;

    // Instance Number (required, empty if unknown)
    value.str("");
    value << i + 1;
    itk::EncapsulateMetaData<std::string>(dictionary, "0020|0013", value.str());

    // Image Position (Patient)
    typename Image3DType::PointType    origin;
    typename Image3DType::IndexType    index;
    index.Fill(0);
    index[2] = i;
    image->TransformIndexToPhysicalPoint(index, origin);
    value.str("");
    value << origin[0] << "\\" << origin[1] << "\\" << origin[2];
    itk::EncapsulateMetaData<std::string>(dictionary, "0020|0032", value.str() );

    // Image Orientation (Patient)
    value.str("");
    value << oMatrix[0][0] << "\\" << oMatrix[1][0] << "\\" << oMatrix[2][0] << "\\";
    value << oMatrix[0][1] << "\\" << oMatrix[1][1] << "\\" << oMatrix[2][1];
    itk::EncapsulateMetaData<std::string>(dictionary, "0020|0037", value.str() );

    // Slice Thickness
    value.str("");
    value << spacing[2];
    itk::EncapsulateMetaData<std::string>(dictionary, "0018|0050", value.str() );

    // Always set the rescale interscept and rescale slope (even if
    // they are at their defaults of 0 and 1 respectively).
    // value.str("");
    // value << rescaleIntercept;
    // itk::EncapsulateMetaData<std::string>(dictionary, "0028|1052", value.str());
    // value.str("");
    // value << rescaleSlope;
    // itk::EncapsulateMetaData<std::string>(dictionary, "0028|1053", value.str());

    typename Image3DType::RegionType extractRegion;
    typename Image3DType::SizeType   extractSize;
    typename Image3DType::IndexType  extractIndex;
    extractSize = image->GetLargestPossibleRegion().GetSize();
    extractIndex.Fill(0);
    if( reverseImages )
      {
      extractIndex[2] = numberOfSlices - i - 1;
      }
    else
      {
      extractIndex[2] = i;
      }
    extractSize[2] = 0;
    extractRegion.SetSize(extractSize);
    extractRegion.SetIndex(extractIndex);

    typedef itk::ExtractImageFilter<Image3DType, Image2DType> ExtractType;
    typename ExtractType::Pointer extract = ExtractType::New();
    extract->SetDirectionCollapseToGuess();  // ITKv3 compatible, but not recommended
    extract->SetInput(image );
    extract->SetExtractionRegion(extractRegion);
    extract->Update();

    // If window center and width are specified then use the same values for all slices.
    // Otherwise use the full scalar range of voxels in the current slice.
    std::string currentWindowCenter = windowCenter;
    std::string currentWindowWidth = windowWidth;
    if (currentWindowCenter.empty() || currentWindowWidth.empty())
      {
      // Window width and center are required attributes (if VOI LUT sequence is not present), therefore
      // if the value is not specified then set it to include the full range of voxel values.
      itk::ImageRegionIterator<Image2DType> it( extract->GetOutput(), extract->GetOutput()->GetLargestPossibleRegion() );
      typename Image2DType::PixelType                minValue = itk::NumericTraits<typename Image2DType::PixelType>::max();
      typename Image2DType::PixelType                maxValue = itk::NumericTraits<typename Image2DType::PixelType>::min();
      for( it.GoToBegin(); !it.IsAtEnd(); ++it )
        {
        typename Image2DType::PixelType p = it.Get();
        if( p > maxValue )
          {
          maxValue = p;
          }
        if( p < minValue )
          {
          minValue = p;
          }
        }
      double windowCenterValue = (static_cast<double>(minValue) + static_cast<double>(maxValue)) / 2.0;
      double windowWidthValue = (static_cast<double>(maxValue) - static_cast<double>(minValue));

      value.str("");
      value << windowCenterValue;
      currentWindowCenter = value.str();

      value.str("");
      value << windowWidthValue;
      currentWindowWidth = value.str();
      }
    itk::EncapsulateMetaData<std::string>(dictionary, "0028|1050", currentWindowCenter);
    itk::EncapsulateMetaData<std::string>(dictionary, "0028|1051", currentWindowWidth);

    extract->GetOutput()->SetMetaDataDictionary(dictionary);

    typename WriterType::Pointer writer = WriterType::New();
    char                imageNumber[BUFSIZ+1];
    imageNumber[BUFSIZ] = '\0';

#if WIN32
#define snprintf sprintf_s
#endif
    // On Windows, it is hard to pass a string such as "%04d" via command-line, as the % is interpreted as an escape character,
    // therefore we allow the user to omit the leading "%". If the format string does not start with "%" then we add it here.
    if (!dicomNumberFormat.empty())
      {
      if (dicomNumberFormat[0] != '%')
        {
        dicomNumberFormat = "%" + dicomNumberFormat;
        }
      }
    snprintf(imageNumber, BUFSIZ, dicomNumberFormat.c_str(), i + 1);
    value.str("");
    value << dicomDirectory << "/" << dicomPrefix << imageNumber << ".dcm";
    writer->SetFileName(value.str().c_str() );

    writer->SetInput(extract->GetOutput() );
    writer->SetUseCompression(useCompression);
    try
      {
      writer->SetImageIO(gdcmIO);
      writer->Update();
      }
    catch( itk::ExceptionObject & excp )
      {
      std::cerr << "Exception thrown while writing the file " << std::endl;
      std::cerr << excp << std::endl;
      return EXIT_FAILURE;
      }
    }
  std::cout << "<filter-end>" << std::endl;
  std::cout << "<filter-name>ImageFileWriter</filter-name>" << std::endl;
  std::cout << "</filter-end>";
  std::cout << std::flush;

  return EXIT_SUCCESS;
}

} // end of anonymous namespace

int main( int argc, char* argv[] )
{
  PARSE_ARGS;

  try
  {
    if( Type == std::string("Char") )
      {
      return DoIt<char>( argc, argv);
      }
    else if( Type == std::string("UnsignedChar") )
      {
      return DoIt<unsigned char>( argc, argv);
      }
    else if( Type == std::string("Short") )
      {
      return DoIt<short>( argc, argv);
      }
    else if( Type == std::string("UnsignedShort") )
      {
      return DoIt<unsigned short>( argc, argv);
      }
    else if( Type == std::string("Int") )
      {
      return DoIt<int>( argc, argv);
      }
    else if( Type == std::string("UnsignedInt") )
      {
      return DoIt<unsigned int>( argc, argv);
      }
    else
      {
      std::cout << "Unknown type to which to cast input volume: " << Type.c_str() << std::endl;
      return EXIT_FAILURE;
      }
    }
  catch( itk::ExceptionObject & excep )
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
