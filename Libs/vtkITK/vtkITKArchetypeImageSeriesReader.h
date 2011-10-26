/*=========================================================================

Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   vtkITK
Module:    $HeadURL$
Date:      $Date$
Version:   $Revision$

==========================================================================*/

///  vtkITKArchetypeImageSeriesReader - Read a series of files
/// that have a common naming convention
/// 
/// ArchetypeImageSeriesReader creates a volume from a series of images
/// stored in files. The series are represented by one filename. This
/// filename, the archetype, is any one of the files in the series.
//
/// \note
/// This work is part of the National Alliance for Medical Image Computing 
/// (NAMIC), funded by the National Institutes of Health through the NIH Roadmap
/// for Medical Research, Grant U54 EB005149.

#ifndef __vtkITKArchetypeImageSeriesReader_h
#define __vtkITKArchetypeImageSeriesReader_h

#include "vtkImageSource.h"
#include "vtkMatrix4x4.h"
#include "itkSpatialOrientation.h"
#include <vector>
#include <string>

#include "itkMetaDataDictionary.h"
#if ITK_VERSION_MAJOR < 4
#include "gdcmDictSet.h"        /// access to dictionary
#endif
#include "gdcmDict.h"           /// access to dictionary
#include "gdcmDictEntry.h"      /// access to dictionary
#include "gdcmGlobal.h"         /// access to dictionary

#include "vtkITK.h"

class VTK_ITK_EXPORT vtkITKArchetypeImageSeriesReader : public vtkImageSource
{
public:
  static vtkITKArchetypeImageSeriesReader *New();
  vtkTypeRevisionMacro(vtkITKArchetypeImageSeriesReader,vtkImageSource);
  void PrintSelf(ostream& os, vtkIndent indent);   

  typedef itk::SpatialOrientation::ValidCoordinateOrientationFlags CoordinateOrientationCode;

  /// 
  /// Specify the archetype filename for the series.
  vtkSetStringMacro(Archetype);
  vtkGetStringMacro(Archetype);

  /// 
  /// See how many file names were generated during ExecuteInformation
  unsigned int GetNumberOfFileNames()
    {
    return this->FileNames.size();
    };

  /// 
  /// Specify the file names to be used when looking for extra files
  /// that match the archetype in defining the volume to load (e.g.
  /// other canidate dicom files to look in for matching tags)
  unsigned int AddFileName( const char* filename );
  const char* GetFileName( unsigned int n );
  void ResetFileNames();

  /// 
  /// Set/Get the default spacing of the data in the file. This will be
  /// used if the reader provided spacing is 1.0. (Default is 1.0)
  vtkSetVector3Macro(DefaultDataSpacing,double);
  vtkGetVector3Macro(DefaultDataSpacing,double);

  /// 
  /// Set/Get the default origin of the data (location of first pixel
  /// in the file). This will be used if the reader provided origin is
  /// 0.0. (Default is 0.0)
  vtkSetVector3Macro(DefaultDataOrigin,double);
  vtkGetVector3Macro(DefaultDataOrigin,double);

  /// 
  /// When reading files which start at an unusual index, this can be added
  /// to the slice number when generating the file name (default = 0)
  vtkSetMacro(FileNameSliceOffset,int);
  vtkGetMacro(FileNameSliceOffset,int);

  /// 
  /// When reading files which have regular, but non contiguous slices
  /// (eg filename.1,filename.3,filename.5)
  /// a spacing can be specified to skip missing files (default = 1)
  vtkSetMacro(FileNameSliceSpacing,int);
  vtkGetMacro(FileNameSliceSpacing,int);

  /// 
  /// The maximum number of files to include in the series. If this is
  /// zero, then all files will be included. (Default is 0)
  vtkSetMacro(FileNameSliceCount,int);
  vtkGetMacro(FileNameSliceCount,int);

  ///  is the given file name a NRRD file?
  virtual int CanReadFile(const char* filename);

