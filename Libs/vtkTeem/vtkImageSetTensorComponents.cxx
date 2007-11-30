/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageSetTensorComponents.cxx,v $
  Date:      $Date: 2006/01/06 17:58:05 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
#include "vtkImageSetTensorComponents.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include <vtkStructuredPointsWriter.h>


//----------------------------------------------------------------------------
vtkImageSetTensorComponents* vtkImageSetTensorComponents::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageSetTensorComponents");
  if(ret)
    {
      return (vtkImageSetTensorComponents*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageSetTensorComponents;
}


using namespace std;



//----------------------------------------------------------------------------
vtkImageSetTensorComponents::vtkImageSetTensorComponents()
{
  this->Components[0]=6;
  this->NumberOfComponents = 1;
}


//----------------------------------------------------------------------------
// Replace superclass Execute with a function that allocates tensors
// as well as scalars.  This gets called before multithreader starts
// (after which we can't allocate, for example in ThreadedExecute).
// Note we return to the regular pipeline at the end of this function.
void vtkImageSetTensorComponents::ExecuteData(vtkDataObject *out)
{
  vtkImageData *output = vtkImageData::SafeDownCast(out);

  // set extent so we know how many tensors to allocate
  output->SetExtent(output->GetUpdateExtent());
  
  // allocate output tensors
  vtkFloatArray* data = vtkFloatArray::New();
  int* dims = output->GetDimensions();
  data->SetNumberOfComponents(9);
  data->SetNumberOfTuples(dims[0]*dims[1]*dims[2]);
  output->GetPointData()->SetTensors(data);
  data->Delete();

  // jump back into normal pipeline: call standard superclass method here
  this->vtkImageToImageFilter::ExecuteData(out);
}

//----------------------------------------------------------------------------
void vtkImageSetTensorComponents::ExecuteInformation(
                   vtkImageData *inData, vtkImageData *outData)
{
  int ext[6];
  outData->SetNumberOfScalarComponents(this->NumberOfComponents);
  inData->GetWholeExtent(ext);
  outData->SetWholeExtent(ext);
}


//----------------------------------------------------------------------------
template <class T>
static void vtkImageSetTensorComponentsExecute(vtkImageSetTensorComponents *self,
                                      vtkImageData *inData, T *inPtr,
                                      vtkImageData *outData, T *outPtr,
                                      int outExt[6], int id)
{
  int idxR, idxY, idxZ;
  int maxX, maxY, maxZ;
  int inIncX, inIncY, inIncZ;
  int outIncX, outIncY, outIncZ;
  int offset[9];
  unsigned long count = 0;
  unsigned long target;
  
  vtkDataArray *outTensors;
  float outT[3][3];
  int ptId;
  
  // Get information to march through output tensor data
  outTensors = self->GetOutput()->GetPointData()->GetTensors();
  // changed from arrays to pointers
  int *outInc,*outFullUpdateExt;
  outInc = self->GetOutput()->GetIncrements();
  outFullUpdateExt = self->GetOutput()->GetUpdateExtent(); //We are only working over the update extent
  ptId = ((outExt[0] - outFullUpdateExt[0]) * outInc[0]
         + (outExt[2] - outFullUpdateExt[2]) * outInc[1]
         + (outExt[4] - outFullUpdateExt[4]) * outInc[2]);

  
  // find the region to loop over
  maxX = outExt[1] - outExt[0];
  maxY = outExt[3] - outExt[2]; 
  maxZ = outExt[5] - outExt[4];
  
  target = (unsigned long)((maxZ+1)*(maxY+1)/50.0);
  target++;
  
  // Get increments to march through data 
  inData->GetContinuousIncrements(outExt, inIncX, inIncY, inIncZ);
  self->GetOutput()->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);

  
  // Loop through output pixels
  // Extract from tensor components 1,2,3;2,4,5;3,5,6
  offset[0]=0;offset[1]=1;offset[2]=2;offset[3]=1;offset[4]=3;
  offset[5]=4;offset[6]=2;offset[7]=4;offset[8]=5;
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
        //cout << idxR << " " << idxY << " " << idxZ << " " << ptId << endl;
      outT[0][0]=(float)*inPtr++;
      
      outT[0][1]=(float)*inPtr;
      outT[1][0]=(float)*inPtr++;
      
      outT[0][2]=(float)*inPtr;
      outT[2][0]=(float)*inPtr++;
      
      outT[1][1]=(float)*inPtr++;
      
      outT[1][2]=(float)*inPtr;
      outT[2][1]=(float)*inPtr++;
      
      outT[2][2]=(float)*inPtr++;

          outTensors->SetTuple(ptId,(float *)outT);
      ptId++;
        }
      ptId += outIncY; 
      inPtr += inIncY;
      }
    ptId += outIncZ; 
    inPtr += inIncZ;
    }

}


//----------------------------------------------------------------------------
// This method is passed input and output datas, and executes the
// SetTensorComponents function on each line.  
void vtkImageSetTensorComponents::ThreadedExecute(vtkImageData *inData, 
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
  inTensors = pd->GetScalars();
  max = inTensors->GetNumberOfComponents();
  if (max!=6)
     {
      vtkErrorMacro("Execute: Number of Components is not 6.");
      return;
     }

  
  // choose which templated function to call.
  switch (inData->GetScalarType())
    {
    vtkTemplateMacro7(vtkImageSetTensorComponentsExecute, this, inData, 
                      (VTK_TT *)(inPtr), outData, (VTK_TT *)(outPtr),
                      outExt, id);
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}

void vtkImageSetTensorComponents::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "NumberOfComponents: " << this->NumberOfComponents << endl;
  os << indent << "Components: ( "
     << this->Components[0] << ", "
     << this->Components[1] << ", "
     << this->Components[2] << " )\n";

}

