/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkFSSurfaceScalarReader.cxx,v $
  Date:      $Date: 2006/05/26 19:40:14 $
  Version:   $Revision: 1.11 $

=========================================================================auto=*/

#include "vtkFSSurfaceScalarReader.h"
#include "vtkObjectFactory.h"
#include "vtkFSIO.h"
#include "vtkByteSwap.h"
#include "vtkFloatArray.h"

//-------------------------------------------------------------------------
vtkFSSurfaceScalarReader* vtkFSSurfaceScalarReader::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkFSSurfaceScalarReader");
  if(ret)
    {
    return (vtkFSSurfaceScalarReader*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkFSSurfaceScalarReader;
}

vtkFSSurfaceScalarReader::vtkFSSurfaceScalarReader()
{
    this->scalars = NULL;

}

vtkFSSurfaceScalarReader::~vtkFSSurfaceScalarReader()
{
}


void vtkFSSurfaceScalarReader::ReadFSScalars()
{
  FILE* scalarFile;
  int magicNumber;
  int numValues = 0;
  int numFaces = 0;
  int numValuesPerPoint = 0;
  int vIndex;
  int ivalue;
  float fvalue;
  float *scalars;
  vtkFloatArray *output = this->scalars;

  if (output == NULL)
  {
      cerr << "ERROR vtkFSSurfaceScalarReader ReadFSScalars() : output is null" << endl;
      return;
  }
  vtkDebugMacro( << "vtkFSSurfaceScalarReader Execute() " << endl);

  if (!this->FileName) {
    vtkErrorMacro(<<"vtkFSSurfaceScalarReader Execute: FileName not specified.");
    return;
  }

  vtkDebugMacro(<<"Reading surface scalar data...");

  // Try to open the file.
  scalarFile = fopen(this->FileName, "rb") ;
  if (!scalarFile) {
    vtkErrorMacro (<< "Could not open file " << this->FileName);
    return;
  }  


  // In the old file type, there is no magin number; the first three
  // byte int is the number of values. In the (more common) new type,
  // the first three bytes is the magic number. So read three bytes
  // and assume it's a magic number, check and assign it to the number
  // of values if not. New style files also have a number of faces and
  // values per point, which aren't really used.
  vtkFSIO::ReadInt3 (scalarFile, magicNumber);
  if (this->FS_NEW_SCALAR_MAGIC_NUMBER == magicNumber) {
    fread (&numValues, sizeof(int), 1, scalarFile);
    fread (&numFaces, sizeof(int), 1, scalarFile);
    fread (&numValuesPerPoint, sizeof(int), 1, scalarFile);
    vtkByteSwap::Swap4BE (&numValues);
    vtkByteSwap::Swap4BE (&numFaces);
    vtkByteSwap::Swap4BE (&numValuesPerPoint);
    
    if (numValuesPerPoint != 1) {
      vtkErrorMacro (<< "vtkFSSurfaceScalarReader.cxx Execute: Number of values per point is not 1, can't process file.");
      return;
    }

  } else {
    numValues = magicNumber;
  }

  if (numValues < 0) {
    vtkErrorMacro (<< "vtkFSSurfaceScalarReader.cxx Execute: Number of vertices is 0 or negative, can't process file.");
      return;
  }
  
  // Make our float array.
  scalars = (float*) calloc (numValues, sizeof(float));

  // For each value, if it's a new style file read a float, otherwise
  // read a two byte int and divide it by 100. Add this value to the
  // array.
  for (vIndex = 0; vIndex < numValues; vIndex ++ ) {

    if (feof(scalarFile)) {
      vtkErrorMacro (<< "vtkFSSurfaceScalarReader.cxx Execute: Unexpected EOF after " << vIndex << " values read.");
      return;
    }

    if (this->FS_NEW_SCALAR_MAGIC_NUMBER == magicNumber) {
      fread (&fvalue, sizeof(float), 1, scalarFile);
      vtkByteSwap::Swap4BE (&fvalue);

    } else {
      vtkFSIO::ReadInt2 (scalarFile, ivalue);
      fvalue = ivalue / 100.0;
    }

    scalars[vIndex] = fvalue;

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
  output->SetArray (scalars, numValues, 0);
}

void vtkFSSurfaceScalarReader::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataReader::PrintSelf(os,indent);
}
