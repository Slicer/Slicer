
#include "itkImageSeriesReader.h"
#include "itkOrientedImage.h"
#include "itkMetaDataDictionary.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkNumericSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "vtkGlobFileNames.h"

#include "gdcmFile.h"
#include "gdcmGlobal.h"
#include "gdcmUtil.h"
#include "gdcmValEntry.h"
#include "gdcmBinEntry.h"
#include "gdcmSeqEntry.h"
#include "gdcmSQItem.h"

#include "itkImageFileWriter.h"

#include "ComputeSUVBodyWeight_ForRSNACLP.h"

#include "vtkITKArchetypeImageSeriesScalarReader.h"
#include "vtkImageData.h"
#include "vtkImageAccumulate.h"
#include "vtkImageThreshold.h"
#include "vtkImageToImageStencil.h"


// ...
// ...............................................................................................
// ...
/*
SOME NOTES on SUV and parameters of interest:

This is the first-pass implementation we'll make:

Standardized uptake value, SUV, (also referred to as the dose uptake ratio, DUR) is a widely used, simple PET quantifier, calculated as a ratio of tissue radioactivity concentration (e.g. in units kBq/ml) at time T, CPET(T) and injected dose (e.g. in units MBq) at the time of injection divided by body weight (e.g. in units kg).

SUVbw = CPET(T) / (Injected dose / Patient's weight)

Instead of body weight, the injected dose may also be corrected by the lean body mass, or body surface area (BSA) (Kim et al., 1994). Verbraecken et al. (2006) review the different formulas for calculating the BSA.

SUVbsa= CPET(T) / (Injected dose / BSA)

If the above mentioned units are used, the unit of SUV will be g/ml.

===

Later, we can try a more careful computation that includes decay correction:

Most PET systems record their pixels in units of activity concentration (MBq/ml) (once Rescale Slope has been applied, and the units are specified in the Units attribute).

To compute SUVbw, for example, it is necessary to apply the decay formula and account for the patient's weight. For that to be possible, during de-identification, the Patient's Weight must not have been removed, and even though dates may have been removed, it is important not to remove the times, since the difference between the time of injection and the acquisition time is what is important.

In particular, DO NOT REMOVE THE FOLLOWING DICOM TAGS:
Radiopharmaceutical Start Time (0018,1072) Decay Correction (0054,1102) Decay Factor (0054,1321) Frame Reference Time (0054,1300) Radionuclide Half Life (0018,1075) Series Time (0008,0031) Patient's Weight (0010,1030)

Note that to calculate other common SUV values like SUVlbm and SUVbsa, you also need to retain:
Patient's Sex (0010,0040)
Patient's Size (0010,1020)

If there is a strong need to remove times from an identity leakage perspective, then one can normalize all times to some epoch, but it has to be done consistently across all images in the entire study (preferably including the CT reference images); note though, that the time of injection may be EARLIER than the Study Time, which you might assume would be the earliest, so it takes a lot of effort to get this right.

For Philips images, none of this applies, and the images are in COUNTS and the private tag (7053,1000) SUV Factor must be used.
To calculate the SUV of a particular pixel, you just have to calculate [pixel _value * tag 7053|1000 ]

The tag 7053|1000 is a number (double) taking into account the patient's weight, the injection quantity.
We get the tag from the original file with:

double suv;
itk::ExposeMetaData<double>(*dictionary[i], "7053|1000", suv);
*/
// ...
// ...............................................................................................
// ...

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

struct parameters
  {
    std::string PETDICOMPath;
  std::string PETVolumeName;
  std::string VOIVolumeName;
  std::string parameterFile;
  std::string SUVOutputTable;
  std::string patientName;
  std::string studyDate;
  std::string radioactivityUnits;
    std::string tissueRadioactivityUnits;
  std::string weightUnits;
  std::string volumeUnits;
  double injectedDose;
  double calibrationFactor;
  double patientWeight;
  std::string seriesReferenceTime;
  std::string injectionTime;
  std::string decayCorrection;
  std::string decayFactor;
  std::string radionuclideHalfLife;
  std::string frameReferenceTime;
  };

// ...
// ...............................................................................................
// ...

double ConvertTimeToSeconds(const char *time )
{
  if( time == NULL )
    {
    std::cerr << "ConvertTimeToSeconds got a NULL time string." << std::endl;
    return -1.0;
    }

  std::string h;
  std::string m;
  std::string minAndsecStr;
  std::string secStr;

  double hours;
  double minutes;
  double seconds;

  if( time == NULL )
    {
    return 0.0;
    }

  // ---
  // --- time will be in format HH:MM:SS.SSSS
  // --- convert to a double count of seconds.
  // ---
  std::string timeStr = time;
  size_t      i = timeStr.find_first_of(":");
  h = timeStr.substr( 0, 2 );
  hours = atof( h.c_str() );

  minAndsecStr = timeStr.substr( 3 );
  i = minAndsecStr.find_first_of( ":" );
  m = minAndsecStr.substr(0, 2 );
  minutes = atof( m.c_str() );

  secStr = minAndsecStr.substr( 3 );
  seconds = atof( secStr.c_str() );

  double retval = ( seconds
                    + (60.0 * minutes)
                    + (3600.0 * hours ) );
  return retval;
}

// ...
// ...............................................................................................
// ...
double ConvertWeightUnits(double count, const char *fromunits, const char *tounits )
{

  double conversion = count;

  if( fromunits == NULL )
    {
    std::cout << "Got NULL parameter fromunits. A bad param was probably specified." << std::endl;
    return -1.0;
    }
  if( tounits == NULL )
    {
    std::cout << "Got NULL parameter from tounits. A bad parameter was probably specified." << std::endl;
    return -1.0;
    }

  /*
    possibilities include:
  ---------------------------
  "kilograms [kg]"
  "grams [g]"
  "pounds [lb]"
  */

  // --- kg to...
  if( !strcmp(fromunits, "kg") )
    {
    if( !strcmp(tounits, "kg") )
      {
      return conversion;
      }
    else if( !strcmp(tounits, "g") )
      {
      conversion *= 1000.0;
      }
    else if( !strcmp(tounits, "lb") )
      {
      conversion *= 2.2;
      }
    }
  else if( !strcmp(fromunits, "g") )
    {
    if( !strcmp(tounits, "kg") )
      {
      conversion /= 1000.0;
      }
    else if( !strcmp(tounits, "g") )
      {
      return conversion;
      }
    else if( !strcmp(tounits, "lb") )
      {
      conversion *= .0022;
      }
    }
  else if( !strcmp(fromunits, "lb") )
    {
    if( !strcmp(tounits, "kg") )
      {
      conversion *= 0.45454545454545453;
      }
    else if( !strcmp(tounits, "g") )
      {
      conversion *= 454.54545454545453;
      }
    else if( !strcmp(tounits, "lb") )
      {
      return conversion;
      }
    }
  return conversion;

}

