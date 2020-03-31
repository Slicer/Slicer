/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

// VTKITK includes
#include "vtkITKArchetypeImageSeriesReader.h"

// VTK includes
#include <vtkDataArray.h>
#include <vtkErrorCode.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkStreamingDemandDrivenPipeline.h>

// ITK includes
#include <itkNiftiImageIO.h>
#include <itkNrrdImageIO.h>
#include <itkMetaDataDictionary.h>
#include <itkMetaDataObjectBase.h>
#include <itkMetaDataObject.h>
#include <itkMetaImageIO.h>
#include <itkTimeProbe.h>

// STD includes
#include <algorithm>
#include <vector>

#include "itkArchetypeSeriesFileNames.h"
#include "itkOrientImageFilter.h"
#include "itkImageSeriesReader.h"
#ifdef VTKITK_BUILD_DICOM_SUPPORT
#include "itkDCMTKImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkGDCMImageIO.h"
#endif

vtkStandardNewMacro(vtkITKArchetypeImageSeriesReader);

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesReader::vtkITKArchetypeImageSeriesReader()
{
  this->Archetype  = nullptr;
  this->IndexArchetype = 0;
  this->SingleFile = 1;
  this->UseOrientationFromFile = 1;
  this->RasToIjkMatrix = nullptr;
  this->MeasurementFrameMatrix = vtkMatrix4x4::New();
  this->MeasurementFrameMatrix->Identity();
  this->SetDesiredCoordinateOrientationToAxial();
  this->UseNativeCoordinateOrientation = 0;
  this->FileNameSliceOffset = 0;
  this->FileNameSliceSpacing = 1;
  this->FileNameSliceCount = 0;
  this->UseNativeOrigin = true;

#ifdef VTKITK_BUILD_DICOM_SUPPORT
  this->DICOMImageIOApproach = vtkITKArchetypeImageSeriesReader::GDCM;
#endif

  this->OutputScalarType = VTK_FLOAT;
  this->NumberOfComponents = 0;
  this->UseNativeScalarType = 0;
  for (int i = 0; i < 3; i++)
    {
    this->DefaultDataSpacing[i] = 1.0;
    this->DefaultDataOrigin[i] = 0.0;
    }

  this->SeriesInstanceUIDs.resize( 0 );
  this->ContentTime.resize( 0 );
  this->TriggerTime.resize( 0 );
  this->EchoNumbers.resize( 0 );
  this->DiffusionGradientOrientation.resize( 0 );
  this->SliceLocation.resize( 0 );
  this->ImageOrientationPatient.resize( 0 );

  this->AnalyzeHeader = true;

  this->GroupingByTags = false;
  this->IsOnlyFile = false;
  this->ArchetypeIsDICOM = false;

  this->SelectedUID = -1;
  this->SelectedContentTime = -1;
  this->SelectedTriggerTime = -1;
  this->SelectedEchoNumbers = -1;
  this->SelectedDiffusion = -1;
  this->SelectedSlice = -1;
  this->SelectedOrientation = -1;

  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesReader::~vtkITKArchetypeImageSeriesReader()
{
  if (this->Archetype)
    {
    delete [] this->Archetype;
    this->Archetype = nullptr;
    }
 if (RasToIjkMatrix)
   {
   this->RasToIjkMatrix->Delete();
   this->RasToIjkMatrix = nullptr;
   }
  if (MeasurementFrameMatrix)
   {
   MeasurementFrameMatrix->Delete();
   MeasurementFrameMatrix = nullptr;
   }

}

//----------------------------------------------------------------------------
vtkMatrix4x4* vtkITKArchetypeImageSeriesReader::GetRasToIjkMatrix()
{
  this->UpdateInformation();
  return this->RasToIjkMatrix;
}

//----------------------------------------------------------------------------
vtkMatrix4x4* vtkITKArchetypeImageSeriesReader::GetMeasurementFrameMatrix()
{
  this->UpdateInformation();
  return MeasurementFrameMatrix;
}

//----------------------------------------------------------------------------
void vtkITKArchetypeImageSeriesReader::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;

  this->Superclass::PrintSelf(os,indent);

  os << indent << "Archetype: " <<
    (this->Archetype ? this->Archetype : "(none)") << "\n";

  os << indent << "FileNameSliceOffset: "
     << this->FileNameSliceOffset << "\n";
  os << indent << "FileNameSliceSpacing: "
     << this->FileNameSliceSpacing << "\n";
  os << indent << "FileNameSliceCount: "
     << this->FileNameSliceCount << "\n";

  os << indent << "OutputScalarType: "
     << vtkImageScalarTypeNameMacro(this->OutputScalarType)
     << std::endl;
  os << indent << "DefaultDataSpacing: (" << this->DefaultDataSpacing[0];
  for (idx = 1; idx < 3; ++idx)
    {
    os << ", " << this->DefaultDataSpacing[idx];
    }
  os << ")\n";

  os << indent << "DefaultDataOrigin: (" << this->DefaultDataOrigin[0];
  for (idx = 1; idx < 3; ++idx)
    {
    os << ", " << this->DefaultDataOrigin[idx];
    }
  os << ")\n";
#ifdef VTKITK_BUILD_DICOM_SUPPORT
  os << indent << "DICOMImageIOApproach: " << this->GetDICOMImageIOApproach();
#else
  os << indent << "DICOMImageIOApproach: " << "NA";
#endif
}

