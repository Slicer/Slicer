
// XXX # Workaround bug in packaging of DCMTK 3.6.0 on Debian.
//     # See http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=637687
#ifdef HAVE_CONFIG_H
#undef HAVE_CONFIG_H
#endif

#include "PETStandardUptakeValueComputationCLP.h"

// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLColorTableStorageNode.h>

// vtkITK includes
#include <vtkITKArchetypeImageSeriesScalarReader.h>

// VTK includes
#include <vtkGlobFileNames.h>
#include <vtkImageAccumulate.h>
#include <vtkImageData.h>
#include <vtkImageThreshold.h>
#include <vtkImageToImageStencil.h>
#include <vtkNew.h>
#include <vtkVersion.h>

// ITK includes
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
#include <itkImageFileWriter.h>
#include <itkImageSeriesReader.h>
#include <itkImageSeriesReader.h>
#include <itkMetaDataDictionary.h>
#include <itkNumericSeriesFileNames.h>

#undef HAVE_SSTREAM // stupid DCMTK Header issue
#include "itkDCMTKFileReader.h"

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
    std::string VOIVolumeColorTableFile;
    std::string parameterFile;
    std::string SUVOutputTable;
    std::string SUVOutputStringFile;
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
  if( time == nullptr )
    {
    std::cerr << "ConvertTimeToSeconds got a nullptr time string." << std::endl;
    return -1.0;
    }

  std::string h;
  std::string m;
  std::string minAndsecStr;
  std::string secStr;

  double hours;
  double minutes;
  double seconds;

  if( time == nullptr )
    {
    return 0.0;
    }

  // ---
  // --- time will be in format HH:MM:SS.SSSS
  // --- convert to a double count of seconds.
  // ---
  std::string timeStr = time;
  h = timeStr.substr( 0, 2 );
  hours = atof( h.c_str() );

  minAndsecStr = timeStr.substr( 3 );
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

  if( fromunits == nullptr )
    {
    std::cout << "Got nullptr parameter fromunits. A bad param was probably specified." << std::endl;
    return -1.0;
    }
  if( tounits == nullptr )
    {
    std::cout << "Got nullptr parameter from tounits. A bad parameter was probably specified." << std::endl;
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

  if( fromunits == nullptr )
    {
    std::cout << "Got nullptr parameter in fromunits. A bad parameter was probably specified." << std::endl;
    return -1.0;
    }
  if( tounits == nullptr )
    {
    std::cout << "Got nullptr parameter in tounits. A bad parameter was probably specified." << std::endl;
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
std::string MapLabelIDtoColorName( int id, std::string colorFile )
{
  // use the colour table that was passed in with the VOI volume

  std::string colorName;

  vtkNew<vtkMRMLColorTableNode> colorNode;
  vtkNew<vtkMRMLColorTableStorageNode> colorStorageNode;
  colorStorageNode->SetFileName(colorFile.c_str() );

  if( !colorStorageNode->ReadData(colorNode.GetPointer()) )
    {
    std::cerr << "Error reading colour file " << colorStorageNode->GetFileName() << endl;
    }
  else
    {
    colorName = colorNode->GetColorName(id);
    }
  return colorName;
}

// ...
// ...............................................................................................
// ...
template <class T>
int LoadImagesAndComputeSUV( parameters & list, T )
{
  //
  // for writing csv output files
  //
  std::string   outputFile = list.SUVOutputTable;
  std::ofstream ofile;
  std::string  outputStringFile = list.SUVOutputStringFile;
  std::ofstream stringFile;
  vtkImageData *                    petVolume;
  vtkImageData *                    voiVolume;
  vtkITKArchetypeImageSeriesReader *reader1 = nullptr;
  vtkITKArchetypeImageSeriesReader *reader2 = nullptr;
  vtkAlgorithmOutput* petVolumeConnection = nullptr;
  vtkAlgorithmOutput* voiVolumeConnection = nullptr;

  // check for the input files
  FILE * petfile;
  petfile = fopen(list.PETVolumeName.c_str(), "r");
  if( petfile == nullptr )
    {
    std::cerr << "ERROR: cannot open input volume file '" << list.PETVolumeName.c_str() << "'" << endl;
    return EXIT_FAILURE;
    }
  fclose(petfile);

  FILE * voifile;
  voifile = fopen(list.VOIVolumeName.c_str(), "r");
  if( voifile == nullptr )
    {
    std::cerr << "ERROR: cannot open ROI Volume  file '" << list.VOIVolumeName.c_str() << "'" << endl;
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
  petVolume = reader1->GetOutput();
  petVolumeConnection = reader1->GetOutputPort();
  voiVolume = reader2->GetOutput();
  voiVolumeConnection = reader2->GetOutputPort();


  //
  // COMPUTE SUV ///////////////////////////////////////////////////////////////////////////////RSNA CHANGE//////////////////////////
  //

  if( petVolume == nullptr )
    {
    std::cerr << "No input PET volume found." << std::endl;
    return EXIT_FAILURE;
    }

  // find input labelmap volume
  if( voiVolume == nullptr )
    {
    std::cerr <<  "No input VOI volume found" << std::endl;
    return EXIT_FAILURE;
    }

  // read the DICOM dir to get the radiological data

  typedef short PixelValueType;
  typedef itk::Image< PixelValueType, 3 > VolumeType;
  typedef itk::ImageSeriesReader< VolumeType > VolumeReaderType;
  typedef itk::GDCMSeriesFileNames InputNamesGeneratorType;

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
    notDICOM = 1;
    }
  gfn->Delete();
  if ( notDICOM )
    {
    std::cerr << "PET Dicom parameter doesn't point to a dicom directory!" << std::endl;
    return EXIT_FAILURE;
    }


  InputNamesGeneratorType::Pointer inputNames = InputNamesGeneratorType::New();
  inputNames->SetUseSeriesDetails(true);
  inputNames->SetDirectory(list.PETDICOMPath);
  itk::SerieUIDContainer seriesUIDs = inputNames->GetSeriesUIDs();

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
    itk::DCMTKFileReader fileReader;
    fileReader.SetFileName(filenames[0]);
    fileReader.LoadFile();

    itk::DCMTKSequence seq;
    if(fileReader.GetElementSQ(0x0054,0x0016,seq,false) == EXIT_SUCCESS)
      {
      parsingDICOM = 1;
//      for(int i = 0; i < seq.card(); ++i)
//        {
//        itk::DCMTKSequence sqItem;
//        seq.GetSequence(i,sqItem);
        //---
        //--- Radiopharmaceutical Start Time
//        sqItem.GetElementTM(0x0018,0x1072,tag);
        seq.GetElementTM(0x0018,0x1072,tag);
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
        if ( tag.c_str() == nullptr || *(tag.c_str()) == '\0' )
          {
          list.injectionTime  = "MODULE_INIT_NO_VALUE" ;
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
          list.injectionTime = tag.c_str();
          }

        //---
        //--- Radionuclide Total Dose
//        if(sqItem.GetElementDS(0x0018,0x1074,1,&list.injectedDose,false) != EXIT_SUCCESS)
        if(seq.GetElementDS(0x0018,0x1074,1,&list.injectedDose,false) != EXIT_SUCCESS)
          {
          list.injectedDose = 0.0;
          }

        //---
        //--- RadionuclideHalfLife
        //--- Expect a Decimal String
        //--- A string of characters representing either
        //--- a fixed point number or a floating point number.
        //--- A fixed point number shall contain only the characters 0-9
        //--- with an optional leading "+" or "-" and an optional "." to mark
        //--- the decimal point. A floating point number shall be conveyed
        //--- as defined in ANSI X3.9, with an "E" or "e" to indicate the start
        //--- of the exponent. Decimal Strings may be padded with leading
        //--- or trailing spaces. Embedded spaces are not allowed.
//        if(sqItem.GetElementCS(0x0018,0x1075,list.radionuclideHalfLife,false) != EXIT_SUCCESS)
        if(seq.GetElementDS(0x0018,0x1075,list.radionuclideHalfLife,false) != EXIT_SUCCESS)
          {
          list.radionuclideHalfLife = "MODULE_INIT_NO_VALUE";
          }
        //---
        //---Radionuclide Positron Fraction
        //--- not currently using this one?
        std::string radioNuclidePositronFraction;
//        if(sqItem.GetElementCS(0x0018,0x1075,radioNuclidePositronFraction,false) != EXIT_SUCCESS)
        if(seq.GetElementDS(0x0018,0x1075,radioNuclidePositronFraction,false) != EXIT_SUCCESS)
          {
          radioNuclidePositronFraction = "MODULE_INIT_NO_VALUE";
          }
//        }

      //--
      //--- UNITS: something like BQML:
      //--- CNTS, NONE, CM2, PCNT, CPS, BQML,
      //--- MGMINML, UMOLMINML, MLMING, MLG,
      //--- 1CM, UMOLML, PROPCNTS, PROPCPS,
      //--- MLMINML, MLML, GML, STDDEV
      //---

      if(fileReader.GetElementCS(0x0054,0x1001,tag,false) == EXIT_SUCCESS)
        {
        //--- I think these are piled together. MBq ml... search for all.
        std::string units = tag.c_str();
        if (units.find ("BQML") != std::string::npos)
          {
          list.radioactivityUnits= "Bq";
          list.tissueRadioactivityUnits = "Bq";
          }
        else if ( ( units.find ("MBq") != std::string::npos) ||
                  ( units.find ("MBQ") != std::string::npos) )
          {
          list.radioactivityUnits = "MBq";
          list.tissueRadioactivityUnits = "MBq";
          }
        else if ( (units.find ("kBq") != std::string::npos) ||
                  (units.find ("kBQ") != std::string::npos) ||
                  (units.find ("KBQ") != std::string::npos) )
          {
          list.radioactivityUnits = "kBq";
          list.tissueRadioactivityUnits = "kBq";
          }
        else if ( (units.find ("mBq") != std::string::npos) ||
                  (units.find ("mBQ") != std::string::npos) )
          {
          list.radioactivityUnits = "mBq";
          list.tissueRadioactivityUnits = "mBq";
          }
        else if ( (units.find ("uBq") != std::string::npos) ||
                  (units.find ("uBQ") != std::string::npos) )
          {
          list.radioactivityUnits = "uBq";
          list.tissueRadioactivityUnits = "uBq";
          }
        else if ( (units.find ("Bq") != std::string::npos) ||
                  (units.find ("BQ") != std::string::npos) )
          {
          list.radioactivityUnits = "Bq";
          list.tissueRadioactivityUnits = "Bq";
          }
        else if ( (units.find ("MCi") != std::string::npos) ||
                  ( units.find ("MCI") != std::string::npos) )
          {
          list.radioactivityUnits = "MCi";
          list.tissueRadioactivityUnits = "MCi";
          }
        else if ( (units.find ("kCi") != std::string::npos) ||
                  (units.find ("kCI") != std::string::npos)  ||
                  (units.find ("KCI") != std::string::npos) )
          {
          list.radioactivityUnits = "kCi";
          list.tissueRadioactivityUnits = "kCi";
          }
        else if ( (units.find ("mCi") != std::string::npos) ||
                  (units.find ("mCI") != std::string::npos) )
          {
          list.radioactivityUnits = "mCi";
          list.tissueRadioactivityUnits = "mCi";
          }
        else if ( (units.find ("uCi") != std::string::npos) ||
                  (units.find ("uCI") != std::string::npos) )
          {
          list.radioactivityUnits = "uCi";
          list.tissueRadioactivityUnits = "uCi";
          }
        else if ( (units.find ("Ci") != std::string::npos) ||
                  (units.find ("CI") != std::string::npos) )
          {
          list.radioactivityUnits = "Ci";
          list.tissueRadioactivityUnits = "Ci";
          }
        list.volumeUnits = "ml";
        }
      else
        {
        //--- default values.
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
      if(fileReader.GetElementCS(0x0054,0x1102,tag,false) == EXIT_SUCCESS)
        {
        //---A string of characters with leading or trailing spaces (20H) being non-significant.
        list.decayCorrection = tag.c_str();
        }
      else
        {
        list.decayCorrection = "MODULE_INIT_NO_VALUE";
        }

      //---
      //--- StudyDate
      if(fileReader.GetElementDA(0x0008,0x0021,tag,false) == EXIT_SUCCESS)
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
        list.studyDate = "MODULE_INIT_NO_VALUE";
        }

      //---
      //--- PatientName
      if(fileReader.GetElementPN(0x0010,0x0010,tag,false) == EXIT_SUCCESS)
        {
        list.patientName = tag.c_str();
        }
      else
        {
        list.patientName = "MODULE_INIT_NO_VALUE";
        }

      //---
      //--- DecayFactor
      if(fileReader.GetElementDS(0x0054,0x1321,tag,false) == EXIT_SUCCESS)
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
        list.decayFactor =  "MODULE_INIT_NO_VALUE" ;
        }


      //---
      //--- FrameReferenceTime
      if(fileReader.GetElementDS(0x0054,0x1300,tag,false) == EXIT_SUCCESS)
        {
        //--- The time that the pixel values in the image
        //--- occurred. Frame Reference Time is the
        //--- offset, in msec, from the Series reference
        //--- time.
        list.frameReferenceTime = tag.c_str();
        }
      else
        {
        list.frameReferenceTime = "MODULE_INIT_NO_VALUE";
        }


      //---
      //--- SeriesTime
      if(fileReader.GetElementTM(0x0008,0x0031,tag,false) == EXIT_SUCCESS)
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
        list.seriesReferenceTime = tag.c_str();
        }
      else
        {
        list.seriesReferenceTime = "MODULE_INIT_NO_VALUE";
        }


      //---
      //--- PatientWeight
      if(fileReader.GetElementDS(0x0010,0x1030,1,&list.patientWeight,false) == EXIT_SUCCESS)
        {
        //--- Expect same format as RadionuclideHalfLife
        list.weightUnits = "kg";
        }
      else
        {
        list.patientWeight = 0.0;
        list.weightUnits = "";

        }


      //---
      //--- CalibrationFactor
      if(fileReader.GetElementDS(0x7053,0x1009,1,
                                 &list.calibrationFactor,false) != EXIT_SUCCESS)
        {
        list.calibrationFactor =  0.0 ;
        }
      }


    // check.... did we get all params we need for computation?
    if ( (parsingDICOM) &&
         (list.injectedDose != 0.0) &&
         (list.patientWeight != 0.0) &&
         (list.seriesReferenceTime.compare("MODULE_INIT_NO_VALUE") != 0) &&
         (list.injectionTime.compare("MODULE_INIT_NO_VALUE") != 0) &&
         (list.radionuclideHalfLife.compare("MODULE_INIT_NO_VALUE") != 0) )
      {
      std::cout << "Input parameters okay..." << std::endl;
      }
    else
      {
      std::cerr << "Missing some parameters..." << std::endl;
      std::cerr << "\tinjected Dose " << list.injectedDose << " should not be 0" << std::endl;
      std::cerr << "\tpatientWeight " << list.patientWeight << " should not be 0" << std::endl;
      std::cerr << "\tseriesReferenceTime " << list.seriesReferenceTime << " should not be MODULE_INIT_NO_VALUE" << std::endl;
      std::cerr << "\tinjectionTime " << list.injectionTime<< " should not be MODULE_INIT_NO_VALUE" << std::endl;
      std::cerr << "\tradionuclideHalfLife " << list.radionuclideHalfLife << " should not be MODULE_INIT_NO_VALUE" << std::endl;
      return EXIT_FAILURE;
      }




  // convert from input units.
  if( list.radioactivityUnits.c_str() == nullptr )
    {
    std::cerr << "ComputeSUV: Got nullptr radioactivity units. No computation done." << std::endl;
    return EXIT_FAILURE;
    }
  if( list.weightUnits.c_str() == nullptr )
    {
    std::cerr << "ComputeSUV: Got nullptr weight units. No computation could be done." << std::endl;
    return EXIT_FAILURE;
    }

  double suvmax, suvmin, suvmean;

  // make up a string with output to return
  std::string outputLabelString = "OutputLabel = ";
  std::string outputLabelValueString = "OutputLabelValue = ";
  std::string outputSUVMaxString = "SUVMax = ";
  std::string outputSUVMeanString = "SUVMean = ";
  std::string outputSUVMinString = "SUVMin = ";

  // --- find the max and min label in mask
  vtkImageAccumulate *stataccum = vtkImageAccumulate::New();
  stataccum->SetInputConnection( voiVolumeConnection );
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
    labelName = MapLabelIDtoColorName(i, list.VOIVolumeColorTableFile);
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
    thresholder->SetInputConnection(voiVolumeConnection);
    thresholder->SetInValue(1);
    thresholder->SetOutValue(0);
    thresholder->ReplaceOutOn();
    thresholder->ThresholdBetween(i, i);
    thresholder->SetOutputScalarType(petVolume->GetScalarType() );
    thresholder->Update();

    // use vtk's statistics class with the binary labelmap as a stencil
    vtkImageToImageStencil *stencil = vtkImageToImageStencil::New();
    stencil->SetInputConnection(thresholder->GetOutputPort() );
    stencil->ThresholdBetween(1, 1);

    vtkImageAccumulate *labelstat = vtkImageAccumulate::New();
    labelstat->SetInputConnection(petVolumeConnection);
    labelstat->SetInputConnection(1, stencil->GetOutputPort() ); // == SetStencilData()
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
      if( list.radioactivityUnits.c_str() == nullptr )
        {
        std::cerr << "ComputeSUV: Got null radioactivityUnits." << std::endl;
        return EXIT_FAILURE;
        }
      if( dose == 0.0 )
        {
        std::cerr << "ComputeSUV: Got nullptr dose!" << std::endl;
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
      // --- append to output return string file
      std::stringstream outputStringStream;
//      outputStringStream << list.patientName << ", " << list.studyDate << ", " << list.injectedDose  << ", "  << i << ", " << suvmax << ", " << suvmean << ", " << labelName.c_str() << ", " << ", " << ", " << ", " << std::endl;
      std::string postfixStr = ", ";
      if (i == hi)
        {
        postfixStr = "";
        }
      outputStringStream.str("");
      outputStringStream << labelName.c_str() << postfixStr;
      outputLabelString += outputStringStream.str();
      outputStringStream.str("");
      outputStringStream  << i << postfixStr;
      outputLabelValueString += outputStringStream.str();
      outputStringStream.str("");
      outputStringStream  << suvmax << postfixStr;
      outputSUVMaxString += outputStringStream.str();
      outputStringStream.str("");
      outputStringStream  << suvmean << postfixStr;
      outputSUVMeanString += outputStringStream.str();
      outputStringStream.str("");
      outputStringStream << suvmin << postfixStr;
      outputSUVMinString += outputStringStream.str();

      // --- write output CSV file

      // open file containing suvs and append to it.
      if (outputFile.compare("") != 0)
        {
        ofile.open( outputFile.c_str(), ios::out | ios::app );
        if( !ofile.is_open() )
          {
          // report error, clean up, and get out.
          std::cerr << "ERROR: cannot open nuclear medicine output csv parameter file '" << outputFile.c_str() << "', see return strings for values" << std::endl;
          }
        else
          {
          ss.str("");
          ofile.seekp(0,ios::end);
          long pos = ofile.tellp();
          if (pos == 0)
            {
            ss << "patientID,studyDate,dose,labelID,suvmin,suvmax,suvmean,labelName" << std::endl;
            }
          // --- for each value..
          // --- format looks like:
          // patientID, studyDate, dose, labelID, suvmin, suvmax, suvmean, labelName
          // ...

          ss << list.patientName << ", " << list.studyDate << ", " << list.injectedDose  << ", "  << i << ", " << suvmin << ", " << suvmax
             << ", " << suvmean << ", " << labelName.c_str() << std::endl;
          ofile << ss.str();
          ofile.close();
          std::cout << "Wrote output for label " << labelName.c_str() << " to " << outputFile.c_str() << std::endl;
          }
        }
      }


    thresholder->Delete();
    labelstat->Delete();
    }
  // --- write output return string file
  if (outputStringFile.compare("") != 0)
    {
    std::stringstream ss;
    ss << outputLabelString << std::endl;
    ss << outputLabelValueString << std::endl;
    ss << outputSUVMaxString << std::endl;
    ss << outputSUVMeanString << std::endl;
    ss << outputSUVMinString << std::endl;
    std::string stringOutput = ss.str();
    stringFile.open(outputStringFile.c_str());
    if (!stringFile.is_open() )
      {
      // report error, clean up
      std::cerr << "ERROR: cannot open nuclear medicine output string parameter file '" << outputStringFile.c_str() << "', output string was:\n" << stringOutput.c_str() << std::endl;
      return EXIT_FAILURE;
      }
    stringFile << stringOutput;
    stringFile.close();
    std::cout << "Wrote return string to " << outputStringFile.c_str() << ": " << std::endl << stringOutput.c_str() << std::endl;
    }
  else
    {
    // if nothing was written because no output file was specified, report an
    // error
     if (outputFile.compare("") == 0)
       {
       std::cerr << "Neither the nuclear medicine output csv parameter file nor the string parameter file were specified, please specify one or the other to obtain the calculated output." << std::endl;
       return EXIT_FAILURE;
       }
    }
  reader1->Delete();
  reader2->Delete();

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
    // pass the input parameters to the helper method
    list.PETDICOMPath = PETDICOMPath;
    // keep the PET volume as the node selector PET volume
    list.PETVolumeName = PETVolume;
    list.VOIVolumeName = VOIVolume;
    list.VOIVolumeColorTableFile = ColorTable;
    list.SUVOutputTable = OutputCSV;
    // GenerateCLP makes a temporary file with the path saved to
    // returnParameterFile, write the output strings in there as key = value pairs
    list.SUVOutputStringFile = returnParameterFile;
    std::cout << "list.SUVOutputStringFile = " << list.SUVOutputStringFile << std::endl;
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

