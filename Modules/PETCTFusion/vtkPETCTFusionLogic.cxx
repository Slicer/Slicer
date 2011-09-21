#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <vtksys/SystemTools.hxx>
#include <vtksys/Directory.hxx>

// To avoid link errors/warnings gdcm.h shall be included before any other gdcm headers
#include "gdcm.h" 

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkIntArray.h"
#include "vtkImageAccumulate.h"
#include "vtkImageThreshold.h"
#include "vtkImageToImageStencil.h"
#include "vtkImageMathematics.h"
#include "vtkLookupTable.h"
#include "vtkColorTransferFunction.h"

#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLFreeSurferProceduralColorNode.h"
#include "vtkMRMLProceduralColorNode.h"
#include "vtkMRMLPETProceduralColorNode.h"
#include "vtkMRMLScalarVolumeNode.h"

#include "itkImageSeriesReader.h"
#include "itkImage.h"
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

#include "vtkPETCTFusionLogic.h"
#include "vtkPETCTFusion.h"

#include "vtkMRMLScene.h"


//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkPETCTFusionLogic);


//----------------------------------------------------------------------------
vtkPETCTFusionLogic::vtkPETCTFusionLogic()
{
  //--- GET RADIOACTIVITY UNITS FROM DICOM HEADER.
  //--- make SURE that the radioactivity units in the image are cancelled out in the SUV calculation!
  //--- if necessary convert the image units, or the units in injected dose.
  this->PETCTFusionNode = NULL;
  this->Plots = NULL;
//  this->Plots = vtkPETCTFusionPlots::New();
  this->NumberOfVOIs = 0;
  this->Year = 0;
  this->Month = 0;
  this->Day = 0;
}



//----------------------------------------------------------------------------
vtkPETCTFusionLogic::~vtkPETCTFusionLogic()
{ 
  this->ClearStudyDate();
  this->SetAndObserveMRMLScene ( NULL );
  if ( this->Plots )
    {
    this->Plots->Delete();
    this->Plots = NULL;
    }
  if ( this->PETCTFusionNode )
    {
    this->SetPETCTFusionNode ( NULL );
    }
}




//----------------------------------------------------------------------------
void vtkPETCTFusionLogic::ClearStudyDate()
{
  this->Year = 0;
  this->Month = 0;
  this->Day = 0;
}

//----------------------------------------------------------------------------
void vtkPETCTFusionLogic::Enter()
{

  this->Visited = true;
  this->Raised = true;

  //---
  //--- Set up Logic observers on enter, and released on exit.
  vtkIntArray *logicEvents = this->NewObservableEvents();
  if ( logicEvents != NULL )
    {
    this->SetAndObserveMRMLSceneEvents ( this->MRMLScene, logicEvents );
    logicEvents->Delete();
    }

}


//----------------------------------------------------------------------------
void vtkPETCTFusionLogic::Exit()
{
  this->Raised = false;
}



//----------------------------------------------------------------------------
vtkIntArray* vtkPETCTFusionLogic::NewObservableEvents()
{

  if ( !this->Visited )
    {
    return (NULL);
    }

 vtkIntArray *events = vtkIntArray::New();
 events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  // Slicer3.cxx calls delete on events
  return events;
}