// ...
// ...............................................................................................
// ...
double ConvertRadioactivityUnits(double count, const char *fromunits, const char *tounits )
{

  double conversion = count;

  if( fromunits == NULL )
    {
    std::cout << "Got NULL parameter in fromunits. A bad parameter was probably specified." << std::endl;
    return -1.0;
    }
  if( tounits == NULL )
    {
    std::cout << "Got NULL parameter in tounits. A bad parameter was probably specified." << std::endl;
    return -1.0;
    }

/*
  possibilities include:
  ---------------------------
  "megabecquerels [MBq]"
  "kilobecquerels [kBq]"
  "becquerels [Bq]"
  "millibecquerels [mBq]"
  "microbecquerels [uBq]
  "megacuries [MCi]"
  "kilocuries [kCi]"
  "curies [Ci]"
  "millicuries [mCi]"
  "microcuries [uCi]"
*/

  // --- MBq to...
  if( !strcmp(fromunits, "MBq" ) )
    {
    if( !(strcmp(tounits, "MBq" ) ) )
      {
      return conversion;
      }
    else if( !(strcmp(tounits, "kBq" ) ) )
      {
      conversion *= 1000.0;
      }
    else if( !(strcmp(tounits, "Bq" ) ) )
      {
      conversion *= 1000000.0;
      }
    else if( !(strcmp(tounits, "mBq" ) ) )
      {
      conversion *= 1000000000.0;
      }
    else if( !(strcmp(tounits, " uBq" ) ) )
      {
      conversion *= 1000000000000.0;
      }
    else if( !(strcmp(tounits, "MCi" ) ) )
      {
      conversion *= 0.000000000027027027027027;
      }
    else if( !(strcmp(tounits, "kCi" ) ) )
      {
      conversion *= 0.000000027027027027027;
      }
    else if( !(strcmp(tounits, "Ci" ) ) )
      {
      conversion *= 0.000027027027027027;
      }
    else if( !(strcmp(tounits, "mCi" ) ) )
      {
      conversion *= 0.027027027027027;
      }
    else if( !(strcmp(tounits, "uCi" ) ) )
      {
      conversion *= 27.027027027;
      }
    }
  // --- kBq to...
  else if( !strcmp(fromunits, "kBq" ) )
    {
    if( !(strcmp(tounits, "MBq" ) ) )
      {
      conversion *= .001;
      }
    else if( !(strcmp(tounits, "kBq" ) ) )
      {
      return conversion;
      }
    else if( !(strcmp(tounits, "Bq" ) ) )
      {
      conversion *= 1000.0;
      }
    else if( !(strcmp(tounits, "mBq" ) ) )
      {
      conversion *= 1000000.0;
      }
    else if( !(strcmp(tounits, " uBq" ) ) )
      {
      conversion *= 1000000000.0;
      }
    else if( !(strcmp(tounits, "MCi" ) ) )
      {
      conversion *= 0.000000000000027027027027027;
      }
    else if( !(strcmp(tounits, "kCi" ) ) )
      {
      conversion *= 0.000000000027027027027027;
      }
    else if( !(strcmp(tounits, "Ci" ) ) )
      {
      conversion *= 0.000000027027027027027;
      }
    else if( !(strcmp(tounits, "mCi" ) ) )
      {
      conversion *= 0.000027027027027027;
      }
    else if( !(strcmp(tounits, "uCi" ) ) )
      {
      conversion *= 0.027027027027027;
      }
    }
  // --- Bq to...
  else if( !strcmp(fromunits, "Bq" ) )
    {
    if( !(strcmp(tounits, "MBq" ) ) )
      {
      conversion *= 0.000001;
      }
    else if( !(strcmp(tounits, "kBq" ) ) )
      {
      conversion *= 0.001;
      }
    else if( !(strcmp(tounits, "Bq" ) ) )
      {
      return conversion;
      }
    else if( !(strcmp(tounits, "mBq" ) ) )
      {
      conversion *= 1000.0;
      }
    else if( !(strcmp(tounits, " uBq" ) ) )
      {
      conversion *= 1000000.0;
      }
    else if( !(strcmp(tounits, "MCi" ) ) )
      {
      conversion *= 0.000000000000000027027027027027;
      }
    else if( !(strcmp(tounits, "kCi" ) ) )
      {
      conversion *=  0.000000000000027027027027027;
      }
    else if( !(strcmp(tounits, "Ci" ) ) )
      {
      conversion *= 0.000000000027027027027027;
      }
    else if( !(strcmp(tounits, "mCi" ) ) )
      {
      conversion *= 0.000000027027027027027;
      }
    else if( !(strcmp(tounits, "uCi" ) ) )
      {
      conversion *= 0.000027027027027027;
      }
    }
  // --- mBq to...
  else if( !strcmp(fromunits, "mBq" ) )
    {
    if( !(strcmp(tounits, "MBq" ) ) )
      {
      conversion *= 0.000000001;
      }
    else if( !(strcmp(tounits, "kBq" ) ) )
      {
      conversion *= 0.000001;
      }
    else if( !(strcmp(tounits, "Bq" ) ) )
      {
      conversion *= 0.001;
      }
    else if( !(strcmp(tounits, "mBq" ) ) )
      {
      return conversion;
      }
    else if( !(strcmp(tounits, " uBq" ) ) )
      {
      conversion *= 1000.0;
      }
    else if( !(strcmp(tounits, "MCi" ) ) )
      {
      conversion *= 0.00000000000000000002702702702702;
      }
    else if( !(strcmp(tounits, "kCi" ) ) )
      {
      conversion *= 0.000000000000000027027027027027;
      }
    else if( !(strcmp(tounits, "Ci" ) ) )
      {
      conversion *= 0.000000000000027027027027027;
      }
    else if( !(strcmp(tounits, "mCi" ) ) )
      {
      conversion *= 0.000000000027027027027027;
      }
    else if( !(strcmp(tounits, "uCi" ) ) )
      {
      conversion *= 0.000000027027027027027;
      }
    }
  // --- uBq to...
  else if( !strcmp(fromunits, "uBq" ) )
    {
    if( !(strcmp(tounits, "MBq" ) ) )
      {
      conversion *= 0.000000000001;
      }
    else if( !(strcmp(tounits, "kBq" ) ) )
      {
      conversion *= 0.000000001;
      }
    else if( !(strcmp(tounits, "Bq" ) ) )
      {
      conversion *= 0.000001;
      }
    else if( !(strcmp(tounits, "mBq" ) ) )
      {
      conversion *= 0.001;
      }
    else if( !(strcmp(tounits, " uBq" ) ) )
      {
      return conversion;
      }
    else if( !(strcmp(tounits, "MCi" ) ) )
      {
      conversion *= 0.000000000000000000000027027027027027;
      }
    else if( !(strcmp(tounits, "kCi" ) ) )
      {
      conversion *= 0.000000000000000000027027027027027;
      }
    else if( !(strcmp(tounits, "Ci" ) ) )
      {
      conversion *= 0.000000000000000027027027027027;
      }
    else if( !(strcmp(tounits, "mCi" ) ) )
      {
      conversion *= 0.000000000000027027027027027;
      }
    else if( !(strcmp(tounits, "uCi" ) ) )
      {
      conversion *= 0.000000000027027027027027;
      }
    }
  // --- MCi to...
  else if( !strcmp(fromunits, "MCi" ) )
    {
    if( !(strcmp(tounits, "MBq" ) ) )
      {
      conversion *= 37000000000.0;
      }
    else if( !(strcmp(tounits, "kBq" ) ) )
      {
      conversion *= 37000000000000.0;
      }
    else if( !(strcmp(tounits, "Bq" ) ) )
      {
      conversion *= 37000000000000000.0;
      }
    else if( !(strcmp(tounits, "mBq" ) ) )
      {
      conversion *= 37000000000000000000.0;
      }
    else if( !(strcmp(tounits, " uBq" ) ) )
      {
      conversion *=  37000000000000000000848.0;
      }
    else if( !(strcmp(tounits, "MCi" ) ) )
      {
      return conversion;
      }
    else if( !(strcmp(tounits, "kCi" ) ) )
      {
      conversion *= 1000.0;
      }
    else if( !(strcmp(tounits, "Ci" ) ) )
      {
      conversion *= 1000000.0;
      }
    else if( !(strcmp(tounits, "mCi" ) ) )
      {
      conversion *= 1000000000.0;
      }
    else if( !(strcmp(tounits, "uCi" ) ) )
      {
      conversion *= 1000000000000.0;
      }
    }
  // --- kCi to...
  else if( !strcmp(fromunits, "kCi" ) )
    {
    if( !(strcmp(tounits, "MBq" ) ) )
      {
      conversion *= 37000000.0;
      }
    else if( !(strcmp(tounits, "kBq" ) ) )
      {
      conversion *= 37000000000.0;
      }
    else if( !(strcmp(tounits, "Bq" ) ) )
      {
      conversion *= 37000000000000.0;
      }
    else if( !(strcmp(tounits, "mBq" ) ) )
      {
      conversion *= 37000000000000000.0;
      }
    else if( !(strcmp(tounits, " uBq" ) ) )
      {
      conversion *= 37000000000000000000.0;
      }
    else if( !(strcmp(tounits, "MCi" ) ) )
      {
      conversion *= 0.001;
      }
    else if( !(strcmp(tounits, "kCi" ) ) )
      {
      return conversion;
      }
    else if( !(strcmp(tounits, "Ci" ) ) )
      {
      conversion *= 1000.0;
      }
    else if( !(strcmp(tounits, "mCi" ) ) )
      {
      conversion *= 1000000.0;
      }
    else if( !(strcmp(tounits, "uCi" ) ) )
      {
      conversion *= 1000000000.0;
      }
    }
  // --- Ci to...
  else if( !strcmp(fromunits, "Ci" ) )
    {
    if( !(strcmp(tounits, "MBq" ) ) )
      {
      conversion *= 37000.0;
      }
    else if( !(strcmp(tounits, "kBq" ) ) )
      {
      conversion *= 37000000.0;
      }
    else if( !(strcmp(tounits, "Bq" ) ) )
      {
      conversion *= 37000000000.0;
      }
    else if( !(strcmp(tounits, "mBq" ) ) )
      {
      conversion *= 37000000000000.0;
      }
    else if( !(strcmp(tounits, " uBq" ) ) )
      {
      conversion *= 37000000000000000.0;
      }
    else if( !(strcmp(tounits, "MCi" ) ) )
      {
      conversion *= 0.0000010;
      }
    else if( !(strcmp(tounits, "kCi" ) ) )
      {
      conversion *= 0.001;
      }
    else if( !(strcmp(tounits, "Ci" ) ) )
      {
      return conversion;
      }
    else if( !(strcmp(tounits, "mCi" ) ) )
      {
      conversion *= 1000.0;
      }
    else if( !(strcmp(tounits, "uCi" ) ) )
      {
      conversion *= 1000000.0;
      }
    }
  // --- mCi to...
  else if( !strcmp(fromunits, "mCi" ) )
    {
    if( !(strcmp(tounits, "MBq" ) ) )
      {
      conversion *= 37.0;
      }
    else if( !(strcmp(tounits, "kBq" ) ) )
      {
      conversion *= 37000.0;
      }
    else if( !(strcmp(tounits, "Bq" ) ) )
      {
      conversion *= 37000000.0;
      }
    else if( !(strcmp(tounits, "mBq" ) ) )
      {
      conversion *= 37000000000.0;
      }
    else if( !(strcmp(tounits, " uBq" ) ) )
      {
      conversion *= 37000000000000.0;
      }
    else if( !(strcmp(tounits, "MCi" ) ) )
      {
      conversion *= 0.0000000010;
      }
    else if( !(strcmp(tounits, "kCi" ) ) )
      {
      conversion *= 0.0000010;
      }
    else if( !(strcmp(tounits, "Ci" ) ) )
      {
      conversion *= 0.001;
      }
    else if( !(strcmp(tounits, "mCi" ) ) )
      {
      return conversion;
      }
    else if( !(strcmp(tounits, "uCi" ) ) )
      {
      conversion *= 1000.0;
      }
    }
  // --- uCi to...
  else if( !strcmp(fromunits, " uCi" ) )
    {
    if( !(strcmp(tounits, "MBq" ) ) )
      {
      conversion *= 0.037;
      }
    else if( !(strcmp(tounits, "kBq" ) ) )
      {
      conversion *= 37.0;
      }
    else if( !(strcmp(tounits, "Bq" ) ) )
      {
      conversion *= 37000.0;
      }
    else if( !(strcmp(tounits, "mBq" ) ) )
      {
      conversion *= 37000000.0;
      }
    else if( !(strcmp(tounits, " uBq" ) ) )
      {
      conversion *= 37000000000.0;
      }
    else if( !(strcmp(tounits, "MCi" ) ) )
      {
      conversion *= 0.0000000000010;
      }
    else if( !(strcmp(tounits, "kCi" ) ) )
      {
      conversion *= 0.0000000010;
      }
    else if( !(strcmp(tounits, "Ci" ) ) )
      {
      conversion *= 0.0000010;
      }
    else if( !(strcmp(tounits, "mCi" ) ) )
      {
      conversion *= 0.001;
      }
    else if( !(strcmp(tounits, "uCi" ) ) )
      {
      return conversion;
      }
    }

  return conversion;
}

