/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageAccumulateDiscrete.cxx,v $
  Date:      $Date: 2006/04/13 19:30:50 $
  Version:   $Revision: 1.22 $

=========================================================================auto=*/
#include "vtkImageAccumulateDiscrete.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"

#include <math.h>
#include <stdlib.h>

//------------------------------------------------------------------------------
vtkImageAccumulateDiscrete* vtkImageAccumulateDiscrete::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageAccumulateDiscrete");
  if(ret)
    {
    return (vtkImageAccumulateDiscrete*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageAccumulateDiscrete;
}

//----------------------------------------------------------------------------
// Constructor sets default values
vtkImageAccumulateDiscrete::vtkImageAccumulateDiscrete()
{
}

#define  MAX_ACCUMULATION_BIN 65535
//----------------------------------------------------------------------------
void vtkImageAccumulateDiscrete::ExecuteInformation(vtkImageData *vtkNotUsed(input), 
                                                    vtkImageData *output)
{
  int ext[6];
  memset(ext, 0, 6*sizeof(int));

  ext[1] = MAX_ACCUMULATION_BIN;

  vtkFloatingPointType origin[3], spacing[3];
  spacing[0] = spacing[1] = spacing[2] = 1;
  origin[0] = -32768;
  origin[1] = origin[2] = 0;

  output->SetWholeExtent(ext);
  output->SetOrigin(origin);
  output->SetSpacing(spacing);
  output->SetNumberOfScalarComponents(1);
  output->SetScalarType(VTK_INT);
}

//----------------------------------------------------------------------------
// Get ALL of the input.
void vtkImageAccumulateDiscrete::ComputeInputUpdateExtent(int inExt[6],
                                                          int outExt[6])
{
  int *wholeExtent;

  outExt = outExt;
  wholeExtent = this->GetInput()->GetWholeExtent();
  memcpy(inExt, wholeExtent, 6*sizeof(int));
}

//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class T>
static void vtkImageAccumulateDiscreteExecute(vtkImageAccumulateDiscrete *self,
                      vtkImageData *inData, T *inPtr,
                      vtkImageData *outData, int *outPtr)
{
  int min0, max0, min1, max1, min2, max2;
  int idx0, idx1, idx2;
  vtkIdType inInc0, inInc1, inInc2;
  T *inPtr0, *inPtr1, *inPtr2;
  vtkFloatingPointType *origin, *spacing;
  int numC, offset;
  int outExt[6];
  unsigned long count = 0;
  unsigned long target;

  // Zero count in every bin
  outData->GetExtent(min0, max0, min1, max1, min2, max2);
  memset((void *)outPtr, 0,
     (max0-min0+1)*(max1-min1+1)*(max2-min2+1)*sizeof(int));

  // Get information to march through data
  numC = inData->GetNumberOfScalarComponents();
  inData->GetExtent(min0, max0, min1, max1, min2, max2);
  inData->GetIncrements(inInc0, inInc1, inInc2);
  outData->GetExtent(outExt);
  origin = outData->GetOrigin();
  spacing = outData->GetSpacing();
  offset = (int)(-origin[0]);

  // Ignore all components other than first one.
  // NOTE: GetIncrements takes numC into account

  target = (unsigned long)((max2 - min2 + 1)*(max1 - min1 +1)/50.0);
  target++;

  inPtr2 = inPtr;
  for (idx2 = min2; idx2 <= max2; ++idx2)
    {
    inPtr1 = inPtr2;
    for (idx1 = min1; !self->AbortExecute && idx1 <= max1; ++idx1)
      {
      if (!(count%target))
        {
        self->UpdateProgress(count/(50.0*target));
        }
      count++;
      inPtr0  = inPtr1;
      for (idx0 = min0; idx0 <= max0; ++idx0)
        {
        int a = (int)(*inPtr0) + offset;
        if ( a < MAX_ACCUMULATION_BIN && a > 0 )
          {
          outPtr[a]++;
          }
        inPtr0 += inInc0;
        }
      inPtr1 += inInc1;
      }
    inPtr2 += inInc2;
    }
}


//----------------------------------------------------------------------------
// This method is passed a input and output Data, and executes the filter
// algorithm to fill the output from the input.
// It just executes a switch statement to call the correct function for
// the Datas data types.
void vtkImageAccumulateDiscrete::ExecuteData(vtkDataObject *)
{
  vtkImageData *inData = this->GetInput();
  vtkImageData *outData = this->GetOutput();
  outData->SetExtent(this->GetOutput()->GetWholeExtent());
  outData->AllocateScalars();

  void *inPtr;
  int *outPtr;

  inPtr  = inData->GetScalarPointer();
  outPtr = (int *)outData->GetScalarPointer();

  // this filter expects that output is type int.
  if (outData->GetScalarType() != VTK_INT)
  {
    vtkErrorMacro(<< "Execute: out ScalarType " << outData->GetScalarType()
          << " must be int\n");
    return;
  }
  
  int type = inData->GetScalarType();

  switch (type)
  {
    case VTK_CHAR:
      vtkImageAccumulateDiscreteExecute(this, 
              inData, (char *)(inPtr), outData, outPtr);
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageAccumulateDiscreteExecute(this, 
              inData, (unsigned char *)(inPtr), outData, outPtr);
      break;
    case VTK_SHORT:
      vtkImageAccumulateDiscreteExecute(this, 
              inData, (short *)(inPtr), outData, outPtr);
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageAccumulateDiscreteExecute(this, 
              inData, (unsigned short *)(inPtr), outData, outPtr);
      break;
    case VTK_INT:
      vtkImageAccumulateDiscreteExecute(this, 
              inData, (int *)(inPtr), outData, outPtr);
      break;
    case VTK_UNSIGNED_INT:
      vtkImageAccumulateDiscreteExecute(this, 
              inData, (unsigned int *)(inPtr), outData, outPtr);
      break;
    case VTK_LONG:
      vtkImageAccumulateDiscreteExecute(this, 
              inData, (long *)(inPtr), outData, outPtr);
      break;
    case VTK_UNSIGNED_LONG:
      vtkImageAccumulateDiscreteExecute(this, 
              inData, (unsigned long *)(inPtr), outData, outPtr);
      break;
    case VTK_FLOAT:
      vtkImageAccumulateDiscreteExecute(this, 
              inData, (float *)(inPtr), outData, outPtr);
      break;
    case VTK_DOUBLE:
      vtkImageAccumulateDiscreteExecute(this, 
              inData, (double *)(inPtr), outData, outPtr);
      break;
    default:
      vtkErrorMacro(<< "Execute: Unsupported ScalarType");
      return;
    }
}


//----------------------------------------------------------------------------
void vtkImageAccumulateDiscrete::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