//----------------------------------------------------------------------------
void vtkPETCTFusionLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
int vtkPETCTFusionLogic::GetParametersFromDICOMHeader( const char *path)
{

  if ( this->PETCTFusionNode == NULL )
    {
    vtkErrorMacro ( "GetParametersFromDICOMHeader: Got NULL PETCTFusionNode." );
    return 0;
    }


  typedef short PixelValueType;
  typedef itk::Image< PixelValueType, 3 > VolumeType;
  typedef itk::ImageSeriesReader< VolumeType > ReaderType;
  typedef itk::Image< PixelValueType, 2 > SliceType;
  typedef itk::ImageFileReader< SliceType > SliceReaderType;
  typedef itk::GDCMImageIO ImageIOType;
  typedef itk::GDCMSeriesFileNames InputNamesGeneratorType;
  typedef itk::VectorImage< PixelValueType, 3 > NRRDImageType;
    
  if ( path == NULL || !(strcmp(path, "" )) )
    {
    vtkErrorMacro ( "GetParametersFromDicomHeader:Got NULL path." );
    return 0;
    }
  if ( this->PETCTFusionNode == NULL )
    {
    vtkErrorMacro ( "GetParametersFromDICOMHeader: Got NULL PETCTFusionNode." );
    return 0;
    }

  //--- catch non-dicom data
  vtkGlobFileNames* gfn = vtkGlobFileNames::New();
  gfn->SetDirectory(path);
  gfn->AddFileNames("*.nhdr");
  gfn->AddFileNames("*.nrrd");
  gfn->AddFileNames("*.hdr");
  gfn->AddFileNames("*.mha");
  gfn->AddFileNames("*.img");
  gfn->AddFileNames("*.nii");
  gfn->AddFileNames("*.nia");

  this->PETCTFusionNode->SetMessageText ( "Checking for DICOM header...." );
  this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::StatusEvent );

  int notDICOM = 0;
  int nFiles = gfn->GetNumberOfFileNames();
  if (nFiles > 0)
    {
    //--- invoke error via the node
    this->PETCTFusionNode->SetInjectedDose ( 0.0 );
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
    }
  gfn->Delete();
  if ( notDICOM )
    {
    //--- Tell the user they will have to enter
    //--- parameters for computing SUV manually.
    this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::NonDICOMEvent );
    return 0;
    }

  this->PETCTFusionNode->SetMessageText ( "Getting number of series UIDs...." );
  this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::StatusEvent );

  InputNamesGeneratorType::Pointer inputNames = InputNamesGeneratorType::New();
  inputNames->SetUseSeriesDetails(true);
  inputNames->SetDirectory(path);
  itk::SerieUIDContainer seriesUIDs = inputNames->GetSeriesUIDs();
  
  this->PETCTFusionNode->SetMessageText ( "Getting Filenames...." );
  this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::StatusEvent );

  const ReaderType::FileNamesContainer & filenames = inputNames->GetFileNames(seriesUIDs[0]);
      
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
            this->PETCTFusionNode->SetRadiopharmaceuticalStartTime ("no value found");
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
            this->PETCTFusionNode->SetRadiopharmaceuticalStartTime( tag.c_str() );
            }

          //---
          //--- Radionuclide Total Dose 
          tag.clear();
          tag = sqItem->GetEntryValue(0x0018,0x1074);
          if ( tag.c_str() == NULL || *(tag.c_str()) == '\0' )
            {
            this->PETCTFusionNode->SetInjectedDose( 0.0 );
            }
          else
            {
            this->PETCTFusionNode->SetInjectedDose( atof ( tag.c_str() ) );
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
            this->PETCTFusionNode->SetRadionuclideHalfLife( "no value found" );
            }
          else
            {
            this->PETCTFusionNode->SetRadionuclideHalfLife(  tag.c_str() );
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
            this->PETCTFusionNode->SetDoseRadioactivityUnits ("Bq");
            this->PETCTFusionNode->SetTissueRadioactivityUnits ("Bq");
            }
          else if ( ( units.find ("MBq") != std::string::npos) ||
                    ( units.find ("MBQ") != std::string::npos) )
            {
            this->PETCTFusionNode->SetDoseRadioactivityUnits ("MBq");
            this->PETCTFusionNode->SetTissueRadioactivityUnits ("MBq");
            }
          else if ( (units.find ("kBq") != std::string::npos) ||
                    (units.find ("kBQ") != std::string::npos) ||
                    (units.find ("KBQ") != std::string::npos) )
            {
            this->PETCTFusionNode->SetDoseRadioactivityUnits ("kBq");
            this->PETCTFusionNode->SetTissueRadioactivityUnits ("kBq");
            }
          else if ( (units.find ("mBq") != std::string::npos) ||
                    (units.find ("mBQ") != std::string::npos) )
            {
            this->PETCTFusionNode->SetDoseRadioactivityUnits ("mBq");
            this->PETCTFusionNode->SetTissueRadioactivityUnits ("mBq");
            }
          else if ( (units.find ("uBq") != std::string::npos) ||
                    (units.find ("uBQ") != std::string::npos) )
            {
            this->PETCTFusionNode->SetDoseRadioactivityUnits ("uBq");
            this->PETCTFusionNode->SetTissueRadioactivityUnits ("uBq");
            }
          else if ( (units.find ("Bq") != std::string::npos) ||
                    (units.find ("BQ") != std::string::npos) )
            {
            this->PETCTFusionNode->SetDoseRadioactivityUnits ("Bq");
            this->PETCTFusionNode->SetTissueRadioactivityUnits ("Bq");
            }
          else if ( (units.find ("MCi") != std::string::npos) ||
                    ( units.find ("MCI") != std::string::npos) )
            {
            this->PETCTFusionNode->SetDoseRadioactivityUnits ("MCi");
            this->PETCTFusionNode->SetTissueRadioactivityUnits ("MCi");

            }
          else if ( (units.find ("kCi") != std::string::npos) ||
                    (units.find ("kCI") != std::string::npos)  ||
                    (units.find ("KCI") != std::string::npos) )                
            {
            this->PETCTFusionNode->SetDoseRadioactivityUnits ("kCi");
            this->PETCTFusionNode->SetTissueRadioactivityUnits ("kCi");
            }
          else if ( (units.find ("mCi") != std::string::npos) ||
                    (units.find ("mCI") != std::string::npos) )                
            {
            this->PETCTFusionNode->SetDoseRadioactivityUnits ("mCi");
            this->PETCTFusionNode->SetTissueRadioactivityUnits ("mCi");
            }
          else if ( (units.find ("uCi") != std::string::npos) ||
                    (units.find ("uCI") != std::string::npos) )                
            {
            this->PETCTFusionNode->SetDoseRadioactivityUnits ("uCi");
            this->PETCTFusionNode->SetTissueRadioactivityUnits ("uCi");
            }
          else if ( (units.find ("Ci") != std::string::npos) ||
                    (units.find ("CI") != std::string::npos) )                
            {
            this->PETCTFusionNode->SetDoseRadioactivityUnits ("Ci");
            this->PETCTFusionNode->SetTissueRadioactivityUnits ("Ci");
            }
          this->PETCTFusionNode->SetVolumeUnits ( "ml" );
          }
        else
          {
          //--- default values.
          this->PETCTFusionNode->SetDoseRadioactivityUnits( "MBq" );
          this->PETCTFusionNode->SetTissueRadioactivityUnits( "MBq" );
          this->PETCTFusionNode->SetVolumeUnits ( "ml");        
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
          this->PETCTFusionNode->SetDecayCorrection( tag.c_str() );
          }
        else
          {
          this->PETCTFusionNode->SetDecayCorrection( "no value found" );
          }

        //---
        //--- StudyDate
        this->ClearStudyDate();
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
            this->Year = atoi(yearstr.c_str() );
            }
          else
            {
            yearstr = "????";
            this->Year = 0;
            }
          if ( len >= 6 )
            {
            monthstr = tag.substr(4, 2);
            this->Month = atoi ( monthstr.c_str() );
            }
          else
            {
            monthstr = "??";
            this->Month = 0;
            }
          if ( len >= 8 )
            {
            daystr = tag.substr (6, 2);
            this->Day = atoi ( daystr.c_str() );
            }
          else
            {
            daystr = "??";
            this->Day = 0;
            }
          tag.clear();
          tag = yearstr.c_str();
          tag += "/";
          tag += monthstr.c_str();
          tag += "/";
          tag += daystr.c_str();
          this->PETCTFusionNode->SetStudyDate ( tag.c_str() );
          }
        else
          {
          this->PETCTFusionNode->SetStudyDate ( "no value found" );
          }

        //---
        //--- PatientName
        tag.clear();
        tag = f->GetEntryValue (0x0010,0x0010);
        if ( tag.c_str() != NULL && strcmp (tag.c_str(), "" ) )
          {
          this->PETCTFusionNode->SetPatientName ( tag.c_str() );
          }
        else
          {
          this->PETCTFusionNode->SetPatientName ( "no value found");
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
          this->PETCTFusionNode->SetDecayFactor(  tag.c_str()  );
          }
        else
          {
          this->PETCTFusionNode->SetDecayFactor( "no value found" );
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
          this->PETCTFusionNode->SetFrameReferenceTime( tag.c_str() );
          }
        else
          {
          this->PETCTFusionNode->SetFrameReferenceTime( "no value found" );
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
          this->PETCTFusionNode->SetSeriesTime( tag.c_str() );
          }
        else
          {
          this->PETCTFusionNode->SetSeriesTime( "no value found");
          }


        //---
        //--- PatientWeight
        tag.clear();
        tag = f->GetEntryValue (0x0010,0x1030);
        if ( tag.c_str() != NULL && strcmp(tag.c_str(), "" ) )
          {
          //--- Expect same format as RadionuclideHalfLife
          this->PETCTFusionNode->SetPatientWeight( atof ( tag.c_str() ) );
          this->PETCTFusionNode->SetWeightUnits ( "kg" );
          }
        else
          {
          this->PETCTFusionNode->SetPatientWeight( 0.0 );
          this->PETCTFusionNode->SetWeightUnits ( "" );
          }


        //---
        //--- CalibrationFactor
        tag.clear();
        tag = f->GetEntryValue (0x7053,0x1009);
        if ( tag.c_str() != NULL && strcmp(tag.c_str(), "" ) )
          {
          //--- converts counts to Bq/cc. If Units = BQML then CalibrationFactor =1 
          //--- I think we expect the same format as RadiopharmaceuticalStartTime
          this->PETCTFusionNode->SetCalibrationFactor(  tag.c_str() );
          }
        else
          {
          this->PETCTFusionNode->SetCalibrationFactor( "no value found" );
          }


        //---
        //--- PhilipsSUVFactor
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
        }
      }
    //END TEST
    delete f;


    // check.... did we get all params we need for computation?
    if ( (parsingDICOM) &&
         (this->PETCTFusionNode->GetInjectedDose() != 0.0) &&
         (this->PETCTFusionNode->GetPatientWeight() != 0.0) &&
         (this->PETCTFusionNode->GetSeriesTime() != NULL) &&
         (this->PETCTFusionNode->GetRadiopharmaceuticalStartTime() != NULL) &&
         (this->PETCTFusionNode->GetRadionuclideHalfLife() != NULL) )
      {
      this->PETCTFusionNode->SetMessageText ( "...Done" );
      this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::StatusEvent );

      this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::DICOMUpdateEvent );

      this->PETCTFusionNode->SetMessageText ( "" );
      this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::StatusEvent );
      return 1;
      }
    else
      {
      this->PETCTFusionNode->SetMessageText ( "...no PET-related metadata found." );
      this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::StatusEvent );

      this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::DICOMUpdateEvent );

      this->PETCTFusionNode->SetMessageText ( "" );
      this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::StatusEvent );
      return 0;
      }
}




