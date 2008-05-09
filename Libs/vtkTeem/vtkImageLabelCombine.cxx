/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageLabelCombine.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImageLabelCombine.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include <math.h>

vtkCxxRevisionMacro(vtkImageLabelCombine, "$Revision: 1.53 $");
vtkStandardNewMacro(vtkImageLabelCombine);

//----------------------------------------------------------------------------
vtkImageLabelCombine::vtkImageLabelCombine()
{
  this->OverwriteInput = 0;
}

//----------------------------------------------------------------------------
// The output extent is the intersection.
int vtkImageLabelCombine::RequestInformation (
  vtkInformation * vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *inInfo2 = inputVector[1]->GetInformationObject(0);

  int ext[6], ext2[6], idx;

  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),ext);

  // two input take intersection
  if (!inInfo2)
    {
    vtkErrorMacro(<< "Second input must be specified for this operation.");
    return 1;
    }
  
  inInfo2->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),ext2);
  for (idx = 0; idx < 3; ++idx)
    {
    if (ext2[idx*2] > ext[idx*2])
      {
      ext[idx*2] = ext2[idx*2];
      }
    if (ext2[idx*2+1] < ext[idx*2+1])
      {
      ext[idx*2+1] = ext2[idx*2+1];
      }
    }
  
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),ext,6);

  return 1;
}


//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
// Handles the two input operations
template <class T>
void vtkImageLabelCombineExecute2(vtkImageLabelCombine *self,
                                 vtkImageData *in1Data, T *in1Ptr,
                                 vtkImageData *in2Data, T *in2Ptr,
                                 vtkImageData *outData, T *outPtr,
                                 int outExt[6], int id)
{
  int idxR, idxY, idxZ;
  int maxY, maxZ;
  vtkIdType inIncX, inIncY, inIncZ;
  vtkIdType in2IncX, in2IncY, in2IncZ;
  vtkIdType outIncX, outIncY, outIncZ;
  int rowLength;
  unsigned long count = 0;
  unsigned long target;
  int op = self->GetOverwriteInput();
  
  // find the region to loop over
  rowLength = (outExt[1] - outExt[0]+1)*in1Data->GetNumberOfScalarComponents();

  maxY = outExt[3] - outExt[2]; 
  maxZ = outExt[5] - outExt[4];
  target = (unsigned long)((maxZ+1)*(maxY+1)/50.0);
  target++;
  
  T v1;
  T v2;

  // Get increments to march through data 
  in1Data->GetContinuousIncrements(outExt, inIncX, inIncY, inIncZ);
  in2Data->GetContinuousIncrements(outExt, in2IncX, in2IncY, in2IncZ);
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);

  // Loop through ouput pixels
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
      for (idxR = 0; idxR < rowLength; idxR++)
        {
        // Pixel operation
        v1 = *in1Ptr;
        v2 = *in2Ptr;
        if (op)
          {
          if (v1 > 0) 
            {
            *outPtr = v1;
            }
          else if (v2 > 0 && v1 == 0) 
            {
            *outPtr = v2;
            }
          }
        else
          {
          if (v2 > 0) 
            {
            *outPtr = v2;
            }
          else if (v1 > 0 && v2 == 0) 
            {
            *outPtr = v1;
            }
          }
        outPtr++;
        in1Ptr++;
        in2Ptr++;
        }
      outPtr += outIncY;
      in1Ptr += inIncY;
      in2Ptr += in2IncY;
      }
    outPtr += outIncZ;
    in1Ptr += inIncZ;
    in2Ptr += in2IncZ;
    }
}


//----------------------------------------------------------------------------
// This method is passed a input and output datas, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the datas data types.
void vtkImageLabelCombine::ThreadedRequestData(
  vtkInformation * vtkNotUsed( request ), 
  vtkInformationVector ** vtkNotUsed( inputVector ), 
  vtkInformationVector * vtkNotUsed( outputVector ),
  vtkImageData ***inData, 
  vtkImageData **outData,
  int outExt[6], int id)
{
  void *inPtr1;
  void *outPtr;
  
  inPtr1 = inData[0][0]->GetScalarPointerForExtent(outExt);
  outPtr = outData[0]->GetScalarPointerForExtent(outExt);
  

  void *inPtr2;

  if (!inData[1] || ! inData[1][0])
    {
    vtkErrorMacro("ImageMathematics requested to perform a two input operation with only one input\n");
    return;
    }
  
  inPtr2 = inData[1][0]->GetScalarPointerForExtent(outExt);

  // this filter expects that input is the same type as output.
  if (inData[0][0]->GetScalarType() != outData[0]->GetScalarType())
    {
      vtkErrorMacro(<< "Execute: input1 ScalarType, "
                    <<  inData[0][0]->GetScalarType()
                    << ", must match output ScalarType "
                    << outData[0]->GetScalarType());
      return;
    }
  
  if (inData[1][0]->GetScalarType() != outData[0]->GetScalarType())
    {
      vtkErrorMacro(<< "Execute: input2 ScalarType, "
                    << inData[1][0]->GetScalarType()
                    << ", must match output ScalarType "
                    << outData[0]->GetScalarType());
      return;
    }
  
  // this filter expects that inputs that have the same number of components
  if (inData[0][0]->GetNumberOfScalarComponents() != 
      inData[1][0]->GetNumberOfScalarComponents())
    {
      vtkErrorMacro(<< "Execute: input1 NumberOfScalarComponents, "
                    << inData[0][0]->GetNumberOfScalarComponents()
                    << ", must match out input2 NumberOfScalarComponents "
                    << inData[1][0]->GetNumberOfScalarComponents());
      return;
    }
    
  switch (inData[0][0]->GetScalarType())
    {
    vtkTemplateMacro(
                     vtkImageLabelCombineExecute2(this,inData[0][0], (VTK_TT *)(inPtr1), 
                                                  inData[1][0], (VTK_TT *)(inPtr2), 
                                                  outData[0], (VTK_TT *)(outPtr), outExt, id));
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }

}

int vtkImageLabelCombine::FillInputPortInformation(
  int port, vtkInformation* info)
{
  if (port == 1)
    {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  return 1;
}

void vtkImageLabelCombine::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "OverwriteInput: " << this->OverwriteInput  << "\n";

}