  /// 
  /// Set the orientation of the output image
  void SetDesiredCoordinateOrientationToAxial ()
    {
    this->DesiredCoordinateOrientation =
      itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI;
    ///     itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RPS;
    this->UseNativeCoordinateOrientation = 0;
    this->Modified();
    }
  void SetDesiredCoordinateOrientationToCoronal ()
    {
    this->DesiredCoordinateOrientation =
      itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RSA;
    ///      itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RIP;
    this->UseNativeCoordinateOrientation = 0;
    this->Modified();
    }
  void SetDesiredCoordinateOrientationToSagittal ()
  {
    this->DesiredCoordinateOrientation =
      itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ASL;
    ///      itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_AIR;
    this->UseNativeCoordinateOrientation = 0;
    this->Modified();
  }
  void SetDesiredCoordinateOrientationToNative ()
    {
    this->UseNativeCoordinateOrientation = 1;
    this->Modified();
    }

  /// 
  /// Set the data type of pixels in the file.  
  /// If you want the output scalar type to have a different value, set it
  /// after this method is called.
  virtual void SetOutputScalarTypeToDouble()
    {
    UseNativeScalarType = 0;
    this->SetOutputScalarType(VTK_DOUBLE);
    }
  virtual void SetOutputScalarTypeToFloat()
    {
    UseNativeScalarType = 0;
    this->SetOutputScalarType(VTK_FLOAT);
    }
  virtual void SetOutputScalarTypeToLong()
    {
    UseNativeScalarType = 0;
    this->SetOutputScalarType(VTK_LONG);
    }
  virtual void SetOutputScalarTypeToUnsignedLong()
    {
    UseNativeScalarType = 0;
    this->SetOutputScalarType(VTK_UNSIGNED_LONG);
    }
  virtual void SetOutputScalarTypeToInt()
    {
    UseNativeScalarType = 0;
    this->SetOutputScalarType(VTK_INT);
    }
  virtual void SetOutputScalarTypeToUnsignedInt()
    {
    UseNativeScalarType = 0;
    this->SetOutputScalarType(VTK_UNSIGNED_INT);
    }
  virtual void SetOutputScalarTypeToShort()
    {
    UseNativeScalarType = 0;
    this->SetOutputScalarType(VTK_SHORT);
    }
  virtual void SetOutputScalarTypeToUnsignedShort()
    {
    UseNativeScalarType = 0;
    this->SetOutputScalarType(VTK_UNSIGNED_SHORT);
    }
  virtual void SetOutputScalarTypeToChar()
    {
    UseNativeScalarType = 0;
    this->SetOutputScalarType(VTK_CHAR);
    }
  virtual void SetOutputScalarTypeToUnsignedChar()
    {
    UseNativeScalarType = 0;
    this->SetOutputScalarType(VTK_UNSIGNED_CHAR);
    }
  virtual void SetOutputScalarTypeToNative()
    {
    UseNativeScalarType = 1;
    this->Modified();
    }

  /// 
  /// Use image origin from the file
  void SetUseNativeOriginOn() 
    {
    UseNativeOrigin = true;
    }

  /// 
  /// Use image center as origin
  void SetUseNativeOriginOff() 
    {
    UseNativeOrigin = false;
    }

  /// 
  /// Get the file format.  Pixels are this type in the file.
  vtkSetMacro(OutputScalarType, int);
  vtkGetMacro(OutputScalarType, int);

  /// 
  /// Get number of scalars
  vtkSetMacro(NumberOfComponents, unsigned int);
  vtkGetMacro(NumberOfComponents, unsigned int);

  /// 
  /// Whether load in a single file or a series
  vtkSetMacro(SingleFile, int);
  vtkGetMacro(SingleFile, int);

  /// 
  /// Whether to use orientation from file
  vtkSetMacro(UseOrientationFromFile, int);
  vtkGetMacro(UseOrientationFromFile, int);

  /// 
  /// Returns an IJK to RAS transformation matrix
  vtkMatrix4x4* GetRasToIjkMatrix();

  /// 
  /// ITK internally does not register all of the IO types that get built
  /// (possibly due to lingering bugs?) but many slicer users have
  /// GE5 (Signa - magic number: IMGF) files that they need to work
  /// with so we register the factory explictly here
  //
  /// In addition, ITK does register an older dicom parser that incorrectly
  /// report success when reading ill-formed dicom files so we turn that old
  /// parser off.
  //
  void RegisterExtraBuiltInFactories();
  void UnRegisterDeprecatedBuiltInFactories();