//----------------------------------------------------------------------------
double vtkPETCTFusionLogic::ConvertImageUnitsToSUVUnits( double voxValue )
{

  //--- Units:
  //--- image data = CPET(t) (tissue radioactivity in pixels) -- kBq/ml
  //--- injected dose-- MBq and
  //--- patient weight-- kg.
  //--- computed SUV should be in units g/ml
  //---
  double weight = this->PETCTFusionNode->GetPatientWeight();
  double dose = this->PETCTFusionNode->GetInjectedDose();
  if ( weight == 0.0 || dose == 0.0 || this->PETCTFusionNode->GetTissueRadioactivityUnits() == NULL )
    {
    return ( voxValue );
    }

  double tissueConversionFactor = this->ConvertRadioactivityUnits (1, this->PETCTFusionNode->GetTissueRadioactivityUnits(), "kBq");
  dose  = this->ConvertRadioactivityUnits ( dose, this->PETCTFusionNode->GetDoseRadioactivityUnits(), "MBq");
  dose = this->DecayCorrection (dose );
  weight = this->ConvertWeightUnits ( weight, this->PETCTFusionNode->GetWeightUnits(), "kg");

  double weightByDose = weight / dose;
  double suvVal = (voxValue * tissueConversionFactor) * weightByDose;

  return ( suvVal);
}