//----------------------------------------------------------------------------
int vtkITKArchetypeImageSeriesReader::CanReadFile(const char* filename)
{
  if (!filename)
    {
    return false;
    }

  itk::ImageIOBase::Pointer imageIO = this->GetImageIO(filename);
  if (imageIO == nullptr)
    {
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
itk::ImageIOBase::Pointer vtkITKArchetypeImageSeriesReader::GetImageIO(const char* filename)
{
  if (filename == nullptr)
    {
    return nullptr;
    }

  std::string fileNameCollapsed = itksys::SystemTools::CollapseFullPath(filename);

  // First see if the archetype exists
  if (!itksys::SystemTools::FileExists(fileNameCollapsed.c_str()))
    {
    vtkDebugMacro(<<"The filename does not exist.");
    return nullptr;
    }

  // Reduce the selection of filenames
  if (this->IsOnlyFile || this->SingleFile)
    {
    this->FileNames.resize(0);
    this->FileNames.emplace_back(filename);
    }
  else
    {
    if (!GroupingByTags)
      {
      AssembleVolumeContainingArchetype();
      }
    else
      {
      GroupFiles(SelectedUID,
        SelectedContentTime,
        SelectedTriggerTime,
        SelectedEchoNumbers,
        SelectedDiffusion,
        SelectedSlice,
        SelectedOrientation);
      }
    }

#ifdef VTKITK_BUILD_DICOM_SUPPORT
  itk::ImageIOBase::Pointer dicomIO;
#endif
  typedef itk::Image<float, 3> ImageType;
  itk::ImageIOBase::Pointer imageIO = nullptr;
  try
    {
    // Some file types require special processing
#ifdef VTKITK_BUILD_DICOM_SUPPORT
    if (this->GetDICOMImageIOApproach() == vtkITKArchetypeImageSeriesReader::DCMTK)
      {
      dicomIO = itk::DCMTKImageIO::New();
      }
    else
      {
      dicomIO = itk::GDCMImageIO::New();
      }

    // Test whether the input file is a DICOM file
    this->ArchetypeIsDICOM = dicomIO->CanReadFile(filename);
#endif

    if (this->FileNames.size() == 0)
      {
      vtkErrorMacro( "vtkITKArchetypeImageSeriesReader::ExecuteInformation: Files not found!");
      this->SetErrorCode(vtkErrorCode::FileNotFoundError);
      return nullptr;
      }
    else if (this->FileNames.size() == 1) // If there is only one file in the series, just use an image file reader
      {
      itk::ImageFileReader<ImageType>::Pointer imageReader =
        itk::ImageFileReader<ImageType>::New();
      imageReader->SetFileName(this->FileNames[0].c_str());
#ifdef VTKITK_BUILD_DICOM_SUPPORT
      if (this->ArchetypeIsDICOM)
        {
        imageReader->SetImageIO(dicomIO);
        }
#endif
      imageReader->UpdateOutputInformation();
      imageIO = imageReader->GetImageIO();
      if (imageIO.GetPointer() == nullptr)
        {
        vtkErrorMacro( "vtkITKArchetypeImageSeriesReader::ExecuteInformation: ImageIO for file " << fileNameCollapsed.c_str() << " does not exist.");
        this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
        return nullptr;
        }
      }
    else
      {
      //
      // more than one file, use series reader
      //
      itk::ImageSeriesReader<ImageType>::Pointer seriesReader =
        itk::ImageSeriesReader<ImageType>::New();
      seriesReader->SetFileNames(this->FileNames);
#ifdef VTKITK_BUILD_DICOM_SUPPORT
      if (this->ArchetypeIsDICOM)
        {
        seriesReader->SetImageIO(dicomIO);
        }
      else
#endif
        {
        itk::ImageFileReader<ImageType>::Pointer imageReader =
          itk::ImageFileReader<ImageType>::New();
        imageReader->SetFileName(this->FileNames[0].c_str());
        imageReader->UpdateOutputInformation();
        imageIO = imageReader->GetImageIO();
        seriesReader->SetImageIO(imageIO);
        }
      seriesReader->UpdateOutputInformation();
      imageIO = seriesReader->GetImageIO();
      }
    }
  catch (itk::ExceptionObject& e)
    {
    vtkDebugMacro( "vtkITKArchetypeImageSeriesReader::ExecuteInformation: Cannot open " << fileNameCollapsed.c_str() << ". "
      << "ITK exception info: error in " << e.GetLocation() << ": "<< e.GetDescription());
    this->SetErrorCode(vtkErrorCode::FileFormatError);
    return nullptr;
    }

  return imageIO;
}

//----------------------------------------------------------------------------
// This method returns the largest data that can be generated.
int vtkITKArchetypeImageSeriesReader::RequestInformation(
  vtkInformation * vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  std::vector<std::string> candidateFiles;
  std::vector<std::string> candidateSeries;
  int extent[6];
  std::string fileNameCollapsed = itksys::SystemTools::CollapseFullPath( this->Archetype);

  if ( this->SingleFile )
    {
    this->AnalyzeHeader = false;
    }

  // Since we only need origin, spacing and extents, we can use one
  // image type.
  typedef itk::Image<float,3> ImageType;
  itk::ImageRegion<3> region;

  typedef itk::ImageSource<ImageType> FilterType;
  FilterType::Pointer filter;

  // First see if the archetype exists, if it's not a pointer into memory
  if (fileNameCollapsed.find("slicer:") != std::string::npos &&
      fileNameCollapsed.find("#") != std::string::npos)
    {
    vtkDebugMacro("File " << fileNameCollapsed.c_str() << " is a pointer to the mrml scene in memory, not checking for it on disk");
    }
  else
    {
    if (!itksys::SystemTools::FileExists (fileNameCollapsed.c_str()))
      {
      vtkErrorMacro( "vtkITKArchetypeImageSeriesReader::ExecuteInformation: Archetype file " << fileNameCollapsed.c_str() << " does not exist.");
      this->SetErrorCode(vtkErrorCode::FileNotFoundError);
      return 0;
      }
    }

  this->AllFileNames.resize( 0 );

  // the code in this try/catch block uses ITK dicom code to evaluate
  // the files to see if they will be readable.  Some forms of dicom will
  // trigger exceptions which ultimately mean that the file isn't
  // going to be readable
  try
    {
    // if user already set up FileNames, we do not try to find candidate files
    if ( this->GetNumberOfFileNames() > 0 )
      {
      unsigned int nFiles = this->GetNumberOfFileNames();
      this->AllFileNames.resize( 0 );
      for (unsigned int k = 0; k < nFiles; k++)
        {
        this->AllFileNames.push_back( this->FileNames[k] );
        }
      this->FileNames.resize( 0 );

      // if this is the only file set by user
      if (nFiles == 1)
        {
        this->IsOnlyFile = true;
        }

      // if we need to analyze the header
      if ( this->AnalyzeHeader )
        {
        this->AnalyzeDicomHeaders();
        }
      }
    else
      {
#ifdef VTKITK_BUILD_DICOM_SUPPORT
      if ( this->ArchetypeIsDICOM && !this->GetSingleFile() )
        {
        typedef itk::GDCMSeriesFileNames DICOMNameGeneratorType;
        DICOMNameGeneratorType::Pointer inputImageFileGenerator = DICOMNameGeneratorType::New();
        std::string fileNamePath = itksys::SystemTools::GetFilenamePath( this->Archetype );
        if (fileNamePath == "")
          {
          fileNamePath = ".";
          }
        inputImageFileGenerator->SetDirectory( fileNamePath );

        // determine if the file is diffusion weighted MR file

        // Find the series that contains the archetype
        candidateSeries = inputImageFileGenerator->GetSeriesUIDs();

        // Find all dicom files in the directory
        for (unsigned int s = 0; s < candidateSeries.size(); s++)
          {
          std::vector<std::string> seriesFileNames;
          seriesFileNames = inputImageFileGenerator->GetFileNames( candidateSeries[s] );
          for (unsigned int f = 0; f < seriesFileNames.size(); f++)
            {
            this->AllFileNames.push_back( seriesFileNames[f] );
            }
          }
        //int nFiles = this->AllFileNames.size(); UNUSED

        // analysis dicom files and fill the Dicom Tag arrays
        if ( this->AnalyzeHeader )
          {
          this->AnalyzeDicomHeaders();
          }

        // the following for loop set up candidate files with same series number
        // that include the given Archetype;
        int found = 0;
        for (unsigned int s = 0; s < candidateSeries.size() && found == 0; s++)
          {
          candidateFiles = inputImageFileGenerator->GetFileNames(candidateSeries[s]);
          for (unsigned int f = 0; f < candidateFiles.size(); f++)
            {
            if (itksys::SystemTools::CollapseFullPath(candidateFiles[f].c_str()) ==
              fileNameCollapsed)
              {
              found = 1;
              break;
              }
            }
          }

        // do we have just one candidate file
        if ( candidateFiles.size() == 1 )
          {
          this->IsOnlyFile = true;
          }
        }
      else
#endif
      if( !this->GetSingleFile() )
        { // not dicom
        // check the dimensions of the archetype - if there
        // is more then one slice, use only the archetype
        // but if it is a single slice, try to generate a
        // series of filenames
        itk::ImageFileReader<ImageType>::Pointer imageReader =
          itk::ImageFileReader<ImageType>::New();
        imageReader->SetFileName(this->Archetype);
        imageReader->UpdateOutputInformation();
        region = imageReader->GetOutput()->GetLargestPossibleRegion();
        if ( region.GetSize()[2] > 1 )
          {
          candidateFiles.emplace_back(this->Archetype);
          this->AllFileNames.emplace_back(this->Archetype);
          this->IsOnlyFile = true;
          }
        else
          {
          // Generate filenames from the Archetype
          itk::ArchetypeSeriesFileNames::Pointer fit = itk::ArchetypeSeriesFileNames::New();
          fit->SetArchetype (this->Archetype);
          candidateFiles = fit->GetFileNames();
          this->AllFileNames.resize( candidateFiles.size() );
          for (int f = 0; f < (int)(candidateFiles.size()); f ++)
            {
            this->AllFileNames[f] = candidateFiles[f];
            }
          if ( candidateFiles.size() == 1 )
            {
            this->IsOnlyFile = true;
            }
          else if ( AnalyzeHeader )
            {
            this->AnalyzeDicomHeaders();
            }
          }
        }
      else
        {
        this->AllFileNames.emplace_back(this->Archetype);
        this->IsOnlyFile = true;
        }
      }
    }
  catch (itk::ExceptionObject& e)
    {
    vtkErrorMacro( "vtkITKArchetypeImageSeriesReader::ExecuteInformation: Cannot open " << fileNameCollapsed.c_str() << ". "
      << "ITK exception info: error in " << e.GetLocation() << ": "<< e.GetDescription());
    this->SetErrorCode(vtkErrorCode::FileFormatError);
    return 0;
    }

  // figure out the index of Archetype in AllFileNames
  // Collapsing of path is necessary to normalize filenames (path separator, capitalization of drive
  // letter and path) for comparison.
  std::string archetypeCollapsed = itksys::SystemTools::CollapseFullPath(this->Archetype);
  for (unsigned int k = 0; k < this->AllFileNames.size(); k++)
    {
    if (itksys::SystemTools::CollapseFullPath(this->AllFileNames[k]) == archetypeCollapsed)
      {
      this->IndexArchetype = k;
      break;
      }
    }

  // Reduce the selection of filenames
  if ( this->IsOnlyFile || this->SingleFile )
    {
    this->FileNames.resize( 0 );
    this->FileNames.emplace_back(this->Archetype);
    }
  else
    {
    if ( !GroupingByTags )
      {
      AssembleVolumeContainingArchetype();
      }
    else
      {
    GroupFiles( SelectedUID,
                SelectedContentTime,
                SelectedTriggerTime,
                SelectedEchoNumbers,
                SelectedDiffusion,
                SelectedSlice,
                SelectedOrientation );
      }
    }

  if (RasToIjkMatrix)
    {
    this->RasToIjkMatrix->Delete();
    }
  this->RasToIjkMatrix = vtkMatrix4x4::New();

  vtkMatrix4x4* IjkToLpsMatrix = vtkMatrix4x4::New();

  this->RasToIjkMatrix->Identity();
  IjkToLpsMatrix->Identity();

  double spacing[3];
  double origin[3];

  itk::ImageIOBase::Pointer imageIO = this->GetImageIO(this->Archetype);
  try
    {
    if (this->FileNames.size() == 0)
      {
      vtkErrorMacro( "vtkITKArchetypeImageSeriesReader::ExecuteInformation: Failed to read file series");
      this->SetErrorCode(vtkErrorCode::FileNotFoundError);
      return 0;
      }
    else if (this->FileNames.size() == 1) // If there is only one file in the series, just use an image file reader
      {
      itk::OrientImageFilter<ImageType,ImageType>::Pointer orient =
        itk::OrientImageFilter<ImageType,ImageType>::New();
      itk::ImageFileReader<ImageType>::Pointer imageReader =
        itk::ImageFileReader<ImageType>::New();
      imageReader->SetFileName(this->FileNames[0].c_str());
      imageReader->SetImageIO(imageIO);

      if (this->UseNativeCoordinateOrientation)
        {
        imageReader->UpdateOutputInformation();
        filter = imageReader;
        }
      else
        {
        orient->SetInput(imageReader->GetOutput());
        orient->UseImageDirectionOn();
        orient->SetDesiredCoordinateOrientation(this->DesiredCoordinateOrientation);
        orient->UpdateOutputInformation();
        filter = orient;
        }
      for (int i = 0; i < 3; i++)
        {
        spacing[i] = filter->GetOutput()->GetSpacing()[i];
        origin[i] = filter->GetOutput()->GetOrigin()[i];

        // Get IJK to RAS direction vector
        for ( unsigned int j=0; j < filter->GetOutput()->GetImageDimension (); j++ )
          {
          IjkToLpsMatrix->SetElement(j, i, spacing[i]*filter->GetOutput()->GetDirection()[j][i]);
          }
        }
      region = filter->GetOutput()->GetLargestPossibleRegion();
      extent[0] = region.GetIndex()[0];
      extent[1] = region.GetIndex()[0] + region.GetSize()[0] - 1;
      extent[2] = region.GetIndex()[1];
      extent[3] = region.GetIndex()[1] + region.GetSize()[1] - 1;
      extent[4] = region.GetIndex()[2];
      extent[5] = region.GetIndex()[2] + region.GetSize()[2] - 1;

      typedef std::vector<std::vector<double> >    DoubleVectorType;
      typedef itk::MetaDataObject<DoubleVectorType>     MetaDataDoubleVectorType;
      const itk::MetaDataDictionary &        dictionary = imageReader->GetMetaDataDictionary();
      itk::MetaDataDictionary::ConstIterator itr = dictionary.Begin();
      itk::MetaDataDictionary::ConstIterator end = dictionary.End();
      while( itr != end )
        {
        itk::MetaDataObjectBase::Pointer  entry = itr->second;
        MetaDataDoubleVectorType::Pointer entryvalue1
          = dynamic_cast<MetaDataDoubleVectorType *>( entry.GetPointer() );
        if( entryvalue1 )
          {
          int pos = itr->first.find( "NRRD_measurement frame" );
          if( pos != -1 )
            {
            DoubleVectorType tagvalue = entryvalue1->GetMetaDataObjectValue();
            for( int i = 0; i < 3; i++ )
              {
              for( int j = 0; j < 3; j++ )
                {
                this->MeasurementFrameMatrix->SetElement(i,j, tagvalue.at( j ).at( i ));
                }
              }
            }
          }
        ++itr;
        }

      imageIO = imageReader->GetImageIO();
      if (imageIO.GetPointer() == nullptr)
        {
          vtkErrorMacro( "vtkITKArchetypeImageSeriesReader::ExecuteInformation: ImageIO for file " << fileNameCollapsed.c_str() << " does not exist.");
          this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
          return 0;
        }
      }
    else
      {
      //
      // more than one file, use series reader
      //
      itk::OrientImageFilter<ImageType,ImageType>::Pointer orient =
        itk::OrientImageFilter<ImageType,ImageType>::New();
      itk::ImageSeriesReader<ImageType>::Pointer seriesReader =
        itk::ImageSeriesReader<ImageType>::New();
      seriesReader->SetFileNames(this->FileNames);
      seriesReader->SetImageIO(imageIO);
      if (this->UseNativeCoordinateOrientation)
        {
        filter = seriesReader;
        }
      else
        {
        orient->SetInput(seriesReader->GetOutput());
        orient->UseImageDirectionOn();
        orient->SetDesiredCoordinateOrientation(this->DesiredCoordinateOrientation);
        filter = orient;
        }
      filter->UpdateOutputInformation();
      for (int i = 0; i < 3; i++)
        {
        spacing[i] = filter->GetOutput()->GetSpacing()[i];
        origin[i] = filter->GetOutput()->GetOrigin()[i];
        // Get IJK to RAS direction vector
        for ( unsigned int j=0; j < filter->GetOutput()->GetImageDimension (); j++ )
          {
          IjkToLpsMatrix->SetElement(j, i, spacing[i]*filter->GetOutput()->GetDirection()[j][i]);
          }
        }

      region = filter->GetOutput()->GetLargestPossibleRegion();
      extent[0] = region.GetIndex()[0];
      extent[1] = region.GetIndex()[0] + region.GetSize()[0] - 1;
      extent[2] = region.GetIndex()[1];
      extent[3] = region.GetIndex()[1] + region.GetSize()[1] - 1;
      extent[4] = region.GetIndex()[2];
      extent[5] = region.GetIndex()[2] + region.GetSize()[2] - 1;
      imageIO = seriesReader->GetImageIO();

      }
    }
  catch (itk::ExceptionObject& e)
    {
    IjkToLpsMatrix->Delete();
    vtkErrorMacro( "vtkITKArchetypeImageSeriesReader::ExecuteInformation: Cannot open " << fileNameCollapsed.c_str() << ". "
      << "ITK exception info: error in " << e.GetLocation() << ": "<< e.GetDescription());
    this->SetErrorCode(vtkErrorCode::FileFormatError);
    return 0;
    }
  // Transform from LPS to RAS
  vtkMatrix4x4* LpsToRasMatrix = vtkMatrix4x4::New();
  LpsToRasMatrix->Identity();
  LpsToRasMatrix->SetElement(0,0,-1);
  LpsToRasMatrix->SetElement(1,1,-1);

  vtkMatrix4x4* LPSMeasurementFrameMatrix = vtkMatrix4x4::New();
  LPSMeasurementFrameMatrix->DeepCopy(this->MeasurementFrameMatrix);

  vtkMatrix4x4::Multiply4x4(LpsToRasMatrix, IjkToLpsMatrix, this->RasToIjkMatrix);
  vtkMatrix4x4::Multiply4x4(LpsToRasMatrix, LPSMeasurementFrameMatrix, this->MeasurementFrameMatrix);
  LpsToRasMatrix->Delete();
  LPSMeasurementFrameMatrix->Delete();

  // If it looks like the pipeline did not provide the spacing and
  // origin, modify the spacing and origin with the defaults
  for (int j = 0; j < 3; j++)
    {
    if (spacing[j] == 1.0)
      {
      spacing[j] = this->DefaultDataSpacing[j];
      }
    if (origin[j] == 0.0)
      {
      origin[j] = this->DefaultDataOrigin[j];
      }
    }

  origin[0] *= -1;   // L -> R
  origin[1] *= -1;   // P -> A

  if (this->UseNativeOrigin)
    {
    for (int j = 0; j < 3; j++)
      {
      this->RasToIjkMatrix->SetElement(j, 3, origin[j]);
      }
    this->RasToIjkMatrix->Invert();
    }
  else
    {
    this->RasToIjkMatrix->Invert();
    for (int j = 0; j < 3; j++)
      {
      this->RasToIjkMatrix->SetElement(j, 3, (extent[2*j+1] - extent[2*j])/2.0);
      }
    }

  outInfo->Set(vtkDataObject::SPACING(), spacing, 3);
  outInfo->Set(vtkDataObject::ORIGIN(), origin, 3);

  // TODO: this may corrupt the slice spacig in multiframe DICOM files,
  // so probably it would be better to remove it.
  this->RasToIjkMatrix->SetElement(3,3,1.0);

  IjkToLpsMatrix->Delete();

  if ( !this->GetUseOrientationFromFile() )
    {
    this->RasToIjkMatrix->Identity();
    for ( unsigned int j=0; j < 3; j++ )
      {
      this->RasToIjkMatrix->SetElement(j, j, 1.0/spacing[j]);
      }
    }

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent, 6);
  int scalarType = this->OutputScalarType;
  if (this->UseNativeScalarType)
    {
    // If there is only one file in the series
    if (this->FileNames.size() == 1)
      {
      if (imageIO.GetPointer() == nullptr)
        {
        scalarType = VTK_SHORT; // TODO - figure out why multi-file series doesn't have an imageIO
        }
      else if (imageIO->GetComponentType() == itk::ImageIOBase::UCHAR)
        {
        scalarType = VTK_UNSIGNED_CHAR;
        }
      else if (imageIO->GetComponentType() == itk::ImageIOBase::CHAR)
        {
        scalarType = VTK_CHAR;
        }
      else if (imageIO->GetComponentType() == itk::ImageIOBase::USHORT)
        {
        scalarType = VTK_UNSIGNED_SHORT;
        }
      else if (imageIO->GetComponentType() == itk::ImageIOBase::SHORT)
        {
        scalarType = VTK_SHORT;
        }
      else if (imageIO->GetComponentType() == itk::ImageIOBase::UINT)
        {
        scalarType = VTK_UNSIGNED_INT;
        }
      else if (imageIO->GetComponentType() == itk::ImageIOBase::INT)
        {
        scalarType = VTK_INT;
        }
      else if (imageIO->GetComponentType() == itk::ImageIOBase::ULONG)
        {
        scalarType = VTK_UNSIGNED_LONG;
        }
      else if (imageIO->GetComponentType() == itk::ImageIOBase::LONG)
        {
        scalarType = VTK_LONG;
        }
      else if (imageIO->GetComponentType() == itk::ImageIOBase::FLOAT)
        {
        scalarType = VTK_FLOAT;
        }
      else if (imageIO->GetComponentType() == itk::ImageIOBase::DOUBLE)
        {
        scalarType = VTK_DOUBLE;
        }
      }
    else
      {
      double min = 0, max = 0;

      for( unsigned int f = 0; f < this->FileNames.size(); f++ )
        {
        imageIO->SetFileName( this->FileNames[f] );
        imageIO->ReadImageInformation();

        if ( imageIO->GetComponentType() == itk::ImageIOBase::UCHAR )
          {
          min = std::numeric_limits<uint8_t>::min() < min ? std::numeric_limits<uint8_t>::min() : min;
          max = std::numeric_limits<uint8_t>::max() > max ? std::numeric_limits<uint8_t>::max() : max;
          }
        if ( imageIO->GetComponentType() == itk::ImageIOBase::CHAR )
          {
          min = std::numeric_limits<int8_t>::min() < min ? std::numeric_limits<int8_t>::min() : min;
          max = std::numeric_limits<int8_t>::max() > max ? std::numeric_limits<int8_t>::max() : max;
          }
        if ( imageIO->GetComponentType() == itk::ImageIOBase::USHORT )
          {
          min = std::numeric_limits<uint16_t>::min() < min ? std::numeric_limits<uint16_t>::min() : min;
          max = std::numeric_limits<uint16_t>::max() > max ? std::numeric_limits<uint16_t>::max() : max;
          }
        if ( imageIO->GetComponentType() == itk::ImageIOBase::SHORT )
          {
          min = std::numeric_limits<int16_t>::min() < min ? std::numeric_limits<int16_t>::min() : min;
          max = std::numeric_limits<int16_t>::max() > max ? std::numeric_limits<int16_t>::max() : max;
          }
        if ( imageIO->GetComponentType() == itk::ImageIOBase::UINT )
          {
          min = std::numeric_limits<uint32_t>::min() < min ? std::numeric_limits<uint32_t>::min() : min;
          max = std::numeric_limits<uint32_t>::max() > max ? std::numeric_limits<uint32_t>::max() : max;
          }
        if ( imageIO->GetComponentType() == itk::ImageIOBase::INT )
          {
          min = static_cast<double>(std::numeric_limits<int32_t>::min() < min ? std::numeric_limits<int32_t>::min() : min);
          max = static_cast<double>(std::numeric_limits<int32_t>::max() > max ? std::numeric_limits<int32_t>::max() : max);
          }
        if ( imageIO->GetComponentType() == itk::ImageIOBase::ULONG )
          { // note that on windows ULONG is only 32 bit
          min = static_cast<double>(std::numeric_limits<uint64_t>::min() < min ? std::numeric_limits<uint64_t>::min() : min);
          max = static_cast<double>(std::numeric_limits<uint64_t>::max() > max ? std::numeric_limits<uint64_t>::max() : max);
          }
        if ( imageIO->GetComponentType() == itk::ImageIOBase::LONG )
          { // note that on windows LONG is only 32 bit
          min = static_cast<double>(std::numeric_limits<int64_t>::min() < min ? std::numeric_limits<int64_t>::min() : min);
          max = static_cast<double>(std::numeric_limits<int64_t>::max() > max ? std::numeric_limits<int64_t>::max() : max);
          }
        if ( imageIO->GetComponentType() == itk::ImageIOBase::FLOAT )
          {
          // use -max() as min() for both float and double as temp workaround
          // should switch to lowest() function in C++ 11 in the future
          min = -std::numeric_limits<float>::max() < min ? -std::numeric_limits<float>::max() : min;
          max = std::numeric_limits<float>::max() > max ? std::numeric_limits<float>::max() : max;
          }
        if ( imageIO->GetComponentType() == itk::ImageIOBase::DOUBLE )
          {
          min = -std::numeric_limits<double>::max() < min ? -std::numeric_limits<double>::max() : min;
          max = std::numeric_limits<double>::max() > max ? std::numeric_limits<double>::max() : max;
          }
        }
      assert( min <= max );
      if( min >= 0 ) // unsigned
        {
        if( max <= std::numeric_limits<uint8_t>::max() )
          {
          scalarType = VTK_UNSIGNED_CHAR;
          }
        else if( max <= std::numeric_limits<uint16_t>::max() )
          {
          scalarType = VTK_UNSIGNED_SHORT;
          }
        else if( max <= std::numeric_limits<uint32_t>::max() )
          {
          scalarType = VTK_UNSIGNED_INT;
          }
        else if( max <= static_cast<double>(std::numeric_limits<uint64_t>::max()) )
          {
          scalarType = VTK_UNSIGNED_LONG;
          }
        else if( max <= std::numeric_limits<float>::max() )
          {
          scalarType = VTK_FLOAT;
          }
        else if( max <= std::numeric_limits<double>::max() )
          {
          scalarType = VTK_DOUBLE;
          }
        else
          {
          assert(0);
          }
        }
      else
        {
        if( max <= std::numeric_limits<int8_t>::max()
          && min >= std::numeric_limits<int8_t>::min() )
          {
          scalarType = VTK_CHAR;
          }
        else if( max <= std::numeric_limits<int16_t>::max()
          && min >= std::numeric_limits<int16_t>::min() )
          {
          scalarType = VTK_SHORT;
          }
        else if( max <= std::numeric_limits<int32_t>::max()
          && min >= std::numeric_limits<int32_t>::min() )
          {
          scalarType = VTK_INT;
          }
        else if( max <= std::numeric_limits<int64_t>::max()
          && min >= std::numeric_limits<int64_t>::min() )
          {
          scalarType = VTK_LONG;
          }
        else if ( max <= std::numeric_limits<float>::max()
          && min >= -std::numeric_limits<float>::max() )
          {
          scalarType = VTK_FLOAT;
          }
        else if( max <= std::numeric_limits<double>::max()
          && min >= -std::numeric_limits<double>::max() )
          {
          scalarType = VTK_DOUBLE;
          }
        else
          {
          assert(0);
          }
        }
      }
    }

  int numberOfComponents = 1;
  if (imageIO.GetPointer() != nullptr)
    {
    numberOfComponents = imageIO->GetNumberOfComponents();
    }
  this->SetNumberOfComponents(numberOfComponents);
  this->SetOutputScalarType(scalarType);


  vtkDataObject::SetPointDataActiveScalarInfo(outInfo,
                                              scalarType,
                                              numberOfComponents);

  // Copy the MetaDataDictionary from the ITK layer to the VTK layer
  if (imageIO.GetPointer() != nullptr)
    {
    this->Dictionary = imageIO->GetMetaDataDictionary();
    }
  else
    {
    this->Dictionary = itk::MetaDataDictionary();
    }
  ParseDictionary();

  // Store the meta data scalar range if it exists
  std::string range_keys[2];
  this->GetScalarRangeMetaDataKeys(imageIO, range_keys);
  std::string min_str;
  std::string max_str;
  bool hasMin = itk::ExposeMetaData<std::string>(this->Dictionary, range_keys[0], min_str);
  bool hasMax = itk::ExposeMetaData<std::string>(this->Dictionary, range_keys[1], max_str);
  if (hasMin && hasMax)
    {
    this->MetaDataScalarRangeMinima.clear();
    this->MetaDataScalarRangeMaxima.clear();
    double value = 0.0;
    std::istringstream min_sstr(min_str);
    while (min_sstr >> value)
      {
      this->MetaDataScalarRangeMinima.push_back(value);
      }
    std::istringstream max_sstr(max_str);
    while (max_sstr >> value)
      {
      this->MetaDataScalarRangeMaxima.push_back(value);
      }
    }

  return 1;
}

//----------------------------------------------------------------------------
void vtkITKArchetypeImageSeriesReader::GetScalarRangeMetaDataKeys(itk::ImageIOBase::Pointer imageIO,
                                                                  std::string range_keys[2])
{
  if (dynamic_cast<itk::NrrdImageIO*>(imageIO.GetPointer()))
    {
    // http://teem.sourceforge.net/nrrd/format.html#min
    range_keys[0] = "min";
    range_keys[1] = "max";
    }
  else if (dynamic_cast<itk::NiftiImageIO*>(imageIO.GetPointer()))
    {
    // https://nifti.nimh.nih.gov/nifti-1/documentation/nifti1fields/nifti1fields_pages/cal_maxmin.html
    range_keys[0] = "cal_min";
    range_keys[1] = "cal_max";
    }
  else if (dynamic_cast<itk::MetaImageIO*>(imageIO.GetPointer()))
    {
    // https://itk.org/Wiki/MetaIO/Documentation#Tags_Added_by_MetaImage
    range_keys[0] = "ElementMin";
    range_keys[1] = "ElementMax";
    }
}

//----------------------------------------------------------------------------
void vtkITKArchetypeImageSeriesReader::SetMetaDataScalarRangeToPointDataInfo( vtkImageData* data )
{
  if (data == nullptr)
    {
    vtkWarningMacro("No image data specified, can't set scalar range information.");
    return;
    }

  // If no metadata scalar range found
  if (this->MetaDataScalarRangeMinima.empty() ||
     this->MetaDataScalarRangeMaxima.empty())
    {
    return;
    }

  // If metadata scalar range not consistent
  unsigned int nbrOfComponents = data->GetNumberOfScalarComponents();
  if (nbrOfComponents != this->MetaDataScalarRangeMinima.size() ||
      nbrOfComponents != this->MetaDataScalarRangeMaxima.size())
    {
    vtkWarningMacro("Mismatch between image data and meta data number of scalar components, ignoring metada scalar range.");
    return;
    }

  // Check that scalar info exists
  vtkDataArray* scalars = data->GetPointData()->GetScalars();
  vtkInformation* scalarInfo = scalars ? scalars->GetInformation() : nullptr;
  if (scalarInfo == nullptr)
    {
    return;
    }

  // Write meta data scalar range in scalars information
  vtkNew<vtkInformationVector> rangeInfoVector;
  rangeInfoVector->SetNumberOfInformationObjects(nbrOfComponents);
  double scalarRange[2];
  bool scalarRangeIsValid = false; // all values should not be equal to 0
  for (unsigned int i = 0; i < nbrOfComponents; ++i)
    {
    scalarRange[0] = this->MetaDataScalarRangeMinima[i];
    scalarRange[1] = this->MetaDataScalarRangeMaxima[i];
    rangeInfoVector->GetInformationObject(i)->Set(vtkDataArray::COMPONENT_RANGE(), scalarRange, 2);
    if (scalarRange[0] != 0.0 || scalarRange[1] != 0.0)
      {
      scalarRangeIsValid = true;
      }
    }

  if (scalarRangeIsValid)
    {
    scalarInfo->Set(vtkAbstractArray::PER_COMPONENT(), rangeInfoVector.Get());
    }
}

//----------------------------------------------------------------------------
void vtkITKArchetypeImageSeriesReader::AssembleNthVolume ( int n )
{
  this->FileNames.resize( 0 );
  // int nFiles = this->AllFileNames.size(); UNUSED

  unsigned int nSlices = this->GetNumberOfSliceLocation();

  for (unsigned int k = 0; k < nSlices; k++)
    {
    const char* name = GetNthFileName( 0, -1, -1, -1, 0, k, 0, n );
    if (name == nullptr)
      {
      continue;
      }
    std::string nameInString (name);
    this->FileNames.push_back(nameInString);
    }
}

//----------------------------------------------------------------------------
unsigned int vtkITKArchetypeImageSeriesReader::GetNumberOfFileNames()
{
  return this->FileNames.size();
}

//----------------------------------------------------------------------------
const std::vector<std::string>& vtkITKArchetypeImageSeriesReader::GetFileNames()
{
  return this->FileNames;
}

//----------------------------------------------------------------------------
const char* vtkITKArchetypeImageSeriesReader::GetNthFileName ( int idxSeriesInstanceUID,
                                                               int idxContentTime,
                                                               int idxTriggerTime,
                                                               int idxEchoNumbers,
                                                               int idxDiffusionGradientOrientation,
                                                               int idxSliceLocation,
                                                               int idxImageOrientationPatient,
                                                               int n )
{
  int nFiles = this->AllFileNames.size();
  int count = 0;
  std::string FirstName;
  for (int k = 0; k < nFiles; k++)
    {
    if ( (this->IndexSeriesInstanceUIDs[k] != idxSeriesInstanceUID && this->IndexSeriesInstanceUIDs[k] >= 0 && idxSeriesInstanceUID >= 0) ||
         (this->IndexContentTime[k] != idxContentTime && this->IndexContentTime[k] >= 0 && idxContentTime >= 0) ||
         (this->IndexTriggerTime[k] != idxTriggerTime && this->IndexTriggerTime[k] >= 0 && idxTriggerTime >= 0) ||
         (this->IndexEchoNumbers[k] != idxEchoNumbers && this->IndexEchoNumbers[k] >= 0 && idxEchoNumbers >= 0) ||
         (this->IndexDiffusionGradientOrientation[k] != idxDiffusionGradientOrientation &&
           this->IndexDiffusionGradientOrientation[k] >= 0 && idxDiffusionGradientOrientation >= 0) ||
         (this->IndexSliceLocation[k] != idxSliceLocation && this->IndexSliceLocation[k] >= 0 && idxSliceLocation >= 0) ||
         (this->IndexImageOrientationPatient[k] != idxImageOrientationPatient &&
           this->IndexImageOrientationPatient[k] >= 0 && idxImageOrientationPatient >= 0) )
      {
      continue;
      }
    else
      {
      if (count == n)
        {
        return this->AllFileNames[k].c_str();
        }
      else if( count == 0 )
        {
        FirstName = this->AllFileNames[k];
        count ++;
        }
      else
        {
        count ++;
        }
      }
    }
  return nullptr;
}

//----------------------------------------------------------------------------
void vtkITKArchetypeImageSeriesReader::GroupFiles ( int idxSeriesInstanceUID,
                                                    int idxContentTime,
                                                    int idxTriggerTime,
                                                    int idxEchoNumbers,
                                                    int idxDiffusionGradientOrientation,
                                                    int idxSliceLocation,
                                                    int idxImageOrientationPatient )
{
  this->FileNames.resize( 0 );
  int nFiles = this->AllFileNames.size();
  for (int k = 0; k < nFiles; k++)
    {
    if ((this->IndexSeriesInstanceUIDs[k] != idxSeriesInstanceUID && this->IndexSeriesInstanceUIDs[k] >= 0 && idxSeriesInstanceUID >= 0) ||
      (this->IndexContentTime[k] != idxContentTime && this->IndexContentTime[k] >= 0 && idxContentTime >= 0) ||
      (this->IndexTriggerTime[k] != idxTriggerTime && this->IndexTriggerTime[k] >= 0 && idxTriggerTime >= 0) ||
      (this->IndexEchoNumbers[k] != idxEchoNumbers && this->IndexEchoNumbers[k] >= 0 && idxEchoNumbers >= 0) ||
      (this->IndexDiffusionGradientOrientation[k] != idxDiffusionGradientOrientation &&
        this->IndexDiffusionGradientOrientation[k] >= 0 && idxDiffusionGradientOrientation >= 0) ||
      (this->IndexSliceLocation[k] != idxSliceLocation && this->IndexSliceLocation[k] >= 0 && idxSliceLocation >= 0) ||
      (this->IndexImageOrientationPatient[k] != idxImageOrientationPatient &&
        this->IndexImageOrientationPatient[k] >= 0 && idxImageOrientationPatient >= 0))
      {
      continue;
      }
    else
      {
      this->FileNames.push_back( this->AllFileNames[k] );
      }
    }
  return;
}

std::string vtkITKArchetypeImageSeriesReader::GetMetaDataWithoutSpaces(const itk::MetaDataDictionary &dict, const std::string& tag)
{
  std::string tagValue;
  itk::ExposeMetaData<std::string>(dict, tag, tagValue);
  tagValue.erase(std::remove_if(tagValue.begin(), tagValue.end(), isspace), tagValue.end());
  return tagValue;
}

//----------------------------------------------------------------------------
void vtkITKArchetypeImageSeriesReader::AnalyzeDicomHeaders()
{
#ifdef VTKITK_BUILD_DICOM_SUPPORT
  itk::TimeProbe AnalyzeTime;
  AnalyzeTime.Start();

  int nFiles = this->AllFileNames.size();
  typedef itk::Image<float,3> ImageType;

  this->IndexSeriesInstanceUIDs.resize( nFiles );
  this->IndexContentTime.resize( nFiles );
  this->IndexTriggerTime.resize( nFiles );
  this->IndexEchoNumbers.resize( nFiles );
  this->IndexDiffusionGradientOrientation.resize( nFiles );
  this->IndexSliceLocation.resize( nFiles );
  this->IndexImageOrientationPatient.resize( nFiles );
  this->IndexImagePositionPatient.resize( nFiles );

  this->SeriesInstanceUIDs.resize( 0 );
  this->ContentTime.resize( 0 );
  this->TriggerTime.resize( 0 );
  this->EchoNumbers.resize( 0 );
  this->DiffusionGradientOrientation.resize( 0 );
  this->SliceLocation.resize( 0 );
  this->ImageOrientationPatient.resize( 0 );
  this->ImagePositionPatient.resize( 0 );


  itk::GDCMImageIO::Pointer gdcmIO = itk::GDCMImageIO::New();
  if ( !gdcmIO->CanReadFile(this->Archetype) )
    {
    itk::ImageIOBase::Pointer lastImageIO;
    for (int f = 0; f < nFiles; f++)
      {
      itk::ImageFileReader<ImageType>::Pointer imageReader =
        itk::ImageFileReader<ImageType>::New();
      const std::string& fileName = this->AllFileNames[f];
      imageReader->SetFileName( fileName );
      // Try first to reuse the same imageIO for each file. If it fails, then use
      // the default imageIO
      if (lastImageIO && (lastImageIO->CanReadFile(fileName.c_str())))
        {
        imageReader->SetImageIO(lastImageIO);
        }
      else
        {
        imageReader->UpdateOutputInformation();
        lastImageIO = imageReader->GetImageIO();
        }

      // Don't read the image header if it is a 2D file type.
      std::string IOType = imageReader->GetImageIO()->GetNameOfClass();
      bool ioHas3DInformation =
        IOType.find("BPMImageIO") != std::string::npos &&
        IOType.find("JPEGImageIO") != std::string::npos &&
        IOType.find("PNGImageIO") != std::string::npos &&
        IOType.find("TIFFImageIO") != std::string::npos &&
        IOType.find("RawImageIO") != std::string::npos;
      if (ioHas3DInformation)
        {
        imageReader->UpdateOutputInformation();
        lastImageIO = imageReader->GetImageIO();
        }

      // insert series
      int idx = this->InsertSeriesInstanceUIDs( "Non-Dicom Series" );
      this->IndexSeriesInstanceUIDs[f] = idx;

      // for now, assume ContentTime, TriggerTime, and DiffusionGradientOrientation do not exist
      this->IndexContentTime[f] = -1;
      this->IndexTriggerTime[f] = -1;
      this->IndexEchoNumbers[f] = -1;
      this->IndexDiffusionGradientOrientation[f] = -1;

      // Slice Location
      if (ioHas3DInformation)
        {
        ImageType::PointType origin = imageReader->GetOutput()->GetOrigin();
        idx = this->InsertSliceLocation( origin[2] );
        }
      else
        {
        idx = this->InsertNextSliceLocation();
        }
      this->IndexSliceLocation[f] = idx;

      // Orientation
      float sliceOrientation[6] = {1., 0., 0.,
                                   0., 1., 0.};
      if (ioHas3DInformation)
        {
        ImageType::DirectionType orientation =
          imageReader->GetOutput()->GetDirection();
        for (int k = 0; k < 3; k++)
          {
          sliceOrientation[k] = orientation[0][k];
          sliceOrientation[k+3] = orientation[1][k];
          }
        }
      idx = this->InsertImageOrientationPatient( sliceOrientation );
      this->IndexImageOrientationPatient[f] = idx;
      }
    return;
    }

  // if Archetype is a Dicom File
  gdcmIO->SetFileName( this->Archetype );
  for (int f = 0; f < nFiles; f++)
    {
    gdcmIO->SetFileName( this->AllFileNames[f] );
    gdcmIO->ReadImageInformation();
    itk::MetaDataDictionary &dict = gdcmIO->GetMetaDataDictionary();
    std::string tagValue;

    // Use vtkITKArchetypeImageSeriesReader::GetMetaDataWithoutSpaces to remove extra spaces
    // from the DICOM tag, because extra spaces were found in some DICOM file before/after the
    // multi-value separator backslashes.

    // series instance UID
    tagValue = vtkITKArchetypeImageSeriesReader::GetMetaDataWithoutSpaces(dict, "0020|000e");
    if (!tagValue.empty())
      {
      int idx = InsertSeriesInstanceUIDs( tagValue.c_str() );
      this->IndexSeriesInstanceUIDs[f] = idx;
      }
    else
      {
      this->IndexSeriesInstanceUIDs[f] = -1;
      }

    // content time
    tagValue = vtkITKArchetypeImageSeriesReader::GetMetaDataWithoutSpaces(dict, "0008|0033");
    if (!tagValue.empty())
      {
      int idx = InsertContentTime( tagValue.c_str() );
      this->IndexContentTime[f] = idx;
      }
    else
      {
      this->IndexContentTime[f] = -1;
      }

    // trigger time
    tagValue = vtkITKArchetypeImageSeriesReader::GetMetaDataWithoutSpaces(dict, "0018|1060");
    if (!tagValue.empty())
      {
      int idx = InsertTriggerTime( tagValue.c_str() );
      this->IndexTriggerTime[f] = idx;
      }
    else
      {
      this->IndexTriggerTime[f] = -1;
      }

    // echo numbers
    tagValue = vtkITKArchetypeImageSeriesReader::GetMetaDataWithoutSpaces(dict, "0018|0086");
    if (!tagValue.empty())
      {
      int idx = InsertEchoNumbers( tagValue.c_str() );
      this->IndexEchoNumbers[f] = idx;
      }
    else
      {
      this->IndexEchoNumbers[f] = -1;
      }

    // diffision gradient orientation
    tagValue = vtkITKArchetypeImageSeriesReader::GetMetaDataWithoutSpaces(dict, "0010|9089");
    if (!tagValue.empty())
      {
      float a[3] = { -1 };
      sscanf( tagValue.c_str(), "%f\\%f\\%f", a, a+1, a+2 );
      int idx = InsertDiffusionGradientOrientation( a );
      this->IndexDiffusionGradientOrientation[f] = idx;
      }
    else
      {
      this->IndexDiffusionGradientOrientation[f] = -1;
      }

    // slice location
    tagValue = vtkITKArchetypeImageSeriesReader::GetMetaDataWithoutSpaces(dict, "0020|1041");
    if (!tagValue.empty())
      {
      float a = -1;
      sscanf( tagValue.c_str(), "%f", &a );
      int idx = InsertSliceLocation( a );
      this->IndexSliceLocation[f] = idx;
      }
    else
      {
      this->IndexSliceLocation[f] = -1;
      }

    // image orientation patient
    tagValue = vtkITKArchetypeImageSeriesReader::GetMetaDataWithoutSpaces(dict, "0020|0037");
    if (!tagValue.empty())
      {
      float a[6] = { -1 };
      sscanf( tagValue.c_str(), "%f\\%f\\%f\\%f\\%f\\%f", a, a+1, a+2, a+3, a+4, a+5 );
      int idx = InsertImageOrientationPatient( a );
      this->IndexImageOrientationPatient[f] = idx;
      }
    else
      {
      this->IndexImageOrientationPatient[f] = -1;
      }
    // image position patient
    tagValue = vtkITKArchetypeImageSeriesReader::GetMetaDataWithoutSpaces(dict, "0020|0032");
    if (!tagValue.empty())
      {
      float a[3] = { -1 };
      sscanf( tagValue.c_str(), "%f\\%f\\%f", a, a+1, a+2 );
      int idx = InsertImagePositionPatient( a );
      this->IndexImagePositionPatient[f] = idx;
      }
    else
      {
      this->IndexImagePositionPatient[f] = -1;
      }
    }

  AnalyzeTime.Stop();

  // double timeelapsed = AnalyzeTime.GetMean(); UNUSED
  AnalyzeHeader = false;
#endif
}

//----------------------------------------------------------------------------
const itk::MetaDataDictionary&
vtkITKArchetypeImageSeriesReader
::GetMetaDataDictionary() const
{
  return this->Dictionary;
}

//----------------------------------------------------------------------------
void vtkITKArchetypeImageSeriesReader::ParseDictionary()
{
  int nItems = this->Dictionary.GetKeys().size();
  this->Tags.resize(0);
  this->TagValues.resize(0);

  if (nItems == 0)
    {
    return;
    }

  std::vector<std::string> keys = this->Dictionary.GetKeys();
  for (int k = 0; k < nItems; k++)
    {
    this->Tags.push_back( keys[k] );
    std::string tagvalue;
    itk::ExposeMetaData<std::string>(this->Dictionary, keys[k], tagvalue);
    this->TagValues.push_back( tagvalue );
    }
}

//----------------------------------------------------------------------------
unsigned int vtkITKArchetypeImageSeriesReader::GetNumberOfItemsInDictionary()
{
  return this->Tags.size();
}

//----------------------------------------------------------------------------
bool vtkITKArchetypeImageSeriesReader::HasKey( char* tag )
{
  return this->Dictionary.HasKey( tag );
}

//----------------------------------------------------------------------------
const char* vtkITKArchetypeImageSeriesReader::GetNthKey( unsigned int n )
{
  if (n >= this->Tags.size())
    {
    return nullptr;
    }
  return this->Tags[n].c_str();
}

//----------------------------------------------------------------------------
const char* vtkITKArchetypeImageSeriesReader::GetNthValue( unsigned int n )
{
  if (n >= this->TagValues.size())
    {
    return nullptr;
    }
  return this->TagValues[n].c_str();
}

//----------------------------------------------------------------------------
const char* vtkITKArchetypeImageSeriesReader::GetTagValue( char* tag )
{
  std::string tagstr (tag);
  for (unsigned int k = 0; k < this->Tags.size(); k++)
    {
    if (this->Tags[k] == tagstr)
      {
      return this->TagValues[k].c_str();
      }
    }
  return nullptr;
}

//----------------------------------------------------------------------------
const char* vtkITKArchetypeImageSeriesReader::GetFileName( unsigned int n )
{
  if ( n >= this->GetNumberOfFileNames() )
    {
    return nullptr;
    }
  return this->FileNames[n].c_str();
}

//----------------------------------------------------------------------------
unsigned int vtkITKArchetypeImageSeriesReader::AddFileName( const char* filename )
{
  std::string filenamestr (filename);
  this->FileNames.push_back( filenamestr );
  return this->FileNames.size();
}

//----------------------------------------------------------------------------
void vtkITKArchetypeImageSeriesReader::ResetFileNames( )
{
  this->FileNames.resize( 0 );
  this->AllFileNames.resize( 0 );
  this->SeriesInstanceUIDs.resize( 0 );
  this->ContentTime.resize( 0 );
  this->TriggerTime.resize( 0 );
  this->EchoNumbers.resize( 0 );
  this->DiffusionGradientOrientation.resize( 0 );
  this->SliceLocation.resize( 0 );
  this->ImageOrientationPatient.resize( 0 );
}

//----------------------------------------------------------------------------
int vtkITKArchetypeImageSeriesReader::AssembleVolumeContainingArchetype( )
{
  // we will set FileNames to have only volumes that match the archetype
  // and we will return the size of the list.
  // - if the files have ImagePositionPatient tags, we will sort
  //   the files based on this information
  this->FileNames.resize(0);

  // Note: Since IndexArchetype is unsigned int, it's always positive
  if (this->IndexArchetype >= this->IndexSeriesInstanceUIDs.size()
      || this->IndexArchetype >= this->IndexTriggerTime.size()
      || this->IndexArchetype >= this->IndexDiffusionGradientOrientation.size()
      || this->IndexArchetype >= this->IndexImageOrientationPatient.size())
    {
      vtkErrorMacro("AssembleVolumeContainingArchetype: index archetype "
        << this->IndexArchetype << " is out of bounds 0-" << this->IndexSeriesInstanceUIDs.size());
      this->SetErrorCode(vtkErrorCode::FileFormatError);
      return 0;
    }


  long int iArchetypeSeriesUID = this->IndexSeriesInstanceUIDs[this->IndexArchetype];
  long int iArchetypeEchoNumbers = this->IndexEchoNumbers[this->IndexArchetype];
  long int iArchetypeDiffusion = this->IndexDiffusionGradientOrientation[this->IndexArchetype];
  long int iArchetypeOrientation =  this->IndexImageOrientationPatient[this->IndexArchetype];

  // keep track of the locations for the selected files
  std::vector<std::pair <double, int> > fileNameSortKey;
  bool originSet = false;

  // Sort good files based on distance from Origin to this->ImagePositionPatient along ScanAxis
  // Follows logic originally in LoadVolume.tcl
  for (unsigned int k = 0; k < this->AllFileNames.size(); k++)
    {
    if ((this->IndexSeriesInstanceUIDs[k] != iArchetypeSeriesUID &&
         this->IndexSeriesInstanceUIDs[k] >= 0 && iArchetypeSeriesUID >= 0)
        ||
        (this->IndexEchoNumbers[k] != iArchetypeEchoNumbers &&
         this->IndexEchoNumbers[k] >= 0 && iArchetypeEchoNumbers >= 0)
        ||
        (this->IndexDiffusionGradientOrientation[k] != iArchetypeDiffusion  &&
         this->IndexDiffusionGradientOrientation[k] >= 0 && iArchetypeDiffusion >= 0)
        ||
        (this->IndexImageOrientationPatient[k] != iArchetypeOrientation &&
         this->IndexImageOrientationPatient[k] >= 0 && iArchetypeOrientation >= 0) )
      {
      // file doesn't match our criteria
      continue;
      }
    else
      {
      // find the position and orientation corresponding to this
      // file - it could be that all files in the set have the same index
      // (1) or that there is no position information, in which case the
      // position index will be -1
      int kth_orientation = -1;
      int kth_position = -1;
      if ( this->ImagePositionPatient.size() != 0 )
        {
        kth_orientation = this->IndexImageOrientationPatient[k];
        kth_position = this->IndexImagePositionPatient[k];
        }
      if (kth_orientation > 0 && kth_position > 0)
        {
        if (!originSet)
          {
          std::vector<float> iopv = this->ImageOrientationPatient[kth_orientation];
          float iopf1[] = {iopv[0], iopv[1], iopv[2]};
          float iopf2[] = {iopv[3], iopv[4], iopv[5]};

          vtkMath::Cross( iopf1, iopf2, this->ScanAxis );
          this->ScanOrigin[0] = this->ImagePositionPatient[kth_position][0];
          this->ScanOrigin[1] = this->ImagePositionPatient[kth_position][1];
          this->ScanOrigin[2] = this->ImagePositionPatient[kth_position][2];
          originSet = true;
          }
        float tempiop[3], diff[3];
        tempiop[0] = this->ImagePositionPatient[kth_position][0];
        tempiop[1] = this->ImagePositionPatient[kth_position][1];
        tempiop[2] = this->ImagePositionPatient[kth_position][2];

        vtkMath::Subtract( tempiop, this->ScanOrigin, diff );
        float dist = vtkMath::Dot( diff, ScanAxis );

        fileNameSortKey.emplace_back(dist, k);
        }
      else
        {
        // no position info, so use the file index
        fileNameSortKey.emplace_back(k, k);
        }
      }
    }

  std::sort(fileNameSortKey.begin(), fileNameSortKey.end());
  std::vector<std::pair <double, int> >::iterator keyiter;
  for (keyiter = fileNameSortKey.begin(); keyiter != fileNameSortKey.end(); ++keyiter)
    {
    FileNames.push_back(AllFileNames[keyiter->second]);
    }

  return this->FileNames.size();
}