// ...
// ...............................................................................................
// ...
double DecayCorrection(parameters & list, double inVal )
{

  double scanTimeSeconds = ConvertTimeToSeconds(list.seriesReferenceTime.c_str() );
  double startTimeSeconds = ConvertTimeToSeconds( list.injectionTime.c_str() );
  double halfLife = atof( list.radionuclideHalfLife.c_str() );
  double decayTime = scanTimeSeconds - startTimeSeconds;
  double correctedVal = inVal * (double)pow(2.0, -(decayTime / halfLife) );

  return correctedVal;
}

// ...
// ...............................................................................................
// ...
const char * MapLabelIDtoColorName( int id )
{

  // This method is used with a hardcoded colormap for now
  // for Horky PETCT project.
  // TODO: finalize atlas to be used and take that from command line.
  const char *name;

  switch( id )
    {
    case 0:
      name = "Background";
      break;
    case 1:
      name = "R_caudate_head";
      break;
    case 2:
      name = "L_caudate_head";
      break;
    case 3:
      name = "R_thalamus";
      break;
    case 4:
      name = "L_thalamus";
      break;
    case 5:
      name = "R_frontal_cortex";
      break;
    case 6:
      name = "L_frontal_cortex";
      break;
    case 7:
      name = "R_parietal_cortex";
      break;
    case 8:
      name = "L_parietal_cortex";
      break;
    case 9:
      name = "R_cerebellum";
      break;
    case 10:
      name = "L_cerebellum";
      break;
    case 11:
      name = "R_hippo";
      break;
    case 12:
      name = "L_hippo";
      break;
    case 13:
      name = "R_paraventric_WM";
      break;
    case 14:
      name = "L_paraventric_WM";
      break;
    case 15:
      name = "CC";
      break;
    case 16:
      name = "R_olfactory_gyrus";
      break;
    case 17:
      name = "L_olfactory_gyrus";
      break;
    case 18:
      name = "All_CSF_space";
      break;
    case 19:
      name = "All_white_matter";
      break;
    default:
      name = "";
      break;
    }
  return name;
}