//----------------------------------------------------------------------------
double vtkPETCTFusionLogic::ConvertSUVUnitsToImageUnits( double suvValue )
{

  //--- Units:
  //--- SUV should be in units g/ml
  //--- injected dose-- MBq and
  //--- patient weight-- kg.
  //--- computed image units:kBq/ml
  //---
  double weight = this->PETCTFusionNode->GetPatientWeight();
  double dose = this->PETCTFusionNode->GetInjectedDose();

  if ( weight == 0.0 || dose == 0.0 || this->PETCTFusionNode->GetTissueRadioactivityUnits() == NULL )
    {
    return ( suvValue );
    }

  double tissueConversionFactor = this->ConvertRadioactivityUnits (1, this->PETCTFusionNode->GetTissueRadioactivityUnits(), "kBq");
  dose  = this->ConvertRadioactivityUnits ( dose, this->PETCTFusionNode->GetDoseRadioactivityUnits(), "MBq");
  dose = this->UndoDecayCorrection ( dose );
  weight = this->ConvertWeightUnits ( weight, this->PETCTFusionNode->GetWeightUnits(), "kg");

  double weightByDose = weight / dose;
  double voxValue = (suvValue / weightByDose) / tissueConversionFactor;

  return ( voxValue);
}





//----------------------------------------------------------------------------
double vtkPETCTFusionLogic::ConvertTimeToSeconds(const char *time)
{
  if ( time == NULL )
    {
    vtkErrorMacro ( "ConvertTimeToSeconds got a NULL time string." );
    return (-1.0);
    }

  std::string h;
  std::string m;
  std::string minAndsecStr;
  std::string secStr;  

  double hours;
  double minutes;
  double seconds;
  
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





//----------------------------------------------------------------------------
double vtkPETCTFusionLogic::DecayCorrection ( double inVal )
{
  double scanTimeSeconds = this->ConvertTimeToSeconds (this->PETCTFusionNode->GetSeriesTime() );
  double startTimeSeconds = this->ConvertTimeToSeconds ( this->PETCTFusionNode->GetRadiopharmaceuticalStartTime() );
  if ( scanTimeSeconds < 0.0 || startTimeSeconds < 0.0 )
    {
    this->PETCTFusionNode->SetMessageText ( "Note: Scan start time and Radiopharmaceutical start time were not found or specified. SUV results will not be properly decay corrected." );
    this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::ErrorEvent );
    return ( inVal );
    }

  double halfLife = atof ( this->PETCTFusionNode->GetRadionuclideHalfLife() );
  double decayTime = scanTimeSeconds-startTimeSeconds;
  double correctedVal = inVal * (double)pow(2.0, -(decayTime/halfLife) );
  return ( correctedVal );

}


