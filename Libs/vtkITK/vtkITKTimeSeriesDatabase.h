/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL: http://www.na-mic.org/svn/Slicer3/trunk/Libs/vtkITK/vtkITKArchetypeImageSeriesReader.h $
  Date:      $Date: 2008-03-11 13:22:52 -0500 (Tue, 11 Mar 2008) $
  Version:   $Revision: 6159 $

==========================================================================*/

// .NAME vtkITKTimeSeriesDatabase - Effeciently process large datasets in small memory
// .SECTION Description
// TimeSeriesDatabase creates a database on disk from a series of volumes
// stored on disk.  The database allows efficient access to volumes,
// slices and voxels through time. 
//
// \note
// This work is part of the National Alliance for Medical Image Computing 
// (NAMIC), funded by the National Institutes of Health through the NIH Roadmap
// for Medical Research, Grant U54 EB005149.

#ifndef __vtkITKTimeSeriesDatabase_h
#define __vtkITKTimeSeriesDatabase_h

#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkImageSource.h"
#include "itkTimeSeriesDatabase.h"
#include "vtkImageImport.h"
#include "itkVTKImageExport.h"
#include <vector>

#include "vtkITK.h"
#include "vtkITKUtility.h"


class VTK_ITK_EXPORT vtkITKTimeSeriesDatabase : public vtkImageSource
{
public:
  // vtkStandardNewMacro ( vtkITKTimeSeriesDatabase );
  static vtkITKTimeSeriesDatabase *New();
  void PrintSelf(ostream& os, vtkIndent indent){};   
  vtkTypeRevisionMacro(vtkITKTimeSeriesDatabase,vtkImageSource);

public:
  // Create a TimeSeriesDatabase from a series of volumes
  static void CreateFromFileArchetype ( const char* TSDFilename, const char* ArchetypeFilename )
  {
    itk::TimeSeriesDatabase<OutputImagePixelType>::CreateFromFileArchetype ( TSDFilename, ArchetypeFilename );
  };
  
  // Connect/Disconnect to a database
  // void Connect ( const char* filename ) { this->m_Filter->Connect ( filename ); this->Modified(); };
  // void Disconnect() { this->m_Filter->Disconnect(); }

  // Get/Set the current time stamp to read 
  void SetCurrentImage ( unsigned int value )
  { DelegateITKInputMacro ( SetCurrentImage, value); }; 
  unsigned int GetCurrentImage ( unsigned int value )
  { DelegateITKOutputMacro ( GetCurrentImage ); }; 

  int GetNumberOfVolumes() 
  { DelegateITKOutputMacro ( GetNumberOfVolumes ); }; 
  
protected:
  vtkITKTimeSeriesDatabase() 
    {
    m_Filter = SourceType::New();
    this->itkExporter = ImageExportType::New();
    this->vtkImporter = vtkImageImport::New();
    ConnectPipelines ( this->itkExporter, this->vtkImporter );
    this->itkExporter->SetInput ( m_Filter->GetOutput() );
    };
  ~vtkITKTimeSeriesDatabase() 
    {
    this->vtkImporter->Delete();
    }
//BTX
  typedef short InputImagePixelType;
  typedef short OutputImagePixelType;
  typedef itk::Image<OutputImagePixelType, 3> OutputImageType;
  typedef itk::VTKImageExport<OutputImageType> ImageExportType;
  typedef itk::TimeSeriesDatabase<OutputImagePixelType> SourceType;
  typedef SourceType ImageFilterType;

  SourceType::Pointer m_Filter;
  ImageExportType::Pointer itkExporter;
  vtkImageImport* vtkImporter;  
  
//ETX
  virtual void ExecuteInformation();
  // defined in the subclasses
  virtual void ExecuteData(vtkDataObject *output);

private:
  vtkITKTimeSeriesDatabase(const vtkITKTimeSeriesDatabase&);  // Not implemented.
  void operator=(const vtkITKTimeSeriesDatabase&);  // Not implemented.


};

#endif