  /// 
  /// Return the MetaDataDictionary from the ITK layer
  const itk::MetaDataDictionary &GetMetaDataDictionary() const;
  std::vector<std::string> Tags;
  std::vector<std::string> TagValues;
  void ParseDictionary();

  unsigned int GetNumberOfItemsInDictionary(); 
  bool HasKey( char* tag );
  const char* GetNthKey( unsigned int n );
  const char* GetNthValue( unsigned int n );
  const char* GetTagValue( char* tag );

  /// set/get functions for grouping by tags
  bool GetGroupingByTags()
    {
    return GroupingByTags;
    }

  void SetGroupingByTagsOn()
    {
    GroupingByTags = true;
    }

  void SetGroupingByTagsOff()
    {
    GroupingByTags = false;
    }

  /// -------
  int GetSelectedUID()
    {
    return SelectedUID;
    }

  void SetSelectedUID( int v )
    {
    SelectedUID = v;
    SetGroupingByTagsOn();
    }

  /// -------
  int GetSelectedContentTime()
    {
    return SelectedContentTime;
    }

  void SetSelectedContentTime( int v )
    {
    SelectedContentTime = v;
    SetGroupingByTagsOn();
    }

  /// -------
  int GetSelectedTriggerTime()
    {
    return SelectedTriggerTime;
    }

  void SetSelectedTriggerTime( int v )
    {
    SelectedTriggerTime = v;
    SetGroupingByTagsOn();
    }

  /// -------
  int GetSelectedEchoNumbers()
    {
    return SelectedEchoNumbers;
    }

  void SetSelectedEchoNumbers( int v )
    {
    SelectedEchoNumbers = v;
    SetGroupingByTagsOn();
    }

  
  /// -------
  int GetSelectedDiffusion()
    {
    return SelectedDiffusion;
    }

  void SetSelectedDiffusion( int v )
    {
    SelectedDiffusion = v;
    SetGroupingByTagsOn();
    }

  /// -------
  int GetSelectedSlice()
    {
    return SelectedSlice;
    }

  void SetSelectedSlice( int v )
    {
    SelectedSlice = v;
    SetGroupingByTagsOn();
    }

  /// -------
  int GetSelectedOrientation()
    {
    return SelectedOrientation;
    }

  void SetSelectedOrientation( int v )
    {
    SelectedOrientation = v;
    SetGroupingByTagsOn();
    }

  /// get number of certain discriminators in the directory
  unsigned int GetNumberOfSeriesInstanceUIDs()
    {
    return this->SeriesInstanceUIDs.size();
    }

  unsigned int GetNumberOfContentTime()
    {
    return this->ContentTime.size();
    }

  unsigned int GetNumberOfTriggerTime()
    {
    return this->TriggerTime.size();
    }

  unsigned int GetNumberOfEchoNumbers()
    {
    return this->EchoNumbers.size();
    }  

  unsigned int GetNumberOfSliceLocation()
    {
    return this->SliceLocation.size();
    }

  unsigned int GetNumberOfDiffusionGradientOrientation()
    {
    return this->DiffusionGradientOrientation.size();
    };

  unsigned int GetNumberOfImageOrientationPatient()
    {
    return this->ImageOrientationPatient.size();
    };

  unsigned int GetNumberOfImagePositionPatient()
    {
    return this->ImagePositionPatient.size();
    }

  /// check the existance of given discriminator
  int ExistSeriesInstanceUID( const char* SeriesInstanceUID )
    {
    for (unsigned int k = 0; k < GetNumberOfSeriesInstanceUIDs(); k++)
      {
      if ( this->SeriesInstanceUIDs[k].find(SeriesInstanceUID) != std::string::npos )
        {
        return k;
        }
      }
    return -1;
    }

  int ExistContentTime( const char* contentTime )
    {
      for (unsigned int k = 0; k < GetNumberOfContentTime(); k++)
        {
        if ( this->ContentTime[k].find(contentTime) != std::string::npos )
          {
          return k;
          }
        }
      return -1;
    }

  int ExistTriggerTime( const char* triggerTime )
    {
      for (unsigned int k = 0; k < GetNumberOfTriggerTime(); k++)
        {
        if ( this->TriggerTime[k].find(triggerTime) != std::string::npos )
          {
          return k;
          }
        }
      return -1;
    }