//----------------------------------------------------------------------------
double vtkPETCTFusionLogic::UndoDecayCorrection ( double inVal )
{
  double scanTimeSeconds = this->ConvertTimeToSeconds (this->PETCTFusionNode->GetSeriesTime() );
  double startTimeSeconds = this->ConvertTimeToSeconds ( this->PETCTFusionNode->GetRadiopharmaceuticalStartTime() );
  //--- check to see if RadiopharmaceuticalStartTime or ScanTime were not specified...
  //--- if not, don't correct value.
  if ( scanTimeSeconds < 0.0 || startTimeSeconds < 0.0 )
    {
    return ( inVal );
    }
  double halfLife = atof ( this->PETCTFusionNode->GetRadionuclideHalfLife() );
  double decayTime = scanTimeSeconds-startTimeSeconds;
  double correctedVal = inVal / (double)pow(2.0, -(decayTime/halfLife) );
  return ( correctedVal );

}




//----------------------------------------------------------------------------
void vtkPETCTFusionLogic::ComputeSUVmax()
{
  if ( this->PETCTFusionNode == NULL )
    {
    vtkErrorMacro ( "ComputeSUVMax: Got NULL PETCTFusionNode. ");
    return;
    }

  //---
  //--- set SUVmax to be the pixel data max if no SUV attributes have been assigned yet.
  //---
  double weight = this->PETCTFusionNode->GetPatientWeight();
  double dose = this->PETCTFusionNode->GetInjectedDose();
  //---
  //--- go no further if we haven't yet retrieved values from header,
  //--- or have no manually entered values for dose and weight.
  //---
  if ( weight == 0.0 || dose == 0.0 || this->PETCTFusionNode->GetTissueRadioactivityUnits() == NULL )
    {
    this->PETCTFusionNode->SetPETSUVmax ( this->PETCTFusionNode->GetPETMax() );
    return;
    }
  double tissueConversionFactor = this->ConvertRadioactivityUnits (1, this->PETCTFusionNode->GetTissueRadioactivityUnits(), "kBq");
  dose  = this->ConvertRadioactivityUnits ( dose, this->PETCTFusionNode->GetDoseRadioactivityUnits(), "MBq");
  dose = this->DecayCorrection ( dose );

  weight = this->ConvertWeightUnits ( weight, this->PETCTFusionNode->GetWeightUnits(), "kg");
  double weightByDose = weight / dose;
  double suvmax = (this->PETCTFusionNode->GetPETMax() * tissueConversionFactor) * weightByDose;
  this->PETCTFusionNode->SetPETSUVmax ( suvmax );

}




