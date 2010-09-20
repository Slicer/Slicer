
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkPluginUtilities.h"
#include "vtkPluginFilterWatcher.h"
#include "ComputeSUVBodyWeightCLP.h"

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <vector>
#include <stdio.h>

#include <vtksys/SystemTools.hxx>
#include <vtksys/Directory.hxx>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkIntArray.h"
#include "vtkITKArchetypeImageSeriesReader.h"
#include "vtkITKArchetypeImageSeriesScalarReader.h"
#include "vtkImageData.h"
#include "vtkImageAccumulate.h"
#include "vtkImageThreshold.h"
#include "vtkImageToImageStencil.h"
#include "vtkImageMathematics.h"
#include "vtkLookupTable.h"
#include "vtkColorTransferFunction.h"

#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLColorNode.h"

//...
//...............................................................................................
//...
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
//...
//...............................................................................................
//...



// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace {
  
struct parameters
{
  std::string PETVolumeName;
  std::string VOIVolumeName;
  std::string parameterFile;
  std::string SUVOutputTable;
  std::string patientName;
  std::string studyDate;
  std::string radioactivityUnits;
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
} ;
  
//...
//...............................................................................................
//...

double ConvertTimeToSeconds (const char *time )
{
  if ( time == NULL )
    {
    std::cerr << "ConvertTimeToSeconds got a NULL time string." << std::endl;
    return (-1.0);
    }

  std::string h;
  std::string m;
  std::string minAndsecStr;
  std::string secStr;  

  double hours;
  double minutes;
  double seconds;
  
  if ( time == NULL )
    {
    return (0.0);
    }

  //---
  //--- time will be in format HH:MM:SS.SSSS
  //--- convert to a double count of seconds.
  //---
  std::string timeStr = time;
  size_t i = timeStr.find_first_of(":");
  h = timeStr.substr ( 0, 2 );
  hours = atof ( h.c_str() );

  minAndsecStr = timeStr.substr ( 3 );
  i = minAndsecStr.find_first_of ( ":" );
  m = minAndsecStr.substr (0, 2 );
  minutes = atof ( m.c_str() );

  secStr = minAndsecStr.substr ( 3 );
  seconds = atof ( secStr.c_str() );
  
  double retval = ( seconds +
                    (60.0 * minutes) +
                    (3600.0 * hours ));
  return (retval);  
}



//...
//...............................................................................................
//...
double ConvertWeightUnits (double count, const char *fromunits, const char *tounits )
{

  double conversion = count;

  if ( fromunits == NULL )
    {
    std::cout << "Got NULL parameter fromunits. A bad param was probably specified." << std::endl;
    return (-1.0);
    }
  if ( tounits == NULL )
    {
    std::cout << "Got NULL parameter from tounits. A bad parameter was probably specified." << std::endl;
    return (-1.0);
    }

  /*
    possibilities include:
  ---------------------------
  "kilograms [kg]"
  "grams [g]"
  "pounds [lb]"
  */

  //--- kg to...
  if ( !strcmp (fromunits, "kg"))
    {
    if ( !strcmp (tounits, "kg"))
      {
      return (conversion);
      }
    else if ( !strcmp (tounits, "g"))
      {
      conversion *= 1000.0;
      }
    else if ( !strcmp (tounits, "lb"))
      {
      conversion *= 2.2;
      }    
    }
  else if ( !strcmp (fromunits, "g"))
    {
    if ( !strcmp (tounits, "kg"))
      {
      conversion /= 1000.0;
      }
    else if ( !strcmp (tounits, "g"))
      {
      return ( conversion);
      }
    else if ( !strcmp (tounits, "lb"))
      {
      conversion *= .0022;
      }
    }
  else if ( !strcmp (fromunits, "lb"))
    {
    if ( !strcmp (tounits, "kg"))
      {
      conversion *= 0.45454545454545453;
      }
    else if ( !strcmp (tounits, "g"))
      {
      conversion *= 454.54545454545453;
      }
    else if ( !strcmp (tounits, "lb"))
      {
      return ( conversion );
      }
    }
  return (conversion);

}




