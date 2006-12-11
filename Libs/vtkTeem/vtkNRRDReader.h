/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkNRRDReader.h,v $
  Date:      $Date: 2006/04/12 19:00:38 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkNRRDReader.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkNRRDReader - Reads Nearly Raw Raster Data files
// .SECTION Description
// vtkNRRDReader 
// Reads Nearly Raw Raster Data files using the nrrdio library as used in ITK
//

//
// .SECTION See Also
// vtkImageReader2

#ifndef __vtkNRRDReader_h
#define __vtkNRRDReader_h

#include <string>
#include <map>

#include "vtkTeemConfigure.h"
#include "vtkMedicalImageReader2.h"
#include "vtkMatrix4x4.h"

//#include "vtkDataObject.h"
//#include "vtkImageData.h"

#include "vtkPointData.h"

#include "teem/nrrd.h"

#define SCALARS 1
#define VECTORS 2
#define NORMALS 3
#define TENSORS 4

class VTK_TEEM_EXPORT vtkNRRDReader : public vtkMedicalImageReader2
{
public:
  static vtkNRRDReader *New();

  vtkTypeRevisionMacro(vtkNRRDReader,vtkMedicalImageReader2);

  // Description:
  // Returns a IJK to RAS transformation matrix
  vtkMatrix4x4* GetRasToIjkMatrix();

  // Description:
  // Returns the measurement frame matrix used for tensor valued data.
  vtkMatrix4x4* GetMeasurementFrameMatrix();

  // Description:
  // Get a space separated list of all keys in the header
  // the string is allocated and deleted in this object
  char* GetHeaderKeys();

  // Description:
  // Get a value given a key in the header
  char* GetHeaderValue(char *key);

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description: is the given file name a NRRD file?
  virtual int CanReadFile(const char* filename);

  // Description:
  // Valid extentsions
  virtual const char* GetFileExtensions()
    {
      return ".nhdr .nrrd";
    }

  // Description: 
  // A descriptive name for this format
  virtual const char* GetDescriptiveName()
    {
      return "NRRD - Nearly Raw Raster Data";
    }

  //Description:
  // Report the status of the reading process.
  // If this is different than zero, there have been some error
  // parsing the complete header information.
  vtkGetMacro(ReadStatus,int);

  // Description:
  // Point data field type
  vtkSetMacro(PointDataType,int);
  vtkGetMacro(PointDataType,int);
  
  // Description:
  // Set the data type: int, float....
  vtkSetMacro(DataType,int);
  vtkGetMacro(DataType,int);
  
  // Description:
  //Number of components
  vtkSetMacro(NumberOfComponents,int);
  vtkGetMacro(NumberOfComponents,int);
  

  // Description:
  // Use image origin from the file
  void SetUseNativeOriginOn() 
  {
    UseNativeOrigin = true;
  }

  // Description:
  // Use image center as origin
  void SetUseNativeOriginOff() 
  {
    UseNativeOrigin = false;
  }

 int NrrdToVTKScalarType( const int nrrdPixelType );

 int VTKToNrrdPixelType( const int vtkPixelType );

vtkImageData * AllocateOutputData(vtkDataObject *out);
void AllocatePointData(vtkImageData *out);

protected:
  vtkNRRDReader();
  ~vtkNRRDReader();
                         
  vtkMatrix4x4* RasToIjkMatrix;
  vtkMatrix4x4* MeasurementFrameMatrix;

  char* HeaderKeys;
  char* CurrentFileName;

  Nrrd *nrrd;

  int ReadStatus;
  
  int PointDataType;
  int DataType;
  int NumberOfComponents;
  bool UseNativeOrigin;

  //BTX
  std::map <std::string, std::string> HeaderKeyValue;
  //ETX

  virtual void ExecuteInformation();
  virtual void ExecuteData(vtkDataObject *out);

private:
  vtkNRRDReader(const vtkNRRDReader&);  // Not implemented.
  void operator=(const vtkNRRDReader&);  // Not implemented.

};
#endif


