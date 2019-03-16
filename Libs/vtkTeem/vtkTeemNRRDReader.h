/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTeemNRRDReader.h,v $
  Date:      $Date: 2007/06/12 19:13:59 $
  Version:   $Revision: 1.3.2.1 $

=========================================================================auto=*/
/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkTeemNRRDReader.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __vtkTeemNRRDReader_h
#define __vtkTeemNRRDReader_h

#include <string>
#include <map>
#include <iostream>

#include "vtkTeemConfigure.h"
#include "vtkMedicalImageReader2.h"

//#include "vtkDataObject.h"
//#include "vtkImageData.h"

#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkVersion.h>

#include "teem/nrrd.h"

/// \brief Reads Nearly Raw Raster Data files.
///
/// Reads Nearly Raw Raster Data files using the nrrdio library as used in ITK
//
/// \sa vtkImageReader2
class VTK_Teem_EXPORT vtkTeemNRRDReader : public vtkMedicalImageReader2
{
public:
  static vtkTeemNRRDReader *New();

  vtkTypeMacro(vtkTeemNRRDReader,vtkMedicalImageReader2);

  ///
  /// Returns a IJK to RAS transformation matrix
  vtkMatrix4x4* GetRasToIjkMatrix();

  ///
  /// Returns the measurement frame matrix used for tensor valued data.
  vtkMatrix4x4* GetMeasurementFrameMatrix();

  ///
  /// Get a space separated list of all keys in the header
  /// the string is allocated and deleted in this object.  This method
  /// does not support spaces in key names.
  const char* GetHeaderKeys();

  ///
  /// Get a list of keys in the header. Preferred method to use as it
  /// supports spaces in key names.
  const std::vector<std::string> GetHeaderKeysVector();

  ///
  /// Get a map of keys in the header. Preferred method to use as it
  /// supports spaces in key names.
  const std::map<std::string, std::string> GetHeaderKeysMap();

  ///
  /// Get a value given a key in the header
  const char* GetHeaderValue(const char *key);

  /// Get label for specified axis
  const char* GetAxisLabel(unsigned int axis);

  /// Get unit for specified axis
  const char* GetAxisUnit(unsigned int axis);

  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///  is the given file name a NRRD file?
  int CanReadFile(const char* filename) override;

  ///
  /// Valid extentsions
  const char* GetFileExtensions() override
    {
      return ".nhdr .nrrd";
    }

  ///
  /// A descriptive name for this format
  const char* GetDescriptiveName() override
    {
      return "NRRD - Nearly Raw Raster Data";
    }

  //Description:
  /// Report the status of the reading process.
  /// If this is different than zero, there have been some error
  /// parsing the complete header information.
  vtkGetMacro(ReadStatus,int);

  ///
  /// Point data field type
  vtkSetMacro(PointDataType,int);
  vtkGetMacro(PointDataType,int);

  ///
  /// Set the data type: int, float....
  vtkSetMacro(DataType,int);
  vtkGetMacro(DataType,int);

  ///
  //Number of components
  vtkSetMacro(NumberOfComponents,int);
  vtkGetMacro(NumberOfComponents,int);


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

  int NrrdToVTKScalarType( const int nrrdPixelType ) const
  {
  switch( nrrdPixelType )
    {
    default:
    case nrrdTypeDefault:
      return VTK_VOID;
      break;
    case nrrdTypeChar:
      return VTK_CHAR;
      break;
    case nrrdTypeUChar:
      return VTK_UNSIGNED_CHAR;
      break;
    case nrrdTypeShort:
      return VTK_SHORT;
      break;
    case nrrdTypeUShort:
      return VTK_UNSIGNED_SHORT;
      break;
      ///    case nrrdTypeLLong:
      ///      return LONG ;
      ///      break;
      ///    case nrrdTypeULong:
      ///      return ULONG;
      ///      break;
    case nrrdTypeInt:
      return VTK_INT;
      break;
    case nrrdTypeUInt:
      return VTK_UNSIGNED_INT;
      break;
    case nrrdTypeFloat:
      return VTK_FLOAT;
      break;
    case nrrdTypeDouble:
      return VTK_DOUBLE;
      break;
    case nrrdTypeBlock:
      return -1;
      break;
    }
  }

  int VTKToNrrdPixelType( const int vtkPixelType ) const
  {
  switch( vtkPixelType )
    {
    default:
    case VTK_VOID:
      return nrrdTypeDefault;
      break;
    case VTK_CHAR:
      return nrrdTypeChar;
      break;
    case VTK_UNSIGNED_CHAR:
      return nrrdTypeUChar;
      break;
    case VTK_SHORT:
      return nrrdTypeShort;
      break;
    case VTK_UNSIGNED_SHORT:
      return nrrdTypeUShort;
      break;
      ///    case nrrdTypeLLong:
      ///      return LONG ;
      ///      break;
      ///    case nrrdTypeULong:
      ///      return ULONG;
      ///      break;
    case VTK_INT:
      return nrrdTypeInt;
      break;
    case VTK_UNSIGNED_INT:
      return nrrdTypeUInt;
      break;
    case VTK_FLOAT:
      return nrrdTypeFloat;
      break;
    case VTK_DOUBLE:
      return nrrdTypeDouble;
      break;
    }
  }
vtkImageData * AllocateOutputData(vtkDataObject *out, vtkInformation* outInfo) override;
void AllocateOutputData(vtkImageData *out, vtkInformation* outInfo, int *uExtent) override
    { Superclass::AllocateOutputData(out, outInfo, uExtent); }
void AllocatePointData(vtkImageData *out, vtkInformation* outInfo);

protected:
  vtkTeemNRRDReader();
  ~vtkTeemNRRDReader() override;

  static bool GetPointType(Nrrd* nrrdTemp, int& pointDataType, int &numOfComponents);

  vtkSmartPointer<vtkMatrix4x4> RasToIjkMatrix;
  vtkSmartPointer<vtkMatrix4x4> MeasurementFrameMatrix;
  vtkSmartPointer<vtkMatrix4x4> NRRDWorldToRasMatrix;

  std::string CurrentFileName;

  Nrrd *nrrd;

  int ReadStatus;

  int PointDataType;
  int DataType;
  int NumberOfComponents;
  bool UseNativeOrigin;

  std::map <std::string, std::string> HeaderKeyValue;
  std::string HeaderKeys; // buffer for returning key list

  std::map<unsigned int, std::string> AxisLabels;
  std::map<unsigned int, std::string> AxisUnits;

  void ExecuteInformation() override;
  void ExecuteDataWithInformation(vtkDataObject *output, vtkInformation* outInfo) override;

  int tenSpaceDirectionReduce(Nrrd *nout, const Nrrd *nin, double SD[9]);

private:
  vtkTeemNRRDReader(const vtkTeemNRRDReader&) = delete;
  void operator=(const vtkTeemNRRDReader&) = delete;

};

#endif