// ...
// ...............................................................................................
// ...
template <class T>
int LoadImagesAndComputeSUV( parameters & list, T )
{


  typedef    T                           InputPixelType;
  typedef itk::Image<InputPixelType,  3> InputImageType;

  typedef itk::Image<unsigned char, 3> LabelImageType;

  typedef    T                           OutputPixelType;
  typedef itk::Image<OutputPixelType, 3> OutputImageType;

  typedef itk::ImageFileReader<InputImageType>  ReaderType;
  typedef itk::ImageFileReader<LabelImageType>  LabelReaderType;
  typedef itk::ImageFileWriter<OutputImageType> WriterType;

  //
  // for writing csv output files
  //
  std::string   outputFile = list.SUVOutputTable;
  std::ofstream ofile;

  vtkImageData *                    petVolume;
  vtkImageData *                    voiVolume;
  vtkITKArchetypeImageSeriesReader *reader1 = NULL;
  vtkITKArchetypeImageSeriesReader *reader2 = NULL;

  // check for the input files
  FILE * petfile;
  petfile = fopen(list.PETVolumeName.c_str(), "r");
  if( petfile == NULL )
    {
    std::cerr << "ERROR: cannot open input volume file " << list.PETVolumeName.c_str() << endl;
    return EXIT_FAILURE;
    }
  fclose(petfile);

  FILE * voifile;
  voifile = fopen(list.VOIVolumeName.c_str(), "r");
  if( voifile == NULL )
    {
    std::cerr << "ERROR: cannot open input volume file " << list.VOIVolumeName.c_str() << endl;
    return EXIT_FAILURE;
    }
  fclose(voifile);

  // Read the PET file
  
  reader1 = vtkITKArchetypeImageSeriesScalarReader::New();
//    vtkPluginFilterWatcher watchReader1 ( reader1, "Reading PET Volume", CLPProcessInformation );
  reader1->SetArchetype(list.PETVolumeName.c_str() );
  reader1->SetOutputScalarTypeToNative();
  reader1->SetDesiredCoordinateOrientationToNative();
  reader1->SetUseNativeOriginOn();
  reader1->Update();
  std::cout << "Done reading the file " << list.PETVolumeName.c_str() << endl;
  

  // Read the VOI file
  reader2 = vtkITKArchetypeImageSeriesScalarReader::New();
//    vtkPluginFilterWatcher watchReader2 ( reader2, "Reading VOI Volume", CLPProcessInformation );
  reader2->SetArchetype(list.VOIVolumeName.c_str() );
  reader2->SetOutputScalarTypeToNative();
  reader2->SetDesiredCoordinateOrientationToNative();
  reader2->SetUseNativeOriginOn();
  reader2->Update();
  std::cout << "Done reading the file " << list.VOIVolumeName.c_str() << endl;

  // stuff the images.
  reader1->Update();
  reader2->Update();
  petVolume = reader1->GetOutput();
  petVolume->Update();
  voiVolume = reader2->GetOutput();
  voiVolume->Update();


  //
  // COMPUTE SUV ///////////////////////////////////////////////////////////////////////////////RSNA CHANGE//////////////////////////
  //

  if( petVolume == NULL )
    {
    std::cerr << "No input PET volume found." << std::endl;
    return EXIT_FAILURE;
    }

  // find input labelmap volume
  if( voiVolume == NULL )
    {
    std::cerr <<  "No input VOI volume found" << std::endl;
    return EXIT_FAILURE;
    }

  // read the DICOM dir to get the radiological data

  typedef short PixelValueType;
  typedef itk::OrientedImage< PixelValueType, 3 > VolumeType;
  typedef itk::ImageSeriesReader< VolumeType > VolumeReaderType;
  typedef itk::OrientedImage< PixelValueType, 2 > SliceType;
  typedef itk::ImageFileReader< SliceType > SliceReaderType;
  typedef itk::GDCMImageIO ImageIOType;
  typedef itk::GDCMSeriesFileNames InputNamesGeneratorType;
  typedef itk::VectorImage< PixelValueType, 3 > NRRDImageType;
    
  if ( !list.PETDICOMPath.compare(""))
    {
    std::cerr << "GetParametersFromDicomHeader:Got empty list.PETDICOMPath." << std::endl;
    return EXIT_FAILURE;
    }
 

  //--- catch non-dicom data
  vtkGlobFileNames* gfn = vtkGlobFileNames::New();
  gfn->SetDirectory(list.PETDICOMPath.c_str());
  gfn->AddFileNames("*.nhdr");
  gfn->AddFileNames("*.nrrd");
  gfn->AddFileNames("*.hdr");
  gfn->AddFileNames("*.mha");
  gfn->AddFileNames("*.img");
  gfn->AddFileNames("*.nii");
  gfn->AddFileNames("*.nia");

  int notDICOM = 0;
  int nFiles = gfn->GetNumberOfFileNames();
  if (nFiles > 0)
    {

    //--- invoke error via the node
    /*this->PETCTFusionNode->SetInjectedDose ( 0.0 );
    this->PETCTFusionNode->SetPatientWeight( 0.0 );
    this->PETCTFusionNode->SetNumberOfTemporalPositions( 0 );
    this->PETCTFusionNode->SetSeriesTime( "" );
    this->PETCTFusionNode->SetRadiopharmaceuticalStartTime( "" );
    this->PETCTFusionNode->SetFrameReferenceTime( "" );
    this->PETCTFusionNode->SetDecayCorrection( "" );
    this->PETCTFusionNode->SetDecayFactor( "" );
    this->PETCTFusionNode->SetRadionuclideHalfLife( "" );
    this->PETCTFusionNode->SetPhilipsSUVFactor( "" );
    notDICOM = 1;
    */
    // use the list parameter input 
/*
    double InjectedDose = 0.0;
    double PatientWeight = 0.0 ;
    int NumberOfTemporalPositions = 0;
    string SeriesTime =  "";
    string RadiopharmaceuticalStartTime = "";
    string FrameReferenceTime = "";
    string DecayCorrection = "";
    string DecayFactor = "";
    string SetRadionuclideHalfLife = "";
    string SetPhilipsSUVFactor = "";
*/
    notDICOM = 1;
    }
  gfn->Delete();
  if ( notDICOM )
    {
    //--- Tell the user they will have to enter
    //--- parameters for computing SUV manually.
      // this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::NonDICOMEvent );
      //return 0;
      return EXIT_FAILURE;
    }

  
  InputNamesGeneratorType::Pointer inputNames = InputNamesGeneratorType::New();
  inputNames->SetUseSeriesDetails(true);
  inputNames->SetDirectory(list.PETDICOMPath);
  itk::SerieUIDContainer seriesUIDs = inputNames->GetSeriesUIDs();
  
//  this->PETCTFusionNode->SetMessageText ( "Getting Filenames...." );
//  this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::StatusEvent );

  const VolumeReaderType::FileNamesContainer & filenames = inputNames->GetFileNames(seriesUIDs[0]);
      
    std::string tag;
    std::string yearstr;
    std::string monthstr;
    std::string daystr;
    std::string hourstr;
    std::string minutestr;
    std::string secondstr;
    int len;
    
// Nuclear Medicine DICOM info:
/*
    0054,0016  Radiopharmaceutical Information Sequence:
    0018,1072  Radionuclide Start Time: 090748.000000
    0018,1074  Radionuclide Total Dose: 370500000
    0018,1075  Radionuclide Half Life: 6586.2
    0018,1076  Radionuclide Positron Fraction: 0
*/
    int parsingDICOM = 0;
    gdcm::File *f = new gdcm::File();
    if ( f != NULL )
      {
      const char *fn = filenames[0].c_str();
      f->SetFileName( fn );
      //bool res = f->Load();   // FIXME: commented out for now to avoid compile warnings
      f->Load();   // FIXME: handle res

      gdcm::SeqEntry *seq = f->GetSeqEntry(0x0054,0x0016);
      if ( seq != NULL )
        {
        parsingDICOM = 1;
        gdcm::SQItem *sqItem = seq->GetFirstSQItem();
        while ( sqItem )
          {
          //---
          //--- Radiopharmaceutical Start Time
          tag.clear();
          tag = sqItem->GetEntryValue(0x0018,0x1072);
          //--- expect A string of characters of the format hhmmss.frac;
          //---where hh contains hours (range "00" - "23"), mm contains minutes
          //---(range "00" - "59"), ss contains seconds (range "00" - "59"), and frac
          //---contains a fractional part of a second as small as 1 millionth of a
          //---second (range "000000" - "999999"). A 24 hour clock is assumed.
          //---Midnight can be represented by only "0000" since "2400" would
          //---violate the hour range. The string may be padded with trailing
          //---spaces. Leading and embedded spaces are not allowed. One
          //---or more of the components mm, ss, or frac may be unspecified
          //---as long as every component to the right of an unspecified
          //---component is also unspecified. If frac is unspecified the preceding "."
          //---may not be included. Frac shall be held to six decimal places or
          //---less to ensure its format conforms to the ANSI 
          //---Examples -
          //---1. "070907.0705" represents a time of 7 hours, 9 minutes and 7.0705 seconds.
          //---2. "1010" represents a time of 10 hours, and 10 minutes.
          //---3. "021" is an invalid value. 
          if ( tag.c_str() == NULL || *(tag.c_str()) == '\0' )
            {
            //this->PETCTFusionNode->SetRadiopharmaceuticalStartTime ("no value found");
            //list.injectionTime  = "no value found" ;
// already init
            }
          else
            {
            len = tag.length();
            hourstr.clear();
            minutestr.clear();
            secondstr.clear();
            if ( len >= 2 )
              {
              hourstr = tag.substr(0, 2);
              }
            else
              {
              hourstr = "00";
              }
            if ( len >= 4 )
              {
              minutestr = tag.substr(2, 2);
              }
            else
              {
              minutestr = "00";
              }
            if ( len >= 6 )
              {
              secondstr = tag.substr(4);
              }
            else
              {
              secondstr = "00";
              }
            tag.clear();
            tag = hourstr.c_str();
            tag += ":";
            tag += minutestr.c_str();
            tag += ":";
            tag += secondstr.c_str();
            //this->PETCTFusionNode->SetRadiopharmaceuticalStartTime( tag.c_str() );
        list.injectionTime = tag.c_str();
            }

          //---
          //--- Radionuclide Total Dose 
          tag.clear();
          tag = sqItem->GetEntryValue(0x0018,0x1074);
          if ( tag.c_str() == NULL || *(tag.c_str()) == '\0' )
            {
            // this->PETCTFusionNode->SetInjectedDose( 0.0 );
            list.injectedDose = 0.0;
            }
          else
            {
            //this->PETCTFusionNode->SetInjectedDose( atof ( tag.c_str() ) );
            list.injectedDose = atof ( tag.c_str() ) ;
            }


          //---
          //--- RadionuclideHalfLife
          tag.clear();
          tag = sqItem->GetEntryValue(0x0018,0x1075);
          //--- Expect a Decimal String
          //--- A string of characters representing either
          //--- a fixed point number or a floating point number.
          //--- A fixed point number shall contain only the characters 0-9
          //--- with an optional leading "+" or "-" and an optional "." to mark
          //--- the decimal point. A floating point number shall be conveyed
          //--- as defined in ANSI X3.9, with an "E" or "e" to indicate the start
          //--- of the exponent. Decimal Strings may be padded with leading
          //--- or trailing spaces. Embedded spaces are not allowed. 
          if ( tag.c_str() == NULL || *(tag.c_str()) == '\0' )
            {
          // this->PETCTFusionNode->SetRadionuclideHalfLife( "no value found" );
//        list.radionuclideHalfLife = "no value found" ;
            }
          else
            {
          //this->PETCTFusionNode->SetRadionuclideHalfLife(  tag.c_str() );
          list.radionuclideHalfLife =  tag.c_str() ;
            }

          //---
          //---Radionuclide Positron Fraction
          tag.clear();
          tag = sqItem->GetEntryValue(0x0018,0x1076);
          //--- not currently using this one?

          sqItem = seq->GetNextSQItem();
          }

        //--
        //--- UNITS: something like BQML:
        //--- CNTS, NONE, CM2, PCNT, CPS, BQML,
        //--- MGMINML, UMOLMINML, MLMING, MLG,
        //--- 1CM, UMOLML, PROPCNTS, PROPCPS,
        //--- MLMINML, MLML, GML, STDDEV      
        //---
        tag.clear();
        tag = f->GetEntryValue (0x0054,0x1001);
        if ( tag.c_str() != NULL && strcmp(tag.c_str(), "" ) )
          {
          //--- I think these are piled together. MBq ml... search for all.
          std::string units = tag.c_str();
          if ( ( units.find ("BQML") != std::string::npos) ||
               ( units.find ("BQML") != std::string::npos) )
            {
            // this->PETCTFusionNode->SetDoseRadioactivityUnits ("Bq");
            // this->PETCTFusionNode->SetTissueRadioactivityUnits ("Bq");
            list.radioactivityUnits= "Bq";        
            list.tissueRadioactivityUnits = "Bq";
            }
          else if ( ( units.find ("MBq") != std::string::npos) ||
                    ( units.find ("MBQ") != std::string::npos) )
            {
          //this->PETCTFusionNode->SetDoseRadioactivityUnits ("MBq");
          // this->PETCTFusionNode->SetTissueRadioactivityUnits ("MBq");
            list.radioactivityUnits = "MBq";
            list.tissueRadioactivityUnits = "MBq";
            }
          else if ( (units.find ("kBq") != std::string::npos) ||
                    (units.find ("kBQ") != std::string::npos) ||
                    (units.find ("KBQ") != std::string::npos) )
            {
          //  this->PETCTFusionNode->SetDoseRadioactivityUnits ("kBq");
          //  this->PETCTFusionNode->SetTissueRadioactivityUnits ("kBq");

            list.radioactivityUnits = "kBq";
            list.tissueRadioactivityUnits = "kBq";

            }
          else if ( (units.find ("mBq") != std::string::npos) ||
                    (units.find ("mBQ") != std::string::npos) )
            {
          //  this->PETCTFusionNode->SetDoseRadioactivityUnits ("mBq");
          //  this->PETCTFusionNode->SetTissueRadioactivityUnits ("mBq");

            list.radioactivityUnits = "mBq";
            list.tissueRadioactivityUnits = "mBq";


            }
          else if ( (units.find ("uBq") != std::string::npos) ||
                    (units.find ("uBQ") != std::string::npos) )
            {
          //  this->PETCTFusionNode->SetDoseRadioactivityUnits ("uBq");
          //  this->PETCTFusionNode->SetTissueRadioactivityUnits ("uBq");

            list.radioactivityUnits = "uBq";
            list.tissueRadioactivityUnits = "uBq";

            }
          else if ( (units.find ("Bq") != std::string::npos) ||
                    (units.find ("BQ") != std::string::npos) )
            {
          //   this->PETCTFusionNode->SetDoseRadioactivityUnits ("Bq");
          //  this->PETCTFusionNode->SetTissueRadioactivityUnits ("Bq");

            list.radioactivityUnits = "Bq";
            list.tissueRadioactivityUnits = "Bq";


            }
          else if ( (units.find ("MCi") != std::string::npos) ||
                    ( units.find ("MCI") != std::string::npos) )
            {
          //  this->PETCTFusionNode->SetDoseRadioactivityUnits ("MCi");
          // this->PETCTFusionNode->SetTissueRadioactivityUnits ("MCi");

            list.radioactivityUnits = "MCi";
            list.tissueRadioactivityUnits = "MCi";

            }
          else if ( (units.find ("kCi") != std::string::npos) ||
                    (units.find ("kCI") != std::string::npos)  ||
                    (units.find ("KCI") != std::string::npos) )                
            {
          //     this->PETCTFusionNode->SetDoseRadioactivityUnits ("kCi");
          //     this->PETCTFusionNode->SetTissueRadioactivityUnits ("kCi");

            list.radioactivityUnits = "kCi";
            list.tissueRadioactivityUnits = "kCi";

            }
          else if ( (units.find ("mCi") != std::string::npos) ||
                    (units.find ("mCI") != std::string::npos) )                
            {
          //  this->PETCTFusionNode->SetDoseRadioactivityUnits ("mCi");
          //  this->PETCTFusionNode->SetTissueRadioactivityUnits ("mCi");

            list.radioactivityUnits = "mCi";
            list.tissueRadioactivityUnits = "mCi";


            }
          else if ( (units.find ("uCi") != std::string::npos) ||
                    (units.find ("uCI") != std::string::npos) )                
            {
          //  this->PETCTFusionNode->SetDoseRadioactivityUnits ("uCi");
          //  this->PETCTFusionNode->SetTissueRadioactivityUnits ("uCi");

            list.radioactivityUnits = "uCi";
            list.tissueRadioactivityUnits = "uCi";

            }
          else if ( (units.find ("Ci") != std::string::npos) ||
                    (units.find ("CI") != std::string::npos) )                
            {
          // this->PETCTFusionNode->SetDoseRadioactivityUnits ("Ci");
          // this->PETCTFusionNode->SetTissueRadioactivityUnits ("Ci");

            list.radioactivityUnits = "Ci";
            list.tissueRadioactivityUnits = "Ci";
            }
          //this->PETCTFusionNode->SetVolumeUnits ( "ml" );

          list.volumeUnits = "ml";

          }
        else
          {
          //--- default values.
        // this->PETCTFusionNode->SetDoseRadioactivityUnits( "MBq" );
        //this->PETCTFusionNode->SetTissueRadioactivityUnits( "MBq" );
        // this->PETCTFusionNode->SetVolumeUnits ( "ml");     

          list.radioactivityUnits = "MBq";
          list.tissueRadioactivityUnits = "MBq";
          list.volumeUnits = "ml";
   
          }

    
        //---
        //--- DecayCorrection
        //--- Possible values are:
        //--- NONE = no decay correction
        //--- START= acquisition start time
        //--- ADMIN = radiopharmaceutical administration time
        //--- Frame Reference Time  is the time that the pixel values in the Image occurred. 
        //--- It's defined as the time offset, in msec, from the Series Reference Time.
        //--- Series Reference Time is defined by the combination of:
        //--- Series Date (0008,0021) and
        //--- Series Time (0008,0031).      
        //--- We don't pull these out now, but can if we have to.
        tag.clear();
        tag = f->GetEntryValue (0x0054,0x1102);
        if ( tag.c_str() != NULL && strcmp(tag.c_str(), "" ) )
          {
          //---A string of characters with leading or trailing spaces (20H) being non-significant. 
          list.decayCorrection = tag.c_str();
          }
        else
          {
          list.decayCorrection = "no value found";
          }

        //---
        //--- StudyDate
//        this->ClearStudyDate();
        tag.clear();
        tag = f->GetEntryValue (0x0008,0x0021);
        if ( tag.c_str() != NULL && strcmp (tag.c_str(), "" ) )
          {
          //--- YYYYMMDD
          yearstr.clear();
          daystr.clear();
          monthstr.clear();
          len = tag.length();
          if ( len >= 4 )
            {
            yearstr = tag.substr(0, 4);
            // this->Year = atoi(yearstr.c_str() );
            }
          else
            {
            yearstr = "????";
            // this->Year = 0;
            }
          if ( len >= 6 )
            {
            monthstr = tag.substr(4, 2);
            // this->Month = atoi ( monthstr.c_str() );
            }
          else
            {
            monthstr = "??";
            // this->Month = 0;
            }
          if ( len >= 8 )
            {
            daystr = tag.substr (6, 2);
//            this->Day = atoi ( daystr.c_str() );
            }
          else
            {
            daystr = "??";
//            this->Day = 0;
            }
          tag.clear();
          tag = yearstr.c_str();
          tag += "/";
          tag += monthstr.c_str();
          tag += "/";
          tag += daystr.c_str();
          list.studyDate = tag.c_str();
          }
        else
          {
          list.studyDate = "no value found";
          }

        //---
        //--- PatientName
        tag.clear();
        tag = f->GetEntryValue (0x0010,0x0010);
        if ( tag.c_str() != NULL && strcmp (tag.c_str(), "" ) )
          {
          list.patientName = tag.c_str();
          }
        else
          {
          list.patientName = "no value found";
          }

        //---
        //--- DecayFactor
        tag.clear();
        tag = f->GetEntryValue (0x0054,0x1321);
        if ( tag.c_str() != NULL && strcmp(tag.c_str(), "" ) )
          {
          //--- have to parse this out. what we have is
          //---A string of characters representing either a fixed point number or a
          //--- floating point number. A fixed point number shall contain only the
          //---characters 0-9 with an optional leading "+" or "-" and an optional "."
          //---to mark the decimal point. A floating point number shall be conveyed
          //---as defined in ANSI X3.9, with an "E" or "e" to indicate the start of the
          //---exponent. Decimal Strings may be padded with leading or trailing spaces.
          //---Embedded spaces are not allowed. or maybe atof does it already...
          list.decayFactor =  tag.c_str() ;
          }
        else
          {
          list.decayFactor =  "no value found" ;
          }

    
        //---
        //--- FrameReferenceTime
        tag.clear();
        tag = f->GetEntryValue (0x0054,0x1300);
        if ( tag.c_str() != NULL && strcmp(tag.c_str(), "" ) )
          {
          //--- The time that the pixel values in the image
          //--- occurred. Frame Reference Time is the
          //--- offset, in msec, from the Series reference
          //--- time.
          list.frameReferenceTime = tag.c_str();
          }
        else
          {
          list.frameReferenceTime = "no value found";
          }

  
        //---
        //--- SeriesTime
        tag.clear();
        tag = f->GetEntryValue (0x0008,0x0031);
        if ( tag.c_str() != NULL && strcmp(tag.c_str(), "" ) )
          {
          hourstr.clear();
          minutestr.clear();
          secondstr.clear();
          len = tag.length();
          if ( len >= 2 )
            {
            hourstr = tag.substr(0, 2);
            }
          else
            {
            hourstr = "00";
            }
          if ( len >= 4 )
            {
            minutestr = tag.substr(2, 2);
            }
          else
            {
            minutestr = "00";
            }
          if ( len >= 6 )
            {
            secondstr = tag.substr(4);
            }
          else
            {
            secondstr = "00";
            }
          tag.clear();
          tag = hourstr.c_str();
          tag += ":";
          tag += minutestr.c_str();
          tag += ":";
          tag += secondstr.c_str();
          //this->PETCTFusionNode->SetSeriesTime( tag.c_str() );
          list.seriesReferenceTime = tag.c_str();
          }
        else
          {
          //this->PETCTFusionNode->SetSeriesTime( "no value found");
          list.seriesReferenceTime = "no value found";
          }


        //---
        //--- PatientWeight
        tag.clear();
        tag = f->GetEntryValue (0x0010,0x1030);
        if ( tag.c_str() != NULL && strcmp(tag.c_str(), "" ) )
          {
          //--- Expect same format as RadionuclideHalfLife
//        this->PETCTFusionNode->SetPatientWeight( atof ( tag.c_str() ) );
//        this->PETCTFusionNode->SetWeightUnits ( "kg" );
          list.patientWeight = atof ( tag.c_str() );
          list.weightUnits = "kg";
          }
        else
          {
//          this->PETCTFusionNode->SetPatientWeight( 0.0 );
//          this->PETCTFusionNode->SetWeightUnits ( "" );
          list.patientWeight = 0.0;
          list.weightUnits = "";
          
          }


        //---
        //--- CalibrationFactor
        tag.clear();
        tag = f->GetEntryValue (0x7053,0x1009);
        if ( tag.c_str() != NULL && strcmp(tag.c_str(), "" ) )
          {
          //--- converts counts to Bq/cc. If Units = BQML then CalibrationFactor =1 
          //--- I think we expect the same format as RadiopharmaceuticalStartTime
          //this->PETCTFusionNode->SetCalibrationFactor(  tag.c_str() );
          list.calibrationFactor =  atof(tag.c_str());
          }
        else
          {
          //this->PETCTFusionNode->SetCalibrationFactor( "no value found" );
          list.calibrationFactor =  0.0 ;
          }


        //---
        //--- PhilipsSUVFactor
        /*
        tag.clear();
        tag = f->GetEntryValue (0x7053,0x1000);
        if ( tag.c_str() != NULL && strcmp(tag.c_str(), "" ) )
          {
          //--- I think we expect the same format as RadiopharmaceuticalStartTime
          this->PETCTFusionNode->SetPhilipsSUVFactor(  tag.c_str() );
          }
        else
          {
          this->PETCTFusionNode->SetPhilipsSUVFactor( "no value found" );
          }
        */
        }
      }
    //END TEST
    delete f;


    // check.... did we get all params we need for computation?
    if ( (parsingDICOM) &&
         (list.injectedDose != 0.0) &&
         (list.patientWeight != 0.0) &&
         (list.seriesReferenceTime.compare("") != 0) &&
         (list.injectionTime.compare("") != 0) &&
         (list.radionuclideHalfLife.compare("") != 0) )
      {
      /*
      this->PETCTFusionNode->SetMessageText ( "...Done" );
      this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::StatusEvent );

      this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::DICOMUpdateEvent );

      this->PETCTFusionNode->SetMessageText ( "" );
      this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::StatusEvent
      );
      */
//      return 1;
      }
    else
      {
      /*
      this->PETCTFusionNode->SetMessageText ( "...no PET-related metadata found." );
      this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::StatusEvent );

      this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::DICOMUpdateEvent );

      this->PETCTFusionNode->SetMessageText ( "" );
      this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::StatusEvent
      );
      return 0;
      */
      std::cerr << "Missing some parameters..." << std::endl;
      return EXIT_FAILURE;
      }


    

  // convert from input units.
  if( list.radioactivityUnits.c_str() == NULL )
    {
    std::cerr << "ComputeSUV: Got NULL radioactivity units. No computation done." << std::endl;
    return EXIT_FAILURE;
    }
  if( list.weightUnits.c_str() == NULL )
    {
    std::cerr << "ComputeSUV: Got NULL weight units. No computation could be done." << std::endl;
    return EXIT_FAILURE;
    }

  double suvmax, suvmin, suvmean;

  // --- find the max and min label in mask
  vtkImageAccumulate *stataccum = vtkImageAccumulate::New();
  stataccum->SetInput( voiVolume );
  stataccum->Update();
  int lo = static_cast<int>(stataccum->GetMin()[0]);
  int hi = static_cast<int>(stataccum->GetMax()[0]);
  stataccum->Delete();

  std::string labelName;
  int         NumberOfVOIs = 0;
  for( int i = lo; i <= hi; i++ )
    {
    std::stringstream ss;
    if( i == 0 )
      {
      // --- eliminate 0 (background) label.
      continue;
      }

    labelName.clear();
    labelName = MapLabelIDtoColorName(i);
    if( labelName.empty() )
      {
      labelName.clear();
      labelName = "unknown";
      }

    // --- get label name from labelID

    suvmax = 0.0;
    suvmean = 0.0;

    // create the binary volume of the label
    vtkImageThreshold *thresholder = vtkImageThreshold::New();
    thresholder->SetInput(voiVolume);
    thresholder->SetInValue(1);
    thresholder->SetOutValue(0);
    thresholder->ReplaceOutOn();
    thresholder->ThresholdBetween(i, i);
    thresholder->SetOutputScalarType(petVolume->GetScalarType() );
    thresholder->Update();

    // use vtk's statistics class with the binary labelmap as a stencil
    vtkImageToImageStencil *stencil = vtkImageToImageStencil::New();
    stencil->SetInput(thresholder->GetOutput() );
    stencil->ThresholdBetween(1, 1);

    vtkImageAccumulate *labelstat = vtkImageAccumulate::New();
    labelstat->SetInput(petVolume);
    labelstat->SetStencil(stencil->GetOutput() );
    labelstat->Update();

    stencil->Delete();

    // --- For how many labels was SUV computed?

    int voxNumber = labelstat->GetVoxelCount();
    if( voxNumber > 0 )
      {
      NumberOfVOIs++;

      double CPETmin = (labelstat->GetMin() )[0];
      double CPETmax = (labelstat->GetMax() )[0];
      double CPETmean = (labelstat->GetMean() )[0];

      // --- we want to use the following units as noted at file top:
      // --- CPET(t) -- tissue radioactivity in pixels-- kBq/mlunits
      // --- injectced dose-- MBq and
      // --- patient weight-- kg.
      // --- computed SUV should be in units g/ml
      double weight = list.patientWeight;
      double dose = list.injectedDose;

      // --- do some error checking and reporting.
      if( list.radioactivityUnits.c_str() == NULL )
        {
        std::cerr << "ComputeSUV: Got null radioactivityUnits." << std::endl;
        return EXIT_FAILURE;
        }
      if( dose == 0.0 )
        {
        std::cerr << "ComputeSUV: Got NULL dose!" << std::endl;
        return EXIT_FAILURE;
        }
      if( weight == 0.0 )
        {
        std::cerr << "ComputeSUV: got zero weight!" << std::endl;
        return EXIT_FAILURE;
        }

      double tissueConversionFactor = ConvertRadioactivityUnits(1, list.radioactivityUnits.c_str(), "kBq");
      dose  = ConvertRadioactivityUnits( dose, list.radioactivityUnits.c_str(), "MBq");
      dose = DecayCorrection(list, dose);
      weight = ConvertWeightUnits( weight, list.weightUnits.c_str(), "kg");

      // --- check a possible multiply by slope -- take intercept into account?
      if( dose == 0.0 )
        {
        // oops, weight by dose is infinity. make a ridiculous number.
        suvmin = 99999999999999999.;
        suvmax = 99999999999999999.;
        suvmean = 99999999999999999.;
        std::cerr << "Warning: got an injected dose of 0.0. Results of SUV computation not valid." << std::endl;
        }
      else
        {
        double weightByDose = weight / dose;
        suvmax = (CPETmax * tissueConversionFactor) * weightByDose;
        suvmin = (CPETmin * tissueConversionFactor ) * weightByDose;
        suvmean = (CPETmean * tissueConversionFactor) * weightByDose;
        }
      // --- write output file
      // open file containing suvs and append to it.
      ofile.open( outputFile.c_str(), ios::out | ios::app );
      if( !ofile.is_open() )
        {
        // report error, clean up, and get out.
        std::cerr << "ERROR: cannot open nuclear medicine parameter file " << outputFile.c_str() << std::endl;
        ofile.close();
        return EXIT_FAILURE;
        }
      // --- for each value..
      // --- format looks like:
      // patientID, studyDate, dose, blood glucose, labelID, suvmax, suvmean, chemoStartDate, chemoEndDate, labelName
      // ...
      ss << list.patientName << ", " << list.studyDate << ", " << list.injectedDose  << ", "  << i << ", " << suvmax
         << ", " << suvmean << ", " << labelName.c_str() << ", " << ", " << ", " << ", " << std::endl;
      ofile << ss.str();
      ofile.close();
      std::cout << "Wrote to " << outputFile.c_str() << std::endl;
      ss.str("");
      }
  

    thresholder->Delete();
    labelstat->Delete();
    }
  return EXIT_SUCCESS;

}





} // end of anonymous namespace

