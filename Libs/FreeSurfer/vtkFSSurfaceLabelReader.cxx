/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkFSSurfaceLabelReader.cxx,v $
  Date:      $Date: 2006/05/26 19:40:15 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#include "vtkFSSurfaceLabelReader.h"
#include "vtkObjectFactory.h"
#include "vtkFSIO.h"
#include "vtkByteSwap.h"
#include "vtkFloatArray.h"

//-------------------------------------------------------------------------
vtkFSSurfaceLabelReader* vtkFSSurfaceLabelReader::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkFSSurfaceLabelReader");
  if(ret)
    {
    return (vtkFSSurfaceLabelReader*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkFSSurfaceLabelReader;
}

//-------------------------------------------------------------------------
vtkFSSurfaceLabelReader::vtkFSSurfaceLabelReader()
{
  this->Scalars = NULL;
  this->NumberOfVertices = 0;
  this->NumberOfValues = 0;
  // from the freesurfer colour table, this is the index for the Left-Cerebral-Cortex
  this->LabelOn = 3.0;
  // from the freesurfer colour table, this is the index for ctx-lh-unknown
  this->LabelOff = 1000.0;
}

//-------------------------------------------------------------------------
vtkFSSurfaceLabelReader::~vtkFSSurfaceLabelReader()
{
}

//-------------------------------------------------------------------------
// Returns error codes depending on failure
int vtkFSSurfaceLabelReader::ReadLabel()
{
  FILE* labelFile;
  int numValues = 0;
  int vIndex;
  int vIndexFromFile;
  float  xValue, yValue, zValue, wValue;
  float *scalars;
  vtkFloatArray *output = this->Scalars;

  // Do some basic sanity checks.
  if (output == NULL)
    {
    cerr << "ERROR vtkFSSurfaceLabelReader ReadLabel() : output is null" << endl;
    return this->FS_ERROR_W_OUTPUT_NULL;
    }
  vtkDebugMacro( << "vtkFSSurfaceLabelReader Execute() " << endl);
  
  if (!this->FileName) 
    {
    vtkErrorMacro(<<"vtkFSSurfaceLabelReader Execute: FileName not specified.");
    return this->FS_ERROR_W_NO_FILENAME;
    }
  
  vtkDebugMacro(<<"Reading surface Label data...");
  
  // Try to open the file. It's a plain ascii file
  labelFile = fopen(this->FileName, "r") ;
  if (!labelFile) 
    {
    vtkErrorMacro (<< "Could not open file " << this->FileName);
    return this->FS_ERROR_W_OPEN;
    }  

  // read the comment line
  char lineString[1024];
  fgets (lineString, 1024, labelFile);
  vtkDebugMacro("Comment string:" << lineString);

  // TODO: parse the comment string, there may be a label number before the
  // comma
  
  // This is the number of values in the label file.
  fscanf (labelFile, "%d", &numValues);
  if (numValues < 0) 
    {
    vtkErrorMacro (<< "vtkFSSurfaceLabelReader.cxx Execute: Number of vertices is 0 or negative, can't process file.");
    return this->FS_ERROR_W_NUM_VALUES;
    }

  this->NumberOfValues = numValues;
  
  // Check to see that NumberOfVertices was set and is larger than number of values
  if (this->NumberOfVertices == 0)
    {
    // it wasn't set, use numValues
    vtkErrorMacro(<<"vtkFSSurfaceLabelReader: Number of vertices in the associated scalar file has not been set, using number of values in the file");
    this->NumberOfVertices = numValues;
    }
  
  vtkDebugMacro(<<"vtkFSSurfaceLabelReader: numValues = " << this->NumberOfValues << ", numVertices = " << this->NumberOfVertices);
  
  // Make our float array.
  // scalars = (float*) calloc (numValues, sizeof(float));
  // make the array big enough to hold all vertices, calloc inits all values
  // to zero as a default
  scalars = (float*) calloc (this->NumberOfVertices, sizeof(float));
  if (scalars == NULL)
    {
    vtkErrorMacro(<<"vtkFSSurfaceLabelReader: error allocating " << this->NumberOfVertices << " floats!");
    return this->FS_ERROR_W_ALLOC;
    }
  if (this->LabelOff != 0.0)
    {
    // reset the array to the label off value
    for (int i = 0; i < this->NumberOfVertices; i++)
      {
      scalars[i] = this->LabelOff;
      }
    }

  this->NumberOfValues = 0;
  // For each value in the file...
  for (vIndex = 0; vIndex < numValues; vIndex ++ ) 
    {
    
    // Check for eof.
    if (feof(labelFile)) 
      {
      vtkErrorMacro (<< "vtkFSSurfaceLabelReader.cxx Execute: Unexpected EOF after " << vIndex << " values read. Tried to read " << numValues);
      return this->FS_ERROR_W_EOF;
      }

    // Read the 3 byte int index and 3 float values. The label file
    // has an index/coordinate set for every value.
    // This means that the label file could have fewer values than the
    // number of vertices in the surface.
    // index x y z w
    // Currently, ignoring the values after the index    
    int retval = fscanf(labelFile, "%d %f %f %f %f", &vIndexFromFile, &xValue, &yValue, &zValue, &wValue);

    if (retval != 5)
      {
      vtkErrorMacro("ReadLabel: value #" << this->NumberOfValues << ": error reading line");
      break;
      }
    this->NumberOfValues++;
    
    // Make sure the index is in bounds. If not, print a warning and
    // try to do the next value. If this happens, there is probably a
    // mismatch between the file and the surface, but I think there
    // is a reason for being able to load a mismatched file. But this
    // should raise some kind of message to the user like, "This file
    // appears to be for a different surface; continue loading?"
    if (vIndexFromFile < 0 || vIndexFromFile >= this->NumberOfVertices) 
      { 
      vtkErrorMacro (<< "ReadLabel: value #" << this->NumberOfValues << ": Read an index that is out of bounds (" << vIndexFromFile << " not in 0-" << this->NumberOfVertices << "), ignoring.");
      break;
      }
    
    // Set the value in the scalars array based on the index we read
    // in, not the index in our for loop.
    scalars[vIndexFromFile] = this->LabelOn;
    
    if (numValues < 10000 ||
        (vIndex % 100) == 0)
      {
      this->UpdateProgress(1.0*vIndex/numValues);
      }
    }

  this->SetProgressText("");
  this->UpdateProgress(0.0);
  
  // Close the file.
  fclose (labelFile);

  // Set the array in our output.
  //output->SetArray (scalars, numValues, 0);
  output->SetArray(scalars, this->NumberOfVertices, 0);

  return this->FS_ERROR_W_NONE;
}

//-------------------------------------------------------------------------
void vtkFSSurfaceLabelReader::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataReader::PrintSelf(os,indent);
  os << indent << "Number of vertices: " << this->NumberOfVertices << endl;
  os << indent << "Number of values in file: " << this->NumberOfValues << endl;

  os << indent << "Scalar array: " << endl;
  if (this->Scalars == NULL)
    {
    os << indent.GetNextIndent() << "null" << endl;
    }
  else
    {
    os << indent.GetNextIndent() << "Size = " << this->Scalars->GetNumberOfTuples() << endl;
    }
  os << indent << "LabelOff: " << this->LabelOff << endl;
  os << indent << "LabelOn: " << this->LabelOn << endl;
}