  int ExistEchoNumbers( const char* echoNumbers )
    {
      for (unsigned int k = 0; k < GetNumberOfEchoNumbers(); k++)
        {
        if ( this->EchoNumbers[k].find(echoNumbers) != std::string::npos )
          {
          return k;
          }
        }
      return -1;
    }

  int ExistDiffusionGradientOrientation( float* dgo )
    {
      float a = 0;
      for (int n = 0; n < 3; n++)
        {
        a += dgo[n]*dgo[n];
        }
      
      for (unsigned int k = 0; k < GetNumberOfDiffusionGradientOrientation(); k++)
        {
        float b = 0;
        float c = 0;
        for (int n = 0; n < 3; n++)
          {
          b += this->DiffusionGradientOrientation[k][n] * this->DiffusionGradientOrientation[k][n];
          c += this->DiffusionGradientOrientation[k][n] * dgo[n];
          }
        c = fabs(c)/sqrt(a*b);
        
        if ( c > 0.99999 )
          {
          return k;
          }
        }
      return -1;
    }

  int ExistSliceLocation( float sliceLocation )
    {
      for (unsigned int k = 0; k < GetNumberOfSliceLocation(); k++)
        {
        if ( this->SliceLocation[k] == sliceLocation )
          {
          return k;
          }
        }
      return -1;
    }

  int ExistImageOrientationPatient( float * directionCosine )
    {
      /// input has to have six elements
      float a = sqrt( directionCosine[0]*directionCosine[0] + directionCosine[1]*directionCosine[1] + directionCosine[2]*directionCosine[2] );
      for (int k = 0; k < 3; k++)
        {
        directionCosine[k] /= a;
        }
      a = sqrt( directionCosine[3]*directionCosine[3] + directionCosine[4]*directionCosine[4] + directionCosine[5]*directionCosine[5] );
      for (int k = 3; k < 6; k++)
        {
        directionCosine[k] /= a;
        }
      
      for (unsigned int k = 0; k < GetNumberOfImageOrientationPatient(); k++)
        {
        std::vector<float> aVec = ImageOrientationPatient[k];
        a = sqrt( aVec[0]*aVec[0] + aVec[1]*aVec[1] + aVec[2]*aVec[2] );
        float b = (directionCosine[0]*aVec[0] + directionCosine[1]*aVec[1] + directionCosine[2]*aVec[2])/a;
        if ( b < 0.99999 )
          {
          continue;
          }

        a = sqrt( aVec[3]*aVec[3] + aVec[4]*aVec[4] + aVec[5]*aVec[5] );
        b = (directionCosine[3]*aVec[3] + directionCosine[4]*aVec[4] + directionCosine[5]*aVec[5])/a;
        if ( b > 0.99999 )
          {
          return k;
          } 
        }
      return -1;
    }
  
  int ExistImagePositionPatient( float* ipp )
    {
      float a = 0;
      for (int n = 0; n < 3; n++)
        {
        a += ipp[n]*ipp[n];
        }

      for (unsigned int k = 0; k < GetNumberOfImagePositionPatient(); k++)
        {
        float b = 0;
        float c = 0;
        for (int n = 0; n < 3; n++)
          {
          b += this->ImagePositionPatient[k][n] * this->ImagePositionPatient[k][n];
          c += this->ImagePositionPatient[k][n] * ipp[n];
          }
        c = fabs(c)/sqrt(a*b);
        if ( c > 0.99999 )
          {
          return k;
          }
        }
      return -1;
    }
    
  /// methods to get N-th discriminator
  const char* GetNthSeriesInstanceUID( unsigned int n )
    {
      if ( n >= this->GetNumberOfSeriesInstanceUIDs() )
        {
        return NULL;
        } 
      return this->SeriesInstanceUIDs[n].c_str();
    }

  const char* GetNthContentTime( unsigned int n )
    {
      if ( n >= this->GetNumberOfContentTime() )
        {
        return NULL;
        } 
      return this->ContentTime[n].c_str();
    }
  
  const char* GetNthTriggerTime( unsigned int n )
    {
      if ( n >= this->GetNumberOfTriggerTime() )
        {
        return NULL;
        } 
      return this->TriggerTime[n].c_str();
    }

