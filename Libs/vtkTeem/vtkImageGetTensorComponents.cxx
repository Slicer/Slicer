/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageGetTensorComponents.cxx,v $
  Date:      $Date: 2006/01/06 17:58:05 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
#include "vtkImageGetTensorComponents.h"
#include "vtkFloatArray.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkMath.h"
#include <vtkStructuredPointsWriter.h>

//----------------------------------------------------------------------------
vtkImageGetTensorComponents* vtkImageGetTensorComponents::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageGetTensorComponents");
  if(ret)
    {
      return (vtkImageGetTensorComponents*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageGetTensorComponents;
}


using namespace std;


//----------------------------------------------------------------------------
vtkImageGetTensorComponents::vtkImageGetTensorComponents()
{
  this->Components[0] = 9;
  this->NumberOfComponents = 1;
}


//----------------------------------------------------------------------------
void vtkImageGetTensorComponents::ExecuteInformation(
                   vtkImageData *inData, vtkImageData *outData)
{
  int ext[6];
  outData->SetNumberOfScalarComponents(6);
  inData->GetWholeExtent(ext);
  outData->SetWholeExtent(ext);
}

//----------------------------------------------------------------------------
template <class T>
static void vtkImageGetTensorComponentsExecute(vtkImageGetTensorComponents *self,
                                      vtkImageData *inData, 
                                      vtkImageData *outData, T *outPtr,
                                      int outExt[6], int id)
{
  int idxR, idxY, idxZ;
  int maxX, maxY, maxZ;
  int inIncX, inIncY, inIncZ;
  int outIncX, outIncY, outIncZ;
  int offset[6];
  unsigned long count = 0;
  unsigned long target;
  vtkFloatingPointType tensor[3][3];
  
  // find the region to loop over
  maxX = outExt[1] - outExt[0];
  maxY = outExt[3] - outExt[2]; 
  maxZ = outExt[5] - outExt[4];
  target = (unsigned long)((maxZ+1)*(maxY+1)/50.0);
  target++;
  
  vtkDataArray *inTensors;
  vtkPointData *pd;
  int numPts, inPtId;
  
  pd = inData->GetPointData();
  inTensors = pd->GetTensors();
  numPts = inData->GetNumberOfPoints();

  if ( !inTensors || numPts < 1 )
    {
      vtkGenericWarningMacro(<<"No input tensor data to filter!");
      return;
    }
  // Get increments to march through data 
  inData->GetContinuousIncrements(outExt, inIncX, inIncY, inIncZ);
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);

  //Initialize ptId to walk through tensor volume
  int *inInc,*inFullUpdateExt;
  inInc = inData->GetIncrements();
  inFullUpdateExt = inData->GetExtent(); //We are only working over the update extent
  inPtId = ((outExt[0] - inFullUpdateExt[0]) * inInc[0]
     + (outExt[2] - inFullUpdateExt[2]) * inInc[1]
     + (outExt[4] - inFullUpdateExt[4]) * inInc[2]);

  // Loop through output pixels
  // Extract from tensor components 1,2,3,5,6,9
  offset[0]=0;
  offset[1]=1;
  offset[2]=2;
  offset[3]=4;
  offset[4]=5;
  offset[5]=8;
//  cout << "off0 " << offset[0] << endl;
  for (idxZ = 0; idxZ <= maxZ; idxZ++)
    {
    for (idxY = 0; !self->AbortExecute && idxY <= maxY; idxY++)
      {
      if (!id) 
        {
        if (!(count%target))
          {
          self->UpdateProgress(count/(50.0*target));
          }
        count++;
        }
      for (idxR = 0; idxR <= maxX; idxR++)
        {
         inTensors->GetTuple(inPtId,(vtkFloatingPointType *)tensor);
     *outPtr = (T)(tensor[0][0]); 
     outPtr++;
     *outPtr = (T)(tensor[1][0]);
     outPtr++;
     *outPtr = (T)(tensor[2][0]); 
     outPtr++;
     *outPtr = (T)(tensor[1][1]); 
     outPtr++;
     *outPtr = (T)(tensor[1][2]); 
     outPtr++;
     *outPtr = (T)(tensor[2][2]); 
     outPtr++;

        if (inPtId > numPts) 
        {
          vtkGenericWarningMacro(<<"not enough input pts for output extent "<<numPts<<" "<<inPtId);
        }
     
     inPtId++;

        }
      outPtr += outIncY;
      inPtId += inIncY;
      }
    outPtr += outIncZ;
    inPtId += outIncZ;
    }
}


//----------------------------------------------------------------------------
// This method is passed input and output datas, and executes the
// GetTensorComponents function on each line.  
void vtkImageGetTensorComponents::ThreadedExecute(vtkImageData *inData, 
                                                vtkImageData *outData,
                                                int outExt[6], int id)
{
  int max;
  void *inPtr = inData->GetScalarPointerForExtent(outExt);
  void *outPtr = outData->GetScalarPointerForExtent(outExt);
  vtkPointData *pd;
  vtkDataArray *inTensors;
  
  vtkDebugMacro(<< "Execute: inData = " << inData 
                << ", outData = " << outData);
  
  // this filter expects that input is the same type as output.
  if (inData->GetScalarType() != outData->GetScalarType())
    {
    vtkErrorMacro(<< "Execute: input ScalarType, " << inData->GetScalarType()
    << ", must match out ScalarType " << outData->GetScalarType());
    return;
    }
  
  // make sure we can get all of the components.
  pd = inData->GetPointData();
  inTensors = pd->GetTensors();
  max = inTensors->GetNumberOfComponents();
  if (max!=9)
     {
      vtkErrorMacro("Execute: Number of Components is not 9.");
      return;
     }
  
  // choose which templated function to call.
  switch (outData->GetScalarType())
    {
    vtkTemplateMacro6(vtkImageGetTensorComponentsExecute, this, inData, 
                      outData, (VTK_TT *)(outPtr),
                      outExt, id);
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}

void vtkImageGetTensorComponents::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "NumberOfComponents: " << this->NumberOfComponents << endl;
  os << indent << "Components: ( "
     << this->Components[0] << ", "
     << this->Components[1] << ", "
     << this->Components[2] << " )\n";

}