//...
//...............................................................................................
//...  
double ConvertRadioactivityUnits (double count, const char *fromunits, const char *tounits )
{


  double conversion = count;
  if ( fromunits == NULL )
    {
    std::cout << "Got NULL parameter in fromunits. A bad parameter was probably specified." << std::endl;
    return (-1.0);
    }
  if ( tounits == NULL )
    {
    std::cout << "Got NULL parameter in tounits. A bad parameter was probably specified." << std::endl;
    return (-1.0);
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

  //--- MBq to...
  if (!strcmp(fromunits, "MBq" ))
    {
    if (!(strcmp (tounits, "MBq" )))
      {
      return ( conversion);
      }
    else if (!(strcmp (tounits, "kBq" )))
      {
      conversion *= 1000.0;
      }
    else if (!(strcmp (tounits, "Bq" )))
      {
      conversion *=1000000.0;
      }
    else if (!(strcmp (tounits, "mBq" )))
      {
      conversion *= 1000000000.0;
      }
    else if (!(strcmp (tounits, " uBq" )))
      {
      conversion *= 1000000000000.0;
      }
    else if (!(strcmp (tounits, "MCi" )))
      {
      conversion *= 0.000000000027027027027027;
      }
    else if (!(strcmp (tounits, "kCi" )))
      {
      conversion *= 0.000000027027027027027;
      }
    else if (!(strcmp (tounits, "Ci" )))
      {
      conversion *= 0.000027027027027027;
      }
    else if (!(strcmp (tounits, "mCi" )))
      {
      conversion *= 0.027027027027027;
      }
    else if (!(strcmp (tounits, "uCi" )))
      {
      conversion *= 27.027027027;
      }
    }
  //--- kBq to...
  else if ( !strcmp(fromunits, "kBq" ))
    {
    if (!(strcmp (tounits, "MBq" )))
      {
      conversion *= .001;
      }
    else if (!(strcmp (tounits, "kBq" )))
      {
      return ( conversion);      
      }
    else if (!(strcmp (tounits, "Bq" )))
      {
      conversion *= 1000.0;
      }
    else if (!(strcmp (tounits, "mBq" )))
      {
      conversion *= 1000000.0;
      }
    else if (!(strcmp (tounits, " uBq" )))
      {
      conversion *= 1000000000.0;
      }
    else if (!(strcmp (tounits, "MCi" )))
      {
      conversion *= 0.000000000000027027027027027;
      }
    else if (!(strcmp (tounits, "kCi" )))
      {
      conversion *= 0.000000000027027027027027;
      }
    else if (!(strcmp (tounits, "Ci" )))
      {
      conversion *= 0.000000027027027027027;
      }
    else if (!(strcmp (tounits, "mCi" )))
      {
      conversion *=0.000027027027027027;
      }
    else if (!(strcmp (tounits, "uCi" )))
      {
      conversion *= 0.027027027027027;
      }
    }
  //--- Bq to...
  else if ( !strcmp(fromunits, "Bq" ))
    {
    if (!(strcmp (tounits, "MBq" )))
      {
      conversion *= 0.000001;
      }
    else if (!(strcmp (tounits, "kBq" )))
      {
      conversion *= 0.001;
      }
    else if (!(strcmp (tounits, "Bq" )))
      {
      return ( conversion);      
      }
    else if (!(strcmp (tounits, "mBq" )))
      {
      conversion *= 1000.0;
      }
    else if (!(strcmp (tounits, " uBq" )))
      {
      conversion *= 1000000.0;
      }
    else if (!(strcmp (tounits, "MCi" )))
      {
      conversion *= 0.000000000000000027027027027027;
      }
    else if (!(strcmp (tounits, "kCi" )))
      {
      conversion *=  0.000000000000027027027027027;
      }
    else if (!(strcmp (tounits, "Ci" )))
      {
      conversion *= 0.000000000027027027027027;
      }
    else if (!(strcmp (tounits, "mCi" )))
      {
      conversion *= 0.000000027027027027027;
      }
    else if (!(strcmp (tounits, "uCi" )))
      {
      conversion *= 0.000027027027027027;
      }
    }
  //--- mBq to...
  else if ( !strcmp(fromunits, "mBq" ))
    {
    if (!(strcmp (tounits, "MBq" )))
      {
      conversion *= 0.000000001;
      }
    else if (!(strcmp (tounits, "kBq" )))
      {
      conversion *= 0.000001;
      }
    else if (!(strcmp (tounits, "Bq" )))
      {
      conversion *= 0.001;
      }
    else if (!(strcmp (tounits, "mBq" )))
      {
      return ( conversion);      
      }
    else if (!(strcmp (tounits, " uBq" )))
      {
      conversion *= 1000.0;
      }
    else if (!(strcmp (tounits, "MCi" )))
      {
      conversion *= 0.00000000000000000002702702702702;
      }
    else if (!(strcmp (tounits, "kCi" )))
      {
      conversion *= 0.000000000000000027027027027027;
      }
    else if (!(strcmp (tounits, "Ci" )))
      {
      conversion *= 0.000000000000027027027027027;
      }
    else if (!(strcmp (tounits, "mCi" )))
      {
      conversion *= 0.000000000027027027027027;
      }
    else if (!(strcmp (tounits, "uCi" )))
      {
      conversion *= 0.000000027027027027027;
      }
    }
  //--- uBq to...
  else if ( !strcmp(fromunits, "uBq" ))
    {
    if (!(strcmp (tounits, "MBq" )))
      {
      conversion *= 0.000000000001;
      }
    else if (!(strcmp (tounits, "kBq" )))
      {
      conversion *= 0.000000001;
      }
    else if (!(strcmp (tounits, "Bq" )))
      {
      conversion *= 0.000001;
      }
    else if (!(strcmp (tounits, "mBq" )))
      {
      conversion *= 0.001;
      }
    else if (!(strcmp (tounits, " uBq" )))
      { 
      return ( conversion);     
      }
    else if (!(strcmp (tounits, "MCi" )))
      {
      conversion *= 0.000000000000000000000027027027027027;
      }
    else if (!(strcmp (tounits, "kCi" )))
      {
      conversion *= 0.000000000000000000027027027027027;
      }
    else if (!(strcmp (tounits, "Ci" )))
      {
      conversion *= 0.000000000000000027027027027027;
      }
    else if (!(strcmp (tounits, "mCi" )))
      {
      conversion *= 0.000000000000027027027027027;
      }
    else if (!(strcmp (tounits, "uCi" )))
      {
      conversion *= 0.000000000027027027027027;
      }
    }
  //--- MCi to...
  else if ( !strcmp(fromunits, "MCi" ))
    {
    if (!(strcmp (tounits, "MBq" )))
      {
      conversion *= 37000000000.0;      
      }
    else if (!(strcmp (tounits, "kBq" )))
      {
      conversion *= 37000000000000.0;
      }
    else if (!(strcmp (tounits, "Bq" )))
      {
      conversion *= 37000000000000000.0;
      }
    else if (!(strcmp (tounits, "mBq" )))
      {
      conversion *= 37000000000000000000.0;
      }
    else if (!(strcmp (tounits, " uBq" )))
      {
      conversion *=  37000000000000000000848.0;    
      }
    else if (!(strcmp (tounits, "MCi" )))
      {
      return ( conversion);
      }
    else if (!(strcmp (tounits, "kCi" )))
      {
      conversion *= 1000.0;
      }
    else if (!(strcmp (tounits, "Ci" )))
      {
      conversion *= 1000000.0;
      }
    else if (!(strcmp (tounits, "mCi" )))
      {
      conversion *= 1000000000.0;      
      }
    else if (!(strcmp (tounits, "uCi" )))
      {
      conversion *= 1000000000000.0;      
      }
    }
  //--- kCi to...
  else if ( !strcmp(fromunits, "kCi" ))
    {
    if (!(strcmp (tounits, "MBq" )))
      {
      conversion *= 37000000.0;      
      }
    else if (!(strcmp (tounits, "kBq" )))
      {
      conversion *= 37000000000.0;      
      }
    else if (!(strcmp (tounits, "Bq" )))
      {
      conversion *= 37000000000000.0;      
      }
    else if (!(strcmp (tounits, "mBq" )))
      {
      conversion *= 37000000000000000.0;      
      }
    else if (!(strcmp (tounits, " uBq" )))
      {
      conversion *= 37000000000000000000.0;      
      }
    else if (!(strcmp (tounits, "MCi" )))
      {
      conversion *= 0.001;
      }
    else if (!(strcmp (tounits, "kCi" )))
      {
      return ( conversion);      
      }
    else if (!(strcmp (tounits, "Ci" )))
      {
      conversion *= 1000.0;      
      }
    else if (!(strcmp (tounits, "mCi" )))
      {
      conversion *= 1000000.0;      
      }
    else if (!(strcmp (tounits, "uCi" )))
      {
      conversion *= 1000000000.0;      
      }
    }
  //--- Ci to...
  else if ( !strcmp(fromunits, "Ci" ))
    {
    if (!(strcmp (tounits, "MBq" )))
      {
      conversion *= 37000.0;      
      }
    else if (!(strcmp (tounits, "kBq" )))
      {
      conversion *= 37000000.0;      
      }
    else if (!(strcmp (tounits, "Bq" )))
      {
      conversion *= 37000000000.0;      
      }
    else if (!(strcmp (tounits, "mBq" )))
      {
      conversion *= 37000000000000.0;      
      }
    else if (!(strcmp (tounits, " uBq" )))
      {
      conversion *= 37000000000000000.0;      
      }
    else if (!(strcmp (tounits, "MCi" )))
      {
      conversion *= 0.0000010;
      }
    else if (!(strcmp (tounits, "kCi" )))
      {
      conversion *= 0.001;      
      }
    else if (!(strcmp (tounits, "Ci" )))
      {
      return ( conversion);      
      }
    else if (!(strcmp (tounits, "mCi" )))
      {
      conversion *= 1000.0;
      }
    else if (!(strcmp (tounits, "uCi" )))
      {
      conversion *= 1000000.0;
      }
    }
  //--- mCi to...
  else if ( !strcmp(fromunits, "mCi" ))
    {
    if (!(strcmp (tounits, "MBq" )))
      {
      conversion *= 37.0;      
      }
    else if (!(strcmp (tounits, "kBq" )))
      {
      conversion *= 37000.0;      
      }
    else if (!(strcmp (tounits, "Bq" )))
      {
      conversion *= 37000000.0;      
      }
    else if (!(strcmp (tounits, "mBq" )))
      {
      conversion *= 37000000000.0;      
      }
    else if (!(strcmp (tounits, " uBq" )))
      {
      conversion *= 37000000000000.0;      
      }
    else if (!(strcmp (tounits, "MCi" )))
      {
      conversion *= 0.0000000010;
      }
    else if (!(strcmp (tounits, "kCi" )))
      {
      conversion *= 0.0000010;      
      }
    else if (!(strcmp (tounits, "Ci" )))
      {
      conversion *= 0.001;      
      }
    else if (!(strcmp (tounits, "mCi" )))
      {
      return ( conversion);      
      }
    else if (!(strcmp (tounits, "uCi" )))
      {
      conversion *= 1000.0;      
      }
    }
  //--- uCi to...
  else if ( !strcmp(fromunits, " uCi" ))
    {
    if (!(strcmp (tounits, "MBq" )))
      {
      conversion *= 0.037;
      }
    else if (!(strcmp (tounits, "kBq" )))
      {
      conversion *= 37.0;      
      }
    else if (!(strcmp (tounits, "Bq" )))
      {
      conversion *= 37000.0;      
      }
    else if (!(strcmp (tounits, "mBq" )))
      {
      conversion *= 37000000.0;      
      }
    else if (!(strcmp (tounits, " uBq" )))
      {
      conversion *= 37000000000.0;      
      }
    else if (!(strcmp (tounits, "MCi" )))
      {
      conversion *= 0.0000000000010;
      }
    else if (!(strcmp (tounits, "kCi" )))
      {
      conversion *= 0.0000000010;      
      }
    else if (!(strcmp (tounits, "Ci" )))
      {
      conversion *= 0.0000010;      
      }
    else if (!(strcmp (tounits, "mCi" )))
      {
      conversion *= 0.001;      
      }
    else if (!(strcmp (tounits, "uCi" )))
      {
      return ( conversion);      
      }
    }
  
  return (conversion);
}
  



//...
//...............................................................................................
//...  
double DecayCorrection (parameters &list, double inVal )
{

  double scanTimeSeconds = ConvertTimeToSeconds (list.seriesReferenceTime.c_str() );
  double startTimeSeconds = ConvertTimeToSeconds ( list.injectionTime.c_str() );
  double halfLife = atof ( list.radionuclideHalfLife.c_str() );
  double decayTime = scanTimeSeconds-startTimeSeconds;
  double correctedVal = inVal * (double)pow(2.0, -(decayTime/halfLife) );
  return ( correctedVal );
}

//...
//...............................................................................................
//...
const char *MapLabelIDtoColorName ( int id )
{

  // This method is used with a hardcoded colormap for now
  // for Horky PETCT project.
  // TODO: finalize atlas to be used and take that from command line.
  const char *name;
  switch ( id )
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
  return (name );
}



//...
//...............................................................................................
//...
  template<class T>
  int LoadImagesAndComputeSUV( parameters &list, T )
{

  typedef    T       InputPixelType;
  typedef itk::Image< InputPixelType,  3 >   InputImageType;

  typedef itk::Image< unsigned char, 3 > LabelImageType;
  
  typedef    T       OutputPixelType;
  typedef itk::Image< OutputPixelType, 3 >   OutputImageType;

  typedef itk::ImageFileReader< InputImageType > ReaderType;
  typedef itk::ImageFileReader< LabelImageType > LabelReaderType;
  typedef itk::ImageFileWriter< OutputImageType > WriterType;
  
  //
  // for writing csv output files
  //
  std::string outputFile = list.SUVOutputTable;
  std::ofstream ofile;

  vtkImageData *petVolume;
  vtkImageData *voiVolume;
  vtkITKArchetypeImageSeriesReader *reader1 = NULL;
  vtkITKArchetypeImageSeriesReader *reader2 = NULL;

  // check for the input files
  FILE * petfile;
    petfile = fopen(list.PETVolumeName.c_str(),"r");
    if (petfile == NULL)
    {
    std::cerr << "ERROR: cannot open input volume file " << list.PETVolumeName.c_str() << endl;
        return EXIT_FAILURE;
    }
    fclose(petfile);

  FILE * voifile;
    voifile = fopen(list.VOIVolumeName.c_str(),"r");
    if (voifile == NULL)
    {
    std::cerr << "ERROR: cannot open input volume file " << list.VOIVolumeName.c_str() << endl;
        return EXIT_FAILURE;
    }
    fclose(voifile);

    // Read the file
    reader1 = vtkITKArchetypeImageSeriesScalarReader::New();
//    vtkPluginFilterWatcher watchReader1 ( reader1, "Reading PET Volume", CLPProcessInformation );
    reader1->SetArchetype(list.PETVolumeName.c_str());
    reader1->SetOutputScalarTypeToNative();
    reader1->SetDesiredCoordinateOrientationToNative();
    reader1->SetUseNativeOriginOn();
    reader1->Update();
    std::cout << "Done reading the file " << list.PETVolumeName.c_str() << endl;

    // Read the file
    reader2 = vtkITKArchetypeImageSeriesScalarReader::New();
//    vtkPluginFilterWatcher watchReader2 ( reader2, "Reading VOI Volume", CLPProcessInformation );
    reader2->SetArchetype(list.VOIVolumeName.c_str());
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
  // COMPUTE SUV
  //
  if (petVolume == NULL)
    {
    std::cerr << "No input volume found." <<std::endl;
    return EXIT_FAILURE;
    }
  
  // find input labelmap volume
  if (voiVolume == NULL)
    {
    std::cerr <<  "No input volume found" << std::endl;
    return EXIT_FAILURE;
    }

  // convert from input units.
  if ( list.radioactivityUnits.c_str() == NULL )
    {
    std::cerr << "ComputeSUV: Got NULL radioactivity units. No computation done."<< std::endl;
    return EXIT_FAILURE;
    }
  if ( list.weightUnits.c_str() == NULL )
    {
    std::cerr << "ComputeSUV: Got NULL weight units. No computation could be done." << std::endl;
    return EXIT_FAILURE;
    }

  double suvmax, suvmin, suvmean;

  //--- find the max and min label in mask 
  vtkImageAccumulate *stataccum = vtkImageAccumulate::New();
  stataccum->SetInput ( voiVolume );
  stataccum->Update();
  int lo = static_cast<int>(stataccum->GetMin()[0]);
  int hi = static_cast<int>(stataccum->GetMax()[0]);
  stataccum->Delete();

  std::string labelName;
  int NumberOfVOIs = 0;
  for(int i = lo; i <= hi; i++ ) 
    {
    std::stringstream ss;
    if ( i == 0 )
      {
      //--- eliminate 0 (background) label.
      continue;
      }

    labelName.clear();
    labelName = MapLabelIDtoColorName(i);
    if ( labelName.empty() )
      {
      labelName.clear();
      labelName = "unknown";      
      }

    //--- get label name from labelID
    
    suvmax = 0.0;
    suvmean = 0.0;

    // create the binary volume of the label
    vtkImageThreshold *thresholder = vtkImageThreshold::New();
    thresholder->SetInput(voiVolume);
    thresholder->SetInValue(1);
    thresholder->SetOutValue(0);
    thresholder->ReplaceOutOn();
    thresholder->ThresholdBetween(i,i);
    thresholder->SetOutputScalarType(petVolume->GetScalarType());
    thresholder->Update();
    

    // use vtk's statistics class with the binary labelmap as a stencil
    vtkImageToImageStencil *stencil = vtkImageToImageStencil::New();
    stencil->SetInput(thresholder->GetOutput());
    stencil->ThresholdBetween(1, 1);
    

    vtkImageAccumulate *labelstat = vtkImageAccumulate::New();
    labelstat->SetInput(petVolume);
    labelstat->SetStencil(stencil->GetOutput());
    labelstat->Update();
   
    stencil->Delete();

    //--- For how many labels was SUV computed?

    int voxNumber = labelstat->GetVoxelCount();
    if ( voxNumber > 0 )
      {
      NumberOfVOIs++;

      double CPETmin = (labelstat->GetMin())[0];
      double CPETmax = (labelstat->GetMax())[0];
      double CPETmean = (labelstat->GetMean())[0];

      //--- we want to use the following units as noted at file top:
      //--- CPET(t) -- tissue radioactivity in pixels-- kBq/mlunits
      //--- injectced dose-- MBq and
      //--- patient weight-- kg.
      //--- computed SUV should be in units g/ml
      double weight = list.patientWeight;
      double dose = list.injectedDose;

      //--- do some error checking and reporting.
      if ( list.radioactivityUnits.c_str() == NULL )
        {
        std::cerr << "ComputeSUV: Got null radioactivityUnits." << std::endl;
        return EXIT_FAILURE;
        }
      if ( dose == 0.0 )
        {
        std::cerr << "ComputeSUV: Got NULL dose!" << std::endl;
        return EXIT_FAILURE;
        }
      if ( weight == 0.0 )
        {
        std::cerr <<"ComputeSUV: got zero weight!" << std::endl;
        return EXIT_FAILURE;
        }

      double tissueConversionFactor = ConvertRadioactivityUnits (1, list.radioactivityUnits.c_str(), "kBq");
      dose  = ConvertRadioactivityUnits ( dose, list.radioactivityUnits.c_str(), "MBq");
      dose = DecayCorrection (list, dose);
      weight = ConvertWeightUnits ( weight, list.weightUnits.c_str(), "kg");

      //--- check a possible multiply by slope -- take intercept into account?
      if ( dose == 0.0 )
        {
        // oops, weight by dose is infinity. make a ridiculous number.
        suvmin = 99999999999999999.;
        suvmax = 99999999999999999.;
        suvmean = 99999999999999999.;
        std::cerr <<"Warning: got an injected dose of 0.0. Results of SUV computation not valid." << std::endl;
        }
      else 
        {
        double weightByDose = weight / dose;
        suvmax = (CPETmax * tissueConversionFactor) * weightByDose;
        suvmin = (CPETmin * tissueConversionFactor ) * weightByDose;
        suvmean = (CPETmean * tissueConversionFactor) * weightByDose;
        }
      //--- write output file 
      // open file containing suvs and append to it.
      ofile.open ( outputFile.c_str(), ios::out | ios::app );
      if ( !ofile.is_open() )
        {
        // report error, clean up, and get out.
        std::cerr << "ERROR: cannot open nuclear medicine parameter file " << outputFile.c_str() << std::endl;
        ofile.close();
        return EXIT_FAILURE;
        }
      //--- for each value..
      //--- format looks like:
      // patientID, studyDate, dose, blood glucose, labelID, suvmax, suvmean, chemoStartDate, chemoEndDate, labelName ...
      ss << list.patientName << ", " << list.studyDate << ", " << list.injectedDose  << ", "  << i << ", " << suvmax << ", " << suvmean << ", " << labelName.c_str() << ", " << ", " << ", " << ", "<<std::endl;
      ofile << ss.str();
      ofile.close();
      ss.str("");
      }
    
    thresholder->Delete();
    labelstat->Delete();
    }
  return EXIT_SUCCESS;
  
}





} // end of anonymous namespace