  const char* GetNthEchoNumbers( unsigned int n )
    {
      if ( n >= this->GetNumberOfEchoNumbers() )
        {
        return NULL;
        } 
      return this->EchoNumbers[n].c_str();
    }

  float* GetNthDiffusionGradientOrientation( unsigned int n )
    {
      if ( n >= this->GetNumberOfDiffusionGradientOrientation() )
        {
        return NULL;
        } 
      float *dgo = new float [3];
      for (int k = 0; k <3; k++)
        {
        dgo[k] = this->DiffusionGradientOrientation[n][k];
        }
      return dgo;
    }

  float GetNthSliceLocation( unsigned int n )
    {
      if ( n >= this->GetNumberOfSliceLocation() )
        {
        return this->SliceLocation[0];
        } 
      return this->SliceLocation[0];
    }
  
  float* GetNthImageOrientationPatient( unsigned int n )
    {
      if ( n >= this->GetNumberOfImageOrientationPatient() )
        {
        return NULL;
        } 
      float *dgo = new float [6];
      for (int k = 0; k <6; k++)
        {
        dgo[k] = this->ImageOrientationPatient[n][k];
        }
      return dgo;
    }

  float* GetNthImagePositionPatient( unsigned int n )
    {
      if (n >= this->GetNumberOfImagePositionPatient() )
        {
        return NULL;
        }
      float *ipp = new float [3];
      for (int k = 0; k <3; k++)
        {
        ipp[k] = this->ImagePositionPatient[n][k];
        }
      return ipp;
    }

  /// insert unique item into array. Duplicate code for TCL wrapping. 
  /// TODO: need to clean up
  int InsertSeriesInstanceUIDs ( const char * aUID )
    {
      int k = ExistSeriesInstanceUID( aUID );
      if ( k >= 0 )
        {
        return k;
        }
      
      std::string aVector(aUID);
      this->SeriesInstanceUIDs.push_back( aVector );
      return (this->SeriesInstanceUIDs.size()-1);
    }

  int InsertContentTime ( const char * aTime )
    {
      int k = ExistContentTime( aTime );
      if ( k >= 0 )
        {
        return k;
        }
      
      std::string aVector(aTime);
      this->ContentTime.push_back( aVector );
      return (this->ContentTime.size()-1);
    }

  int InsertTriggerTime ( const char * aTime )
    {
      int k = ExistTriggerTime( aTime );
      if ( k >= 0 )
        {
        return k;
        }
      
      std::string aVector(aTime);
      this->TriggerTime.push_back( aVector );
      return (this->TriggerTime.size()-1);
    }

  int InsertEchoNumbers ( const char * aEcho )
    {
      int k = ExistEchoNumbers( aEcho );
      if ( k >= 0 )
        {
        return k;
        }
      
      std::string aVector(aEcho);
      this->EchoNumbers.push_back( aVector );
      return (this->EchoNumbers.size()-1);
    }
  
  int InsertDiffusionGradientOrientation ( float *a )
    {
      int k = ExistDiffusionGradientOrientation( a );
      if ( k >= 0 )
        {
        return k;
        }
      std::vector< float > aVector(3);
      float aMag = sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
      for (k = 0; k < 3; k++)
        {
        aVector[k] = a[k]/aMag;
        }
      
      this->DiffusionGradientOrientation.push_back( aVector );
      return (this->DiffusionGradientOrientation.size()-1);
    }

  int InsertSliceLocation ( float a )
    {
      int k = ExistSliceLocation( a );
      if ( k >= 0 )
        {
        return k;
        }

      this->SliceLocation.push_back( a );
      return (this->SliceLocation.size()-1);
    }

  int InsertImageOrientationPatient ( float *a )
    {
      int k = ExistImageOrientationPatient( a );
      if ( k >= 0 )
        {
        return k;
        }
      std::vector< float > aVector(6);
      float aMag = sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
      float bMag = sqrt(a[3]*a[3]+a[4]*a[4]+a[5]*a[5]);
      for (k = 0; k < 3; k++)
        {
        aVector[k] = a[k]/aMag;
        aVector[k+3] = a[k+3]/bMag;
        }

      this->ImageOrientationPatient.push_back( aVector );
      return (this->ImageOrientationPatient.size()-1);
    }