//----------------------------------------------------------------------------
void vtkPETCTFusionLogic::ComputeSUV()
{
  if ( this->PETCTFusionNode == NULL )
    {
    vtkErrorMacro ( "ComputeSUV: Got NULL PETCTFusionNode. ");
    return;
    }
  
  // find input PET volume
  vtkMRMLScalarVolumeNode *PETVolume = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->PETCTFusionNode->GetInputPETReference()));
  if (PETVolume == NULL)
    {
    vtkErrorMacro("No input volume found with id= " << this->PETCTFusionNode->GetInputPETReference());
    return;
    }
  

  // find input labelmap volume
  vtkMRMLScalarVolumeNode *maskVolume =  vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->PETCTFusionNode->GetInputMask()));
  if (maskVolume == NULL)
    {
    vtkErrorMacro("No input volume found with id= " << this->PETCTFusionNode->GetInputMask());
    return;
    }


  //--- convert from input units.
  if ( this->PETCTFusionNode->GetDoseRadioactivityUnits() == NULL )
    {
    vtkErrorMacro ( "ComputeSUV: Got NULL DoseRadioactivityUnits.No computation could be done." );
    return;
    }
  if ( this->PETCTFusionNode->GetTissueRadioactivityUnits() == NULL )
    {
    vtkErrorMacro ( "ComputeSUV: Got NULL TissueRadioactivityUnits. No computation could be done." );
    return;
    }
  if ( this->PETCTFusionNode->GetWeightUnits() == NULL )
    {
    vtkErrorMacro ( "ComputeSUV: Got NULL WeightUnits. No computation could be done." );
    return;
    }
  

  this->PETCTFusionNode->SetMessageText ( "Starting SUV calculation...." );
  this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::StatusEvent );

  //--- find the max and min label in mask (mostly there'll be just one label i think
  vtkImageAccumulate *stataccum = vtkImageAccumulate::New();
  stataccum->SetInput ( maskVolume->GetImageData() );
  stataccum->Update();

  int lo = static_cast<int>(stataccum->GetMin()[0]);
  int hi = static_cast<int>(stataccum->GetMax()[0]);
  stataccum->Delete();

  this->NumberOfVOIs = 0;
  std::stringstream ss;

  for(int i = lo; i <= hi; i++ ) 
    {
    if ( i == 0 )
      {
      //--- eliminate 0 (nolabel) label.
      continue;
      }


    //--- Provide some feedback
    ss.str("");
    ss << "Processing label ";
    ss << i;
    this->PETCTFusionNode->SetMessageText ( ss.str().c_str() );
    this->PETCTFusionNode->InvokeEvent(vtkMRMLPETCTFusionNode::StatusEvent );

    // create the binary volume of the label
    vtkImageThreshold *thresholder = vtkImageThreshold::New();
    thresholder->SetInput(maskVolume->GetImageData());
    thresholder->SetInValue(1);
    thresholder->SetOutValue(0);
    thresholder->ReplaceOutOn();
    thresholder->ThresholdBetween(i,i);
    thresholder->SetOutputScalarType(PETVolume->GetImageData()->GetScalarType());
    thresholder->Update();
    

    // use vtk's statistics class with the binary labelmap as a stencil
    vtkImageToImageStencil *stencil = vtkImageToImageStencil::New();
    stencil->SetInput(thresholder->GetOutput());
    stencil->ThresholdBetween(1, 1);
    
    vtkImageAccumulate *labelstat = vtkImageAccumulate::New();
    labelstat->SetInput(PETVolume->GetImageData());
    labelstat->SetStencil(stencil->GetOutput());
    labelstat->Update();
   
    stencil->Delete();

    //--- For how many labels was SUV computed?
    this->NumberOfVOIs++;

    int voxNumber = labelstat->GetVoxelCount();
    double suvmax, suvmin, suvmean;
    if ( voxNumber > 0 )
      {
      double CPETmin = (labelstat->GetMin())[0];
      double CPETmax = (labelstat->GetMax())[0];
      double CPETmean = (labelstat->GetMean())[0];

      //--- we want to use the following units as noted at file top:
      //--- CPET(t) -- tissue radioactivity in pixels-- kBq/mlunits
      //--- injectced dose-- MBq and
      //--- patient weight-- kg.
      //--- computed SUV should be in units g/ml
      double weight = this->PETCTFusionNode->GetPatientWeight();
      double dose = this->PETCTFusionNode->GetInjectedDose();

      //--- do some error checking and reporting.
      if ( this->PETCTFusionNode->GetTissueRadioactivityUnits() == NULL )
        {
        this->PETCTFusionNode->SetMessageText ( "No Radioactivity Units were specified. Cannot compute a valid SUV." );
        this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::ErrorEvent );
        return;
        }
      if ( dose == 0.0 )
        {
        this->PETCTFusionNode->SetMessageText ( "Injected dose was found to be 0.0. Cannot compute a valid SUV." );
        this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::ErrorEvent );
        return;
        }
      if ( weight == 0.0 )
        {
        this->PETCTFusionNode->SetMessageText ( "Patient Weight was found to be 0.0. Cannot compute a valid SUV." );
        this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::ErrorEvent );
        return;
        }
      double tissueConversionFactor = this->ConvertRadioactivityUnits (1, this->PETCTFusionNode->GetTissueRadioactivityUnits(), "kBq");
      dose  = this->ConvertRadioactivityUnits ( dose, this->PETCTFusionNode->GetDoseRadioactivityUnits(), "MBq");
      dose = this->DecayCorrection (dose);
      weight = this->ConvertWeightUnits ( weight, this->PETCTFusionNode->GetWeightUnits(), "kg");

      //--- check a possible multiply by slope -- take intercept into account?
      if ( dose != 0.0 )
        {
        double weightByDose = weight / dose;
        suvmax = (CPETmax * tissueConversionFactor) * weightByDose;
        suvmin = (CPETmin * tissueConversionFactor ) * weightByDose;
        suvmean = (CPETmean * tissueConversionFactor) * weightByDose;
        }
      else
        {
        suvmin = 99999999999999999.;
        suvmax = 99999999999999999.;
        suvmean = 99999999999999999.;
        vtkErrorMacro ( "Warning: got an injected dose of 0.0; results of SUV computation are not valid." );
        }
      }
    else
      {
      //--- if there were no voxels in VOI:
      suvmax = 0.0;
      suvmin = 0.0;
      suvmean = 0.0;
      }

    // add an entry to the SUVEntry if there were voxels in VOI
    if ( voxNumber > 0 )
      {
      vtkMRMLPETCTFusionNode::SUVEntry entry;
      entry.Label = i;
      entry.Max = suvmax;
      entry.Mean = suvmean;
      this->PETCTFusionNode->LabelResults.push_back(entry);
      }
    
    if ( this->Plots )
      {
      //--- if we have plottable information, add a sample  to the plot.
      //--- (each label gets its own plot)
      if ( this->Plots->GetMRMLScene() == NULL )
        {
        this->Plots->SetMRMLScene ( this->GetMRMLScene() );
        }
      this->Plots->AddSUV ( i, this->Year, this->Month, this->Day, suvmax, suvmean );
      }
    
    thresholder->Delete();
    labelstat->Delete();
    }
  
  this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::ComputeDoneEvent );

}


