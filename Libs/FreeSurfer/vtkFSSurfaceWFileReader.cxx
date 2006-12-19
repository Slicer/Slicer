/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkFSSurfaceWFileReader.cxx,v $
  Date:      $Date: 2006/05/26 19:40:15 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#include "vtkFSSurfaceWFileReader.h"
#include "vtkObjectFactory.h"
#include "vtkFSIO.h"
#include "vtkByteSwap.h"
#include "vtkFloatArray.h"

//-------------------------------------------------------------------------
vtkFSSurfaceWFileReader* vtkFSSurfaceWFileReader::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkFSSurfaceWFileReader");
  if(ret)
    {
    return (vtkFSSurfaceWFileReader*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkFSSurfaceWFileReader;
}

//-------------------------------------------------------------------------
vtkFSSurfaceWFileReader::vtkFSSurfaceWFileReader()
{
  this->scalars = NULL;
  this->NumberOfVertices = 0;
}

//-------------------------------------------------------------------------
vtkFSSurfaceWFileReader::~vtkFSSurfaceWFileReader()
{
}

//-------------------------------------------------------------------------
// Returns error codes depending on failure
int vtkFSSurfaceWFileReader::ReadWFile()
{
  FILE* wFile;
  int magicNumber;
  int numValues = 0;
  int vIndex;
  int vIndexFromFile;
  float fvalue;
  float *scalars;
  vtkFloatArray *output = this->scalars;

  // Do some basic sanity checks.
  if (output == NULL)
    {
    cerr << "ERROR vtkFSSurfaceWFileReader ReadWFile() : output is null" << endl;
    return this->FS_ERROR_W_OUTPUT_NULL;
    }
  vtkDebugMacro( << "vtkFSSurfaceWFileReader Execute() " << endl);
  
  if (!this->FileName) 
    {
    vtkErrorMacro(<<"vtkFSSurfaceWFileReader Execute: FileName not specified.");
    return this->FS_ERROR_W_NO_FILENAME;
    }
  
  vtkDebugMacro(<<"Reading surface WFile data...");
  
  // Try to open the file.
  wFile = fopen(this->FileName, "rb") ;
  if (!wFile) 
    {
    vtkErrorMacro (<< "Could not open file " << this->FileName);
    return this->FS_ERROR_W_OPEN;
    }  

  // I'm not sure what this is. In the original FreeSurfer code, there
  // is this:
  //
  //    fread2(&ilat,fp);
  //    lat = ilat/10.0;
  //
  // And then the lat variable is not used again. Maybe it was a scale
  // factor of some kind? No idea.
  vtkFSIO::ReadInt2 (wFile, magicNumber);

  // This is the number of values in the wfile.
  vtkFSIO::ReadInt3 (wFile, numValues);
  if (numValues < 0) 
    {
    vtkErrorMacro (<< "vtkFSSurfaceWFileReader.cxx Execute: Number of vertices is 0 or negative, can't process file.");
    return this->FS_ERROR_W_NUM_VALUES;
    }

  // Check to see that NumberOfVertices was set and is larger than number of values
  if (this->NumberOfVertices == 0)
    {
    // it wasn't set, use numValues
    vtkErrorMacro(<<"vtkFSSurfaceWFileReader: Number of vertices in the associated scalar file has not been set, using number of values in the file");
    this->NumberOfVertices = numValues;
    }
  
  vtkDebugMacro(<<"vtkFSSurfaceWFileReader: numValues = " << numValues << ", numVertices = " << this->NumberOfVertices);
  
  // Make our float array.
  // scalars = (float*) calloc (numValues, sizeof(float));
  // make the array big enough to hold all vertices, calloc inits all values
  // to zero as a default
  scalars = (float*) calloc (this->NumberOfVertices, sizeof(float));
  if (scalars == NULL)
    {
    vtkErrorMacro(<<"vtkFSSurfaceWFileReader: error allocating " << this->NumberOfVertices << " floats!");
    return this->FS_ERROR_W_ALLOC;
    }
  
  // For each value in the wfile...
  for (vIndex = 0; vIndex < numValues; vIndex ++ ) 
    {
    
    // Check for eof.
    if (feof(wFile)) 
      {
      vtkErrorMacro (<< "vtkFSSurfaceWFileReader.cxx Execute: Unexpected EOF after " << vIndex << " values read. Tried to read " << numValues);
      return this->FS_ERROR_W_EOF;
      }
    
    // Read the 3 byte int index and float value. The wfile is weird
    // in that there is an index/value pair for every value. I guess
    // this means that the wfile could have fewer values than the
    // number of vertices in the surface, but I've never seen this
    // happen in practice. Additionally, these are usually written
    // with indices from 0->nvertices, so this index value isn't even
    // really needed.
    vtkFSIO::ReadInt3 (wFile, vIndexFromFile);
    vtkFSIO::ReadFloat (wFile, fvalue);
    
    // Make sure the index is in bounds. If not, print a warning and
    // try to do the next value. If this happens, there is probably a
    // mismatch between the wfile and the surface, but I think there
    // is a reason for being able to load a mismatched file. But this
    // should raise some kind of message to the user like, "This wfile
    // appears to be for a different surface; continue loading?"
    if (vIndexFromFile < 0 || vIndexFromFile >= this->NumberOfVertices) 
      { 
      vtkErrorMacro (<< "vtkFSSurfaceWFileReader.cxx Execute: Read an index that is out of bounds (" << vIndexFromFile << " not in 0-" << this->NumberOfVertices << ", breaking.");
      break;
      }
    
    // Set the value in the scalars array based on the index we read
    // in, not the index in our for loop.
    scalars[vIndexFromFile] = fvalue;
    
    if (numValues < 10000 ||
        (vIndex % 100) == 0)
      {
      this->UpdateProgress(1.0*vIndex/numValues);
      }
    }

  this->SetProgressText("");
  this->UpdateProgress(0.0);
  
  // Close the file.
  fclose (wFile);

  // Set the array in our output.
  //output->SetArray (scalars, numValues, 0);
  output->SetArray(scalars, this->NumberOfVertices, 0);

  return this->FS_ERROR_W_NONE;
}

//-------------------------------------------------------------------------
void vtkFSSurfaceWFileReader::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataReader::PrintSelf(os,indent);
}