  int InsertImagePositionPatient ( float *a )
    {
      int k = ExistImagePositionPatient( a );
      if ( k >= 0 )
        {
        return k;
        }
      
      std::vector< float > aVector(3);
      for ( unsigned int i = 0; i < 3; i++ ) aVector[i] = a[i]; 
      this->ImagePositionPatient.push_back( aVector );
      return (this->ImagePositionPatient.size()-1);
    }

  void AnalyzeDicomHeaders( );

  void AssembleNthVolume( int n );
  int AssembleVolumeContainingArchetype();

  void GroupFiles ( int idxSeriesInstanceUID,
                    int idxContentTime,
                    int idxTriggerTime,
                    int idxEchoNumbers,
                    int idxDiffusionGradientOrientation,
                    int idxSliceLocation,
                    int idxImageOrientationPatient );

  const char* GetNthFileName ( int idxSeriesInstanceUID,
                               int idxContentTime,
                               int idxTriggerTime,
                               int idxEchoNumbers,
                               int idxDiffusionGradientOrientation,
                               int idxSliceLocation,
                               int idxImageOrientationPatient,
                               int n );


protected:
  vtkITKArchetypeImageSeriesReader();
  ~vtkITKArchetypeImageSeriesReader();

  char *Archetype;
  int SingleFile;
  int UseOrientationFromFile;
  int DataExtent[6];

  int          OutputScalarType;
  unsigned int NumberOfComponents;

  double DefaultDataSpacing[3];
  double DefaultDataOrigin[3];
  float ScanAxis[3];
  float ScanOrigin[3];

  int FileNameSliceOffset;
  int FileNameSliceSpacing;
  int FileNameSliceCount;

  vtkMatrix4x4* RasToIjkMatrix;

  char UseNativeCoordinateOrientation;
  char UseNativeScalarType;
  bool UseNativeOrigin;

  bool GroupingByTags;
  int SelectedUID;
  int SelectedContentTime;
  int SelectedTriggerTime;
  int SelectedEchoNumbers;
  int SelectedDiffusion;
  int SelectedSlice;
  int SelectedOrientation;

  unsigned int IndexArchetype;

  std::vector<std::string> FileNames;
  std::vector<std::pair <double, int> > FileNameSliceKey;
  CoordinateOrientationCode DesiredCoordinateOrientation;
  virtual void ExecuteInformation();
  /// defined in the subclasses
  virtual void ExecuteData(vtkDataObject *data);

  itk::MetaDataDictionary Dictionary;

  /// The following variables provide support
  /// for reading a directory with multiple series/groups.
  /// The current scheme is to check the following and see 
  /// if multiple values exist:
  /// 
  /// SeriesInstanceUID              0020,000E
  /// ContentTime                    0008,0033
  /// TriggerTime                    0018,1060
  /// EchoNumbers                    0018,0086
  /// DiffusionGradientOrientation   0018,9089 
  /// SliceLocation                  0020,1041
  /// ImageOrientationPatient        0020,0037
  /// ImagePositionPatient           0020,0032

  std::vector<std::string> AllFileNames;
  bool AnalyzeHeader;
  bool IsOnlyFile;

  std::vector<std::string> SeriesInstanceUIDs;
  std::vector<std::string> ContentTime;
  std::vector<std::string> TriggerTime;
  std::vector<std::string> EchoNumbers;
  std::vector< std::vector<float> > DiffusionGradientOrientation;
  std::vector<float> SliceLocation;
  std::vector< std::vector<float> > ImageOrientationPatient;
  std::vector< std::vector<float> > ImagePositionPatient;

  /// index of each dicom file into the above arrays
  std::vector<long int> IndexSeriesInstanceUIDs;
  std::vector<long int> IndexContentTime;
  std::vector<long int> IndexTriggerTime;
  std::vector<long int> IndexEchoNumbers;  
  std::vector<long int> IndexDiffusionGradientOrientation;
  std::vector<long int> IndexSliceLocation;
  std::vector<long int> IndexImageOrientationPatient;
  std::vector<long int> IndexImagePositionPatient;

private:
  vtkITKArchetypeImageSeriesReader(const vtkITKArchetypeImageSeriesReader&);  /// Not implemented.
  void operator=(const vtkITKArchetypeImageSeriesReader&);  /// Not implemented.
};

#endif