//----------------------------------------------------------------------------
/*
  double *vtkPETCTFusionLogic::GetColorForLabel ( int label )
{
  //---
  //--- find the color (rgb) for a given label in label map.
  //--- first, find the number of colors and the color range.
  //--- which will depend on what kind of color node is selected.
  bool isFSProcedural = false;
  bool isProcedural = false;
  int numColors = 0;
  double *range = NULL;

  if ( this->PETCTFusionNode->GetInputMask()==NULL )
    {
    return NULL;
    }
  vtkMRMLVolumeNode *vn = vtkMRMLVolumeNode::SafeDownCast (
                                                           this->MRMLScene->GetNodeByID(
                                                                                        this->PETCTFusionNode->GetInputMask() ));
  if ( vn == NULL )
    {
    return NULL;
    }
  vtkMRMLDisplayNode *dn = vn->GetDisplayNode();
  if ( dn == NULL )
    {
    return NULL;
    }
  vtkMRMLColorNode *colorNode = dn->GetColorNode();
      
  if ( colorNode == NULL )
    {
    return NULL;
    }

  if ( vtkMRMLColorTableNode::SafeDownCast (colorNode) != NULL )
    {
    //---
    //--- color table node.
    //---
    numColors = vtkMRMLColorTableNode::SafeDownCast(colorNode)->GetNumberOfColors();
    range = vtkMRMLColorTableNode::SafeDownCast(colorNode)->GetLookupTable()->GetRange();
    }
  else if ( vtkMRMLFreeSurferProceduralColorNode::SafeDownCast(colorNode) != NULL &&
            vtkMRMLFreeSurferProceduralColorNode::SafeDownCast(colorNode)->GetLookupTable() != NULL )
    {
    //---
    //--- freesurfer procedural node
    //---
    isFSProcedural = true;
    range = vtkMRMLFreeSurferProceduralColorNode::SafeDownCast (colorNode)->GetLookupTable()->GetRange();
    if ( range )
      {
      numColors = (int)floor(range[1]-range[0]);
      if ( range[0] < 0 && range[1] >= 0)
        {
        numColors++;
        }
      }
    }
  else if ( vtkMRMLProceduralColorNode::SafeDownCast(colorNode) != NULL &&
            vtkMRMLProceduralColorNode::SafeDownCast(colorNode)->GetColorTransferFunction() != NULL )
    {
    //---
    //--- procedural node
    //---
    isProcedural = true;
    range = vtkMRMLProceduralColorNode::SafeDownCast(colorNode)->GetColorTransferFunction()->GetRange();
    if ( range )
      {
      numColors = (int)floor(range[1]-range[0]);
      if ( range[0] < 0 && range[1] >=0)
        {
        numColors++;
        }
      }
    }
      
  //---
  double color[3];
  if (isFSProcedural)
    {
    if ( label <= range[1] )
      {
      vtkMRMLFreeSurferProceduralColorNode::SafeDownCast ( colorNode)->GetLookupTable()->GetColor(label, color);
      }
    }
  else if ( isProcedural)
    {
    vtkMRMLProceduralColorNode::SafeDownCast(colorNode)->GetColorTransferFunction()->GetColor(label, color);
    }
  else if ( colorNode->GetLookupTable() != NULL )
    {
    colorNode->GetLookupTable()->GetColor((double)label, color);
    }

  return ( color );
}
*/




