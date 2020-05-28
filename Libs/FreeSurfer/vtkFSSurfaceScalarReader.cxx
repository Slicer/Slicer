/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkFSSurfaceScalarReader.cxx,v $
  Date:      $Date: 2006/05/26 19:40:14 $
  Version:   $Revision: 1.11 $

=========================================================================auto=*/

// FreeSurfer includes
#include "vtkFSIO.h"
#include "vtkFSSurfaceScalarReader.h"

// VTK includes
#include <vtkByteSwap.h>
#include <vtkFloatArray.h>
#include <vtkObjectFactory.h>

//-------------------------------------------------------------------------
vtkStandardNewMacro(vtkFSSurfaceScalarReader);

//-------------------------------------------------------------------------
vtkFSSurfaceScalarReader::vtkFSSurfaceScalarReader()
{
    this->Scalars = nullptr;
}

//-------------------------------------------------------------------------
vtkFSSurfaceScalarReader::~vtkFSSurfaceScalarReader() = default;

//-------------------------------------------------------------------------
vtkFloatArray * vtkFSSurfaceScalarReader::GetOutput()
{
  return this->Scalars;
}

//-------------------------------------------------------------------------
void vtkFSSurfaceScalarReader::SetOutput(vtkFloatArray *output)
{
  this->Scalars = output;
}

//-------------------------------------------------------------------------
int vtkFSSurfaceScalarReader::ReadFSScalars()
{
  FILE* scalarFile;
  int magicNumber;
  int numValues = 0;
  int numFaces = 0;
  int numValuesPerPoint = 0;
  int vIndex;
  int ivalue;
  float fvalue;
  float *FSscalars;
  vtkFloatArray *output = this->Scalars;

  if (output == nullptr)
  {
      cerr << "ERROR vtkFSSurfaceScalarReader ReadFSScalars() : output is null" << endl;
      return 0;
  }
  vtkDebugMacro( << "vtkFSSurfaceScalarReader Execute() " << endl);

  if (!this->GetFileName()) {
    vtkErrorMacro(<<"vtkFSSurfaceScalarReader Execute: FileName not specified.");
    return 0;
  }

  vtkDebugMacro(<<"Reading surface scalar data...");

  // Try to open the file.
  scalarFile = fopen(this->GetFileName(), "rb") ;
  if (!scalarFile) {
    vtkErrorMacro (<< "Could not open file " << this->GetFileName());
    return 0;
  }


  // In the old file type, there is no magin number; the first three
  // byte int is the number of values. In the (more common) new type,
  // the first three bytes is the magic number. So read three bytes
  // and assume it's a magic number, check and assign it to the number
  // of values if not. New style files also have a number of faces and
  // values per point, which aren't really used.
  vtkFSIO::ReadInt3 (scalarFile, magicNumber);
  if (this->FS_NEW_SCALAR_MAGIC_NUMBER == magicNumber)
    {
    size_t retval;
    retval = fread (&numValues, sizeof(int), 1, scalarFile);
    if (retval == 1)
      {
      vtkByteSwap::Swap4BE (&numValues);
      }
    else
      {
      vtkErrorMacro("Error reading number of values from file " << this->GetFileName());
      }
    retval = fread (&numFaces, sizeof(int), 1, scalarFile);
    if (retval == 1)
      {
      vtkByteSwap::Swap4BE (&numFaces);
      }
    else
      {
      vtkErrorMacro("Error reading number of faces from file " << this->GetFileName());
      }
    retval = fread (&numValuesPerPoint, sizeof(int), 1, scalarFile);
    if (retval == 1)
      {
      vtkByteSwap::Swap4BE (&numValuesPerPoint);
      }
    else
      {
      vtkErrorMacro("Error reading number of values per point, should be 1, in filename " << this->GetFileName());
      }

    if (numValuesPerPoint != 1) {
      vtkErrorMacro (<< "vtkFSSurfaceScalarReader.cxx Execute: Number of values per point is not 1, can't process file.");
      return 0;
    }

  } else {
    numValues = magicNumber;
  }

  if (numValues <= 0) {
    vtkErrorMacro (<< "vtkFSSurfaceScalarReader.cxx Execute: Number of vertices is 0 or negative, can't process file.");
      return 0;
  }

  // Make our float array.
  FSscalars = (float*) calloc (numValues, sizeof(float));

  // For each value, if it's a new style file read a float, otherwise
  // read a two byte int and divide it by 100. Add this value to the
  // array.
  for (vIndex = 0; vIndex < numValues; vIndex ++ ) {

    if (feof(scalarFile)) {
      vtkErrorMacro (<< "vtkFSSurfaceScalarReader.cxx Execute: Unexpected EOF after " << vIndex << " values read.");
      return 0;
    }

    if (this->FS_NEW_SCALAR_MAGIC_NUMBER == magicNumber) {
      size_t retval = fread (&fvalue, sizeof(float), 1, scalarFile);
      if (retval == 1)
        {
        vtkByteSwap::Swap4BE (&fvalue);
        }
      else
        {
        vtkErrorMacro("Error reading fvalue from file " << this->GetFileName());
        }

    } else {
      vtkFSIO::ReadInt2 (scalarFile, ivalue);
      fvalue = ivalue / 100.0;
    }

    FSscalars[vIndex] = fvalue;

    if (numValues < 10000 ||
        (vIndex % 100) == 0)
    {
        this->UpdateProgress(1.0*vIndex/numValues);
    }

  }

  this->SetProgressText("");
  this->UpdateProgress(0.0);

  // Close the file.
  fclose (scalarFile);

  // Set the array in our output.
  output->SetArray (FSscalars, numValues, 0);

  return 1;
}

//-------------------------------------------------------------------------
void vtkFSSurfaceScalarReader::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataReader::PrintSelf(os,indent);
}