//...
//...............................................................................................
//...
int main( int argc, char * argv[] )
{

  PARSE_ARGS;
  parameters list;

  //...
  //... strings used for parsing out DICOM header info
  //...  
  std::string yearstr;
  std::string monthstr;
  std::string daystr;
  std::string hourstr;
  std::string minutestr;
  std::string secondstr;
  std::string tag;

  //... parse command line
 
  if ( argc != 9 )
    {

    std::cerr << argv[0] << ": Bad command line: try " << argv[0] << " -petVolume FileName -labelMap FileName -parameterFile FileName(.dat) -csvFile FileName(.csv)" << std::endl;
    return EXIT_FAILURE;
    }

  try
    {
    //...
    // open file containing radiopharmaceutical data.
    //...
    list.PETVolumeName = argv[2];
    list.VOIVolumeName = argv[4];
    list.parameterFile = argv[6];
    list.SUVOutputTable = argv[8];

    std::ifstream pfile;
    pfile.open ( list.parameterFile.c_str(), ios::in );
    if ( !pfile.is_open() )
      {
      // report error, clean up, and get out.
      std::cerr << "ERROR: cannot open nuclear medicine parameter file " << list.parameterFile.c_str() << std::endl;
      pfile.close();
      return EXIT_FAILURE;
      }

    //...
    // read the metadata text file and grab all parameters.
    //...
    size_t colonPos;
    int numchars;
    std::string line;
    while (! pfile.eof() )
      {
      line.clear();
      // grab a line
      getline (pfile, line);



      // process a line.
      //
      // we want to find all of the following parameters.
      //
      std::string sep = ": ";
      size_t len = sep.size();

      //...
      // PATIENT ID
      //...
      if ( line.find ("Patient_Name: ") != std::string::npos )
        {
        // initialize
        numchars = 0;
        list.patientName = "MODULE_INIT_NO_VALUE";

        // find start of target string
        colonPos = line.find ( sep );
        if ( colonPos != std::string::npos )
          {
          list.patientName = line.substr ( colonPos+len );
          std::cout << "patientName = " << list.patientName.c_str() << std::endl;
          }
        // catch any failure to extract parameter value
        if ( list.patientName.find ( "MODULE_INIT_NO_VALUE") != std::string::npos )
          {
          std::cerr << "Unable to extract patient ID." << std::endl;
          }
        }
      //...
      // STUDY DATE
      //...
      else if ( line.find ("Study_Date: ")!= std::string::npos )
        {
        // initialize
        numchars = 0;
        list.studyDate = "MODULE_INIT_NO_VALUE";

        // find start of target string
        colonPos = line.find ( sep);
        if ( colonPos != std::string::npos )
          {
          // find end of target string
          list.studyDate = line.substr ( colonPos+len );
          std::cout << "studyDate = " << list.studyDate.c_str() << std::endl;
          }
        // catch any failure to extract parameter value
        if ( list.studyDate.find ( "MODULE_INIT_EMPTY_ID") != std::string::npos )
          {
          std::cerr << "Unable to extract study date." << std::endl;
          }
        else
          {
          //--- YYYYMMDD
          tag = list.studyDate;
          list.studyDate.clear();
          yearstr.clear();
          daystr.clear();
          monthstr.clear();
          len = tag.length();
          if ( len >= 4 )
            {
            yearstr = tag.substr(0, 4);
            }
          else
            {
            yearstr = "????";
            }
          if ( len >= 6 )
            {
            monthstr = tag.substr(4, 2);
            }
          else
            {
            monthstr = "??";
            }
          if ( len >= 8 )
            {
            daystr = tag.substr (6, 2);
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
      //...
      // UNITS (volume and radioactivity and weight)
      //...
      else if ( line.find ("Units: ") != std::string::npos )
        {
        // initialize
        numchars = 0;
        std::string units = "MODULE_INIT_NO_VALUE";

        // find start of target string
        colonPos = line.find ( sep);
        if ( colonPos != std::string::npos )
          {
          // find end of target string
          units.clear();
          units = line.substr ( colonPos+len );
          std::cout << "units = " << units.c_str() << std::endl;
          }
        // catch any failure to extract parameter value
        if ( units.find ( "MODULE_INIT_EMPTY_ID") != std::string::npos )
          {
          std::cerr << "Unable to extract units." << std::endl;
          }
        //--- hopefully...
        if ( units.find ( "BQML") != std::string::npos )
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
      //...
      // INJECTED DOSE
      //...
      else if ( line.find ("Radionuclide_Total_Dose: ") != std::string::npos )
        {
        // initialize
        numchars = 0;
        list.injectedDose = 0.0;
        std::string tmp;

        // find start of target string
        colonPos = line.find ( sep);
        if ( colonPos != std::string::npos )
          {
          // find end of target string
          tmp = line.substr ( colonPos+len );
          list.injectedDose = atof ( tmp.c_str() );
          std::cout << "injectedDose = " << list.injectedDose << std::endl;
          }
        // catch any failure to extract parameter value
        if ( list.injectedDose <= 0.0 )
          {
          std::cerr << "Unable to extract injected dose." << std::endl;
          }
        }
      //...
      // PATIENT WEIGHT
      //...
      else if ( line.find ("Patients_Weight: ") != std::string::npos )
        {
        // initialize
        numchars = 0;
        list.patientWeight  = 0.0;
        std::string tmp;

        // find start of target string
        colonPos = line.find ( sep);
        if ( colonPos != std::string::npos )
          {
          // find end of target string
          tmp.clear();
          tmp = line.substr ( colonPos+len );
          list.patientWeight = atof ( tmp.c_str() );
          std::cout << "patientWeight = " << list.patientWeight << std::endl;
          }
        // catch any failure to extract parameter value
        if ( list.patientWeight == 0.0 )
          {
          std::cerr << "Unable to extract patient weight." << std::endl;
          }
        }
      //...
      // SERIES TIME
      //...
      else if ( line.find ("Series_Time: ") != std::string::npos )
        {
        // initialize
        numchars = 0;
        list.seriesReferenceTime = "MODULE_INIT_NO_VALUE";

        // find start of target string
        colonPos = line.find ( sep);
        if ( colonPos != std::string::npos )
          {
          // find end of target string
          std::string tag = line.substr ( colonPos+len );
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
          std::cout << "seriesReferenceTime = " << list.seriesReferenceTime.c_str() << std::endl;
          }
        // catch any failure to extract parameter value
        if ( list.seriesReferenceTime.find ( "MODULE_INIT_EMPTY_ID") != std::string::npos )
          {
          std::cerr << "Unable to extract series reference time." << std::endl;
          }
        }
      //...
      // RADIOPHARMACEUTICAL START TIME
      //...
      else if ( line.find ("Radionuclide_Start_Time: ") != std::string::npos )
        {
        // initialize
        numchars = 0;
        list.injectionTime = "MODULE_INIT_NO_VALUE";
        std::string hourstr;
        std::string minutestr;
        std::string secondstr;

        // find start of target string
        colonPos = line.find ( sep);
        if ( colonPos != std::string::npos )
          {
          // find end of target string
          std::string tag = line.substr ( colonPos+len );
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
          list.injectionTime = tag.c_str();
          std::cout << "radiopharmaceuticalStartTime = " << list.injectionTime.c_str() << std::endl;
          }
        // catch any failure to extract parameter value
        if ( list.injectionTime.find ( "MODULE_INIT_EMPTY_ID") != std::string::npos )
          {
          std::cerr << "Unable to extract radiopharmaceutical start time." << std::endl;
          }
        }
      //...
      // DECAY CORRECTION
      //...
      else if ( line.find ("Decay_Correction:") != std::string::npos )
        {
        // initialize
        numchars = 0;
        list.decayCorrection = "MODULE_INIT_NO_VALUE";

        // find start of target string
        colonPos = line.find ( sep);
        if ( colonPos != std::string::npos )
          {
          // find end of target string
          list.decayCorrection = line.substr ( colonPos+len );
          std::cout << "decayCorrection = " << list.decayCorrection.c_str() << std::endl;
          }
        // catch any failure to extract parameter value
        if ( list.decayCorrection.find ( "MODULE_INIT_EMPTY_ID") != std::string::npos )
          {
          std::cerr << "Unable to extract decay correction." << std::endl;
          }
        }
      //...
      // DECAY FACTOR
      //...
      else if ( line.find ("DecayFactor:") != std::string::npos )
        {
        // initialize
        numchars = 0;
        list.decayFactor = "MODULE_INIT_EMPTY_ID";


        // find start of target string
        colonPos = line.find ( sep);
        if ( colonPos != std::string::npos )
          {
          // find end of target string
          list.decayFactor = line.substr ( colonPos+len );
          std::cout << "decayFactor = " << list.decayFactor.c_str() << std::endl;
          }
        // catch any failure to extract parameter value
        if ( list.decayFactor.find ( "MODULE_INIT_EMPTY_ID" ) != std::string::npos )
          {
          std::cerr << "Unable to extract decay factor." << std::endl;
          }
        }
      //...
      // RADIONUCLIDE HALF LIFE
      //...
      else if ( line.find ("Radionuclide_Half_Life:") != std::string::npos )
        {
        // initialize
        numchars = 0;
        list.radionuclideHalfLife = "MODULE_INIT_NO_VALUE";

        // find start of target string
        colonPos = line.find ( sep);
        if ( colonPos != std::string::npos )
          {
          // find end of target string
          list.radionuclideHalfLife = line.substr ( colonPos+len );
          std::cout << "radionuclideHalfLife = " << list.radionuclideHalfLife.c_str() << std::endl;
          }
        // catch any failure to extract parameter value
        if ( list.radionuclideHalfLife.find ("MODULE_INIT_NO_VALUE") != std::string::npos)
          {
          std::cerr << "Unable to extract radionuclide half life." << std::endl;
          }
        }
      //...
      // FRAME REFERENCE TIME
      //...
      //--- The time that the pixel values in the image
      //--- occurred. Frame Reference Time is the
      //--- offset, in msec, from the Series reference
      //--- time.      
      else if ( line.find ("Frame_Reference_Time: ") != std::string::npos )
        {
        // initialize
        numchars = 0;
        list.frameReferenceTime = "MODULE_INIT_NO_VALUE";

        // find start of target string
        colonPos = line.find ( sep);
        if ( colonPos != std::string::npos )
          {
          // find end of target string
          list.frameReferenceTime = line.substr ( colonPos+len );
          std::cout << "frameReferenceTime = " << list.frameReferenceTime.c_str() << std::endl;
          }
        // catch any failure to extract parameter value
        if ( list.frameReferenceTime.find ( "MODULE_INIT_NO_VALUE") != std::string::npos )
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
    LoadImagesAndComputeSUV( list, static_cast<double>(0));
    }
  
  catch( itk::ExceptionObject &excep)
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