//----------------------------------------------------------------------------
void vtkPETCTFusionLogic::ShowLongitudinalPlot ( )
{

  // TODO -- add plot functionality
  if ( this->Plots && this->PETCTFusionNode )
    {
    this->Plots->Update();
    this->PETCTFusionNode->InvokeEvent ( vtkMRMLPETCTFusionNode::PlotReadyEvent );
    }
}


//----------------------------------------------------------------------------
void vtkPETCTFusionLogic::ClearLongitudinalPlot ( )
{

  // TODO -- add plot functionality
  if ( this->Plots )
    {
    this->Plots->ClearPlots();
    }
}


//----------------------------------------------------------------------------
void vtkPETCTFusionLogic::ComputePercentChange()
{
}

//----------------------------------------------------------------------------
void vtkPETCTFusionLogic::SetAndScaleLUT()
{
}

//----------------------------------------------------------------------------
double vtkPETCTFusionLogic::ConvertWeightUnits ( double count,
                                                 const char *fromunits,
                                                 const char *tounits )
{
  double conversion = count;

  if ( fromunits == NULL && this->PETCTFusionNode != NULL)
    {
    vtkWarningMacro ( "Got NULL parameter fromunits. Either PET volume is non-DICOM, or a bad parameter was specified." );
    return (-1.0);
    }
  if ( tounits == NULL && this->PETCTFusionNode != NULL )
    {
    vtkWarningMacro ( "Got NULL parameter tounits. Either PET volume is non-DICOM, or a bad parameter was specified." );
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


//----------------------------------------------------------------------------
double vtkPETCTFusionLogic::ConvertRadioactivityUnits (double count,
                                              const char *fromunits,
                                              const char *tounits)
{

  double conversion = count;
  if ( fromunits == NULL && this->PETCTFusionNode != NULL)
    {
    vtkWarningMacro ( "Got NULL parameter fromunits. Either PET volume is non-DICOM, or a bad parameter was specified." );
    return (-1.0);
    }
  if ( tounits == NULL && this->PETCTFusionNode != NULL )
    {
    vtkWarningMacro ( "Got NULL parameter tounits. Either PET volume is non-DICOM, or a bad parameter was specified." );
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