// ...
// ...............................................................................................
// ...
int main( int argc, char * argv[] )
{

  PARSE_ARGS;
  parameters list;

  // ...
  // ... strings used for parsing out DICOM header info
  // ...
  std::string yearstr;
  std::string monthstr;
  std::string daystr;
  std::string hourstr;
  std::string minutestr;
  std::string secondstr;
  std::string tag;

  // ... parse command line

  // compile exec only for now
  if( //strncmp(argv[0], "slicer:", 7) != 0 &&
      argc != 9)
    {

    std::cerr << argv[0] << ":\nBad command line, argc = " << argc << ", try:\n" << argv[0]
              << " --petDICOMPath Directory --petVolume pet.nrrd --labelMap label.nrrd --csvFile FileName(.csv)"
              << std::endl;
    return EXIT_FAILURE;
    }

  // convert dicom head to radiopharm data vars
  list.patientName = "MODULE_INIT_NO_VALUE";
  list.studyDate = "MODULE_INIT_NO_VALUE";
  list.radioactivityUnits = "MODULE_INIT_NO_VALUE";
  list.volumeUnits = "MODULE_INIT_NO_VALUE";
  list.calibrationFactor = 0.0;
   list.injectedDose = 0.0;
   list.patientWeight  = 0.0;
   list.seriesReferenceTime = "MODULE_INIT_NO_VALUE";
   list.injectionTime = "MODULE_INIT_NO_VALUE";
    list.decayCorrection = "MODULE_INIT_NO_VALUE";
     list.decayFactor = "MODULE_INIT_EMPTY_ID";
     list.radionuclideHalfLife = "MODULE_INIT_NO_VALUE";
      list.frameReferenceTime = "MODULE_INIT_NO_VALUE";
       list.weightUnits = "kg";
     
  try
    {
    // ...
    // open file containing radiopharmaceutical data.
    // ...
    list.PETDICOMPath = PETDICOMPath;
    // keep the PET volume as the node selector PET volume
    list.PETVolumeName = PETVolume; // argv[2];
    list.VOIVolumeName = VOIVolume; // argv[4];
//    list.parameterFile = argv[6]; // --> .dat FILE 
    list.SUVOutputTable = OutputCSV; // argv[6]; // --> CSV file

    
    /*
    std::ifstream pfile;
    pfile.open( list.parameterFile.c_str(), ios::in );
    if( !pfile.is_open() )
      {
      // report error, clean up, and get out.
      std::cerr << "ERROR: cannot open nuclear medicine parameter file " << list.parameterFile.c_str() << std::endl;
      pfile.close();
      return EXIT_FAILURE;
      }

    // ...
    // read the metadata text file and grab all parameters.
    // ...
    size_t      colonPos;
    int         numchars;
    std::string line;
    while( !pfile.eof() )
      {
      line.clear();
      // grab a line
      getline(pfile, line);

      // process a line.
      //
      // we want to find all of the following parameters.
      //
      std::string sep = ": ";
      size_t      len = sep.size();

      // ...
      // PATIENT ID
      // ...
      if( line.find("Patient_Name: ") != std::string::npos )
        {
        // initialize
        numchars = 0;
        list.patientName = "MODULE_INIT_NO_VALUE";

        // find start of target string
        colonPos = line.find( sep );
        if( colonPos != std::string::npos )
          {
          list.patientName = line.substr( colonPos + len );
          std::cout << "patientName = " << list.patientName.c_str() << std::endl;
          }
        // catch any failure to extract parameter value
        if( list.patientName.find( "MODULE_INIT_NO_VALUE") != std::string::npos )
          {
          std::cerr << "Unable to extract patient ID." << std::endl;
          }
        }
      // ...
      // STUDY DATE
      // ...
      else if( line.find("Study_Date: ") != std::string::npos )
        {
        // initialize
        numchars = 0;
        list.studyDate = "MODULE_INIT_NO_VALUE";

        // find start of target string
        colonPos = line.find( sep);
        if( colonPos != std::string::npos )
          {
          // find end of target string
          list.studyDate = line.substr( colonPos + len );
          std::cout << "studyDate = " << list.studyDate.c_str() << std::endl;
          }
        // catch any failure to extract parameter value
        if( list.studyDate.find( "MODULE_INIT_EMPTY_ID") != std::string::npos )
          {
          std::cerr << "Unable to extract study date." << std::endl;
          }
        else
          {
          // --- YYYYMMDD
          tag = list.studyDate;
          list.studyDate.clear();
          yearstr.clear();
          daystr.clear();
          monthstr.clear();
          len = tag.length();
          if( len >= 4 )
            {
            yearstr = tag.substr(0, 4);
            }
          else
            {
            yearstr = "????";
            }
          if( len >= 6 )
            {
            monthstr = tag.substr(4, 2);
            }
          else
            {
            monthstr = "??";
            }
          if( len >= 8 )
            {
            daystr = tag.substr(6, 2);
            }
          else
            {
            daystr = "??";
            }
          tag.clear();
          tag = yearstr.c_str();
          tag += "/";
          tag += monthstr.c_str();
          tag += "/";
          tag += daystr.c_str();
          list.studyDate = tag.c_str();
          }
        }
      // ...
      // UNITS (volume and radioactivity and weight)
      // ...
      else if( line.find("Units: ") != std::string::npos )
        {
        // initialize
        numchars = 0;
        std::string units = "MODULE_INIT_NO_VALUE";

        // find start of target string
        colonPos = line.find( sep);
        if( colonPos != std::string::npos )
          {
          // find end of target string
          units.clear();
          units = line.substr( colonPos + len );
          std::cout << "units = " << units.c_str() << std::endl;
          }
        // catch any failure to extract parameter value
        if( units.find( "MODULE_INIT_EMPTY_ID") != std::string::npos )
          {
          std::cerr << "Unable to extract units." << std::endl;
          }
        // --- hopefully...
        if( units.find( "BQML") != std::string::npos )
          {
          list.radioactivityUnits = "Bq";
          list.volumeUnits = "ml";
          list.calibrationFactor = 1.0;
          }
        else
          {
          list.radioactivityUnits = "MODULE_INIT_NO_VALUE";
          list.volumeUnits = "MODULE_INIT_NO_VALUE";
          list.calibrationFactor = 0.0;
          }
        }
      // ...
      // INJECTED DOSE
      // ...
      else if( line.find("Radionuclide_Total_Dose: ") != std::string::npos )
        {
        // initialize
        numchars = 0;
        list.injectedDose = 0.0;
        std::string tmp;

        // find start of target string
        colonPos = line.find( sep);
        if( colonPos != std::string::npos )
          {
          // find end of target string
          tmp = line.substr( colonPos + len );
          list.injectedDose = atof( tmp.c_str() );
          std::cout << "injectedDose = " << list.injectedDose << std::endl;
          }
        // catch any failure to extract parameter value
        if( list.injectedDose <= 0.0 )
          {
          std::cerr << "Unable to extract injected dose." << std::endl;
          }
        }
      // ...
      // PATIENT WEIGHT
      // ...
      else if( line.find("Patients_Weight: ") != std::string::npos )
        {
        // initialize
        numchars = 0;
        list.patientWeight  = 0.0;
        std::string tmp;

        // find start of target string
        colonPos = line.find( sep);
        if( colonPos != std::string::npos )
          {
          // find end of target string
          tmp.clear();
          tmp = line.substr( colonPos + len );
          list.patientWeight = atof( tmp.c_str() );
          std::cout << "patientWeight = " << list.patientWeight << std::endl;
          }
        // catch any failure to extract parameter value
        if( list.patientWeight == 0.0 )
          {
          std::cerr << "Unable to extract patient weight." << std::endl;
          }
        }
      // ...
      // SERIES TIME
      // ...
      else if( line.find("Series_Time: ") != std::string::npos )
        {
        // initialize
        numchars = 0;
        list.seriesReferenceTime = "MODULE_INIT_NO_VALUE";

        // find start of target string
        colonPos = line.find( sep);
        if( colonPos != std::string::npos )
          {
          // find end of target string
          std::string tag = line.substr( colonPos + len );
          hourstr.clear();
          minutestr.clear();
          secondstr.clear();
          len = tag.length();
          if( len >= 2 )
            {
            hourstr = tag.substr(0, 2);
            }
          else
            {
            hourstr = "00";
            }
          if( len >= 4 )
            {
            minutestr = tag.substr(2, 2);
            }
          else
            {
            minutestr = "00";
            }
          if( len >= 6 )
            {
            secondstr = tag.substr(4);
            }
          else
            {
            secondstr = "00";
            }
          tag.clear();
          tag = hourstr.c_str();
          tag += ":";
          tag += minutestr.c_str();
          tag += ":";
          tag += secondstr.c_str();
          list.seriesReferenceTime = tag.c_str();
          std::cout << "seriesReferenceTime = " << list.seriesReferenceTime.c_str() << std::endl;
          }
        // catch any failure to extract parameter value
        if( list.seriesReferenceTime.find( "MODULE_INIT_EMPTY_ID") != std::string::npos )
          {
          std::cerr << "Unable to extract series reference time." << std::endl;
          }
        }
      // ...
      // RADIOPHARMACEUTICAL START TIME
      // ...
      else if( line.find("Radionuclide_Start_Time: ") != std::string::npos )
        {
        // initialize
        numchars = 0;
        list.injectionTime = "MODULE_INIT_NO_VALUE";
        std::string hourstr;
        std::string minutestr;
        std::string secondstr;

        // find start of target string
        colonPos = line.find( sep);
        if( colonPos != std::string::npos )
          {
          // find end of target string
          std::string tag = line.substr( colonPos + len );
          hourstr.clear();
          minutestr.clear();
          secondstr.clear();
          len = tag.length();
          if( len >= 2 )
            {
            hourstr = tag.substr(0, 2);
            }
          else
            {
            hourstr = "00";
            }
          if( len >= 4 )
            {
            minutestr = tag.substr(2, 2);
            }
          else
            {
            minutestr = "00";
            }
          if( len >= 6 )
            {
            secondstr = tag.substr(4);
            }
          else
            {
            secondstr = "00";
            }
          tag.clear();
          tag = hourstr.c_str();
          tag += ":";
          tag += minutestr.c_str();
          tag += ":";
          tag += secondstr.c_str();
          list.injectionTime = tag.c_str();
          std::cout << "radiopharmaceuticalStartTime = " << list.injectionTime.c_str() << std::endl;
          }
        // catch any failure to extract parameter value
        if( list.injectionTime.find( "MODULE_INIT_EMPTY_ID") != std::string::npos )
          {
          std::cerr << "Unable to extract radiopharmaceutical start time." << std::endl;
          }
        }
      // ...
      // DECAY CORRECTION
      // ...
      else if( line.find("Decay_Correction:") != std::string::npos )
        {
        // initialize
        numchars = 0;
        list.decayCorrection = "MODULE_INIT_NO_VALUE";

        // find start of target string
        colonPos = line.find( sep);
        if( colonPos != std::string::npos )
          {
          // find end of target string
          list.decayCorrection = line.substr( colonPos + len );
          std::cout << "decayCorrection = " << list.decayCorrection.c_str() << std::endl;
          }
        // catch any failure to extract parameter value
        if( list.decayCorrection.find( "MODULE_INIT_EMPTY_ID") != std::string::npos )
          {
          std::cerr << "Unable to extract decay correction." << std::endl;
          }
        }
      // ...
      // DECAY FACTOR
      // ...
      else if( line.find("DecayFactor:") != std::string::npos )
        {
        // initialize
        numchars = 0;
        list.decayFactor = "MODULE_INIT_EMPTY_ID";

        // find start of target string
        colonPos = line.find( sep);
        if( colonPos != std::string::npos )
          {
          // find end of target string
          list.decayFactor = line.substr( colonPos + len );
          std::cout << "decayFactor = " << list.decayFactor.c_str() << std::endl;
          }
        // catch any failure to extract parameter value
        if( list.decayFactor.find( "MODULE_INIT_EMPTY_ID" ) != std::string::npos )
          {
          std::cerr << "Unable to extract decay factor." << std::endl;
          }
        }
      // ...
      // RADIONUCLIDE HALF LIFE
      // ...
      else if( line.find("Radionuclide_Half_Life:") != std::string::npos )
        {
        // initialize
        numchars = 0;
        list.radionuclideHalfLife = "MODULE_INIT_NO_VALUE";

        // find start of target string
        colonPos = line.find( sep);
        if( colonPos != std::string::npos )
          {
          // find end of target string
          list.radionuclideHalfLife = line.substr( colonPos + len );
          std::cout << "radionuclideHalfLife = " << list.radionuclideHalfLife.c_str() << std::endl;
          }
        // catch any failure to extract parameter value
        if( list.radionuclideHalfLife.find("MODULE_INIT_NO_VALUE") != std::string::npos )
          {
          std::cerr << "Unable to extract radionuclide half life." << std::endl;
          }
        }
      // ...
      // FRAME REFERENCE TIME
      // ...
      // --- The time that the pixel values in the image
      // --- occurred. Frame Reference Time is the
      // --- offset, in msec, from the Series reference
      // --- time.
      else if( line.find("Frame_Reference_Time: ") != std::string::npos )
        {
        // initialize
        numchars = 0;
        list.frameReferenceTime = "MODULE_INIT_NO_VALUE";

        // find start of target string
        colonPos = line.find( sep);
        if( colonPos != std::string::npos )
          {
          // find end of target string
          list.frameReferenceTime = line.substr( colonPos + len );
          std::cout << "frameReferenceTime = " << list.frameReferenceTime.c_str() << std::endl;
          }
        // catch any failure to extract parameter value
        if( list.frameReferenceTime.find( "MODULE_INIT_NO_VALUE") != std::string::npos )
          {
          std::cerr << "Unable to extract frame reference time." << std::endl;
          }
        }
      }

    pfile.close();

    //
    // hardcode this for now.
    //
    list.weightUnits = "kg";
    */
    LoadImagesAndComputeSUV( list, static_cast<double>(0) );
    }

  catch( itk::ExceptionObject & excep )
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

