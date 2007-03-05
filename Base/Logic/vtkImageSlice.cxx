/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageSlice.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImageSlice.h"

#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTransform.h"
#include "vtkDataSetAttributes.h"
#include "vtkPointData.h"

#include "vtkTemplateAliasMacro.h"
// turn off 64-bit ints when templating over all types
# undef VTK_USE_INT64
# define VTK_USE_INT64 0
# undef VTK_USE_UINT64
# define VTK_USE_UINT64 0

#include <limits.h>
#include <float.h>
#include <math.h>

vtkCxxRevisionMacro(vtkImageSlice, "$Revision: 1.63.6.2 $");
vtkStandardNewMacro(vtkImageSlice);
//
// TODO: why?
//vtkCxxSetObjectMacro(vtkImageSlice,SliceTransform,vtkAbstractTransform);

//----------------------------------------------------------------------------
vtkImageSlice::vtkImageSlice()
{
  // if NULL, the main Input is used

  // flag to use default Spacing
  this->OutputSpacing[0] = 1.0;
  this->OutputSpacing[1] = 1.0;
  this->OutputSpacing[2] = 1.0;

  // ditto
  this->OutputOrigin[0] = 0.0;
  this->OutputOrigin[1] = 0.0;
  this->OutputOrigin[2] = 0.0;

  // ditto
  this->OutputDimensions[0] = 256;
  this->OutputDimensions[1] = 256;
  this->OutputDimensions[2] = 1;

  this->InterpolationMode = VTK_SLICE_NEAREST; // no interpolation

  // default black background
  this->BackgroundColor[0] = 0;
  this->BackgroundColor[1] = 0;
  this->BackgroundColor[2] = 0;
  this->BackgroundColor[3] = 0;

  this->SliceTransform = NULL;

  // There is an optional second input.
  this->SetNumberOfInputPorts(1);
}

//----------------------------------------------------------------------------
vtkImageSlice::~vtkImageSlice()
{
  this->SetSliceTransform(NULL);
}

//----------------------------------------------------------------------------
void vtkImageSlice::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "SliceTransform: " << this->SliceTransform << "\n";
  if (this->SliceTransform)
    {
    this->SliceTransform->PrintSelf(os,indent.GetNextIndent());
    }
  os << indent << "OutputSpacing: " << this->OutputSpacing[0] << " " <<
    this->OutputSpacing[1] << " " << this->OutputSpacing[2] << "\n";
  os << indent << "OutputOrigin: " << this->OutputOrigin[0] << " " <<
    this->OutputOrigin[1] << " " << this->OutputOrigin[2] << "\n";
  os << indent << "OutputDimensions: " << this->OutputDimensions[0] << " " <<
    this->OutputDimensions[1] << " " << this->OutputDimensions[2] << "\n";
  os << indent << "InterpolationMode: " 
     << this->GetInterpolationModeAsString() << "\n";
  os << indent << "BackgroundColor: " <<
    this->BackgroundColor[0] << " " << this->BackgroundColor[1] << " " <<
    this->BackgroundColor[2] << " " << this->BackgroundColor[3] << "\n";
  os << indent << "BackgroundLevel: " << this->BackgroundColor[0] << "\n";
}

// the MTime of the filter
unsigned long int vtkImageSlice::GetMTime()
{
  unsigned long mTime=this->vtkObject::GetMTime();
  unsigned long time;

  if ( this->SliceTransform != NULL )
    {
    time = this->SliceTransform->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    if (this->SliceTransform->IsA("vtkHomogeneousTransform"))
      { // this is for people who directly modify the transform matrix
      time = ((vtkHomogeneousTransform *)this->SliceTransform)
        ->GetMatrix()->GetMTime();
      mTime = ( time > mTime ? time : mTime );
      }    
    }

  return mTime;
}

//----------------------------------------------------------------------------
// TODO
int vtkImageSlice::RequestUpdateExtent(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  int inExt[6], outExt[6];
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);

  outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), outExt);

  if (this->SliceTransform)
    {
    this->SliceTransform->Update();
    if (!this->SliceTransform->IsA("vtkHomogeneousTransform"))
      { // update the whole input extent if the transform is nonlinear
      inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), inExt);
      inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), inExt, 6);
      return 1;
      }
    }

  // TODO: for now, request the full extent...
  // - later, to optimize, pass the output dimensions through the
  // slice transform and find the extent in input space (for most
  // slicing jobs this will essentially be the full extent anyway...)
  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), inExt);
  inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), inExt, 6);
  return 1;
}

//----------------------------------------------------------------------------
int vtkImageSlice::FillInputPortInformation(int port, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  return 1;
}


//----------------------------------------------------------------------------
// TODO
int vtkImageSlice::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  int outWholeExt[6];

  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  outWholeExt[0] = 0;
  outWholeExt[1] = this->OutputDimensions[0] - 1;
  outWholeExt[2] = 0;
  outWholeExt[3] = this->OutputDimensions[1] - 1;
  outWholeExt[4] = 0;
  outWholeExt[5] = this->OutputDimensions[2] - 1;


  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),outWholeExt,6);
  outInfo->Set(vtkDataObject::SPACING(), this->OutputSpacing, 3);
  outInfo->Set(vtkDataObject::ORIGIN(), this->OutputOrigin, 3);

  return 1;
}

//----------------------------------------------------------------------------
// This function executes the filter for any type of data.  
template <class T>
void vtkImageSliceExecute(vtkImageSlice *self,
                            vtkImageData *inData, 
                            vtkImageData *outData, T *outPtr,
                            int outExt[6], int id)
{
  int numscalars;
  int idX, idY, idZ;
  vtkIdType outIncX, outIncY, outIncZ;
  int scalarSize;
  int inExt[6];
  unsigned long count = 0;
  unsigned long target;
  int intPoint[3];
  vtkIdType scalarIncs[3], scalarsMaxId;

  vtkDataArray *inScalars = inData->GetPointData()->GetScalars();
  inData->GetArrayIncrements(inScalars, scalarIncs);
  scalarsMaxId = inScalars->GetMaxId();

  // the transformation to apply to the data
  vtkAbstractTransform *transform = self->GetSliceTransform();

  // for the progress meter
  target = (unsigned long)
    ((outExt[5]-outExt[4]+1)*(outExt[3]-outExt[2]+1)/50.0);
  target++;
  
  // Get Increments to march through data 
  inData->GetExtent(inExt);
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);
  scalarSize = outData->GetScalarSize();
  numscalars = inData->GetNumberOfScalarComponents();

//vtkDebugMacro( "id " << id << " ext " << outExt[0] << " " << outExt[1] << " " << outExt[2] << " " << outExt[3] << " " << outExt[4] << " " << outExt[5] << "\n"); 

  double steps = (outExt[1] - outExt[0]);
  double invSteps = ( ( steps == 0.0 ) ? 1.0 : 1.0 / steps );


  // Loop through output voxels
  for (idZ = outExt[4]; idZ <= outExt[5]; idZ++)
    {
    for (idY = outExt[2]; idY <= outExt[3]; idY++)
      {
      if (id == 0) 
        { // update the progress if this is the main thread
        if (!(count%target)) 
          {
          self->UpdateProgress(count/(50.0*target));
          }
        count++;
        }

      // 
      // calculate the delta IJK with per step of X
      //

      double ijk[3], ijkStart[3], ijkEnd[3];
      double dIJKdX[3];

      ijkStart[0] = outExt[0];
      ijkEnd[0] = outExt[1];
      ijkStart[1] = ijkEnd[1] = idY;
      ijkStart[2] = ijkEnd[2] = idZ;

      // apply SliceTransform
      if (transform)
        {
        transform->InternalTransformPoint(ijkStart, ijkStart);
        transform->InternalTransformPoint(ijkEnd, ijkEnd);
        }
      
      dIJKdX[0] = (ijkEnd[0] - ijkStart[0]) * invSteps;
      dIJKdX[1] = (ijkEnd[1] - ijkStart[1]) * invSteps;
      dIJKdX[2] = (ijkEnd[2] - ijkStart[2]) * invSteps;

      ijk[0] = ijkStart[0];
      ijk[1] = ijkStart[1];
      ijk[2] = ijkStart[2];
      T *inPtr;
      T *baseInPtr = (T *) inScalars->GetVoidPointer(0);
      
      for (idX = outExt[0]; idX <= outExt[1]; idX++)
        {

        intPoint[0] = (int) (ijk[0] += dIJKdX[0]);
        intPoint[1] = (int) (ijk[1] += dIJKdX[1]);
        intPoint[2] = (int) (ijk[2] += dIJKdX[2]);

        if ( intPoint[0] < inExt[0] || intPoint[0] > inExt[1] ||
             intPoint[1] < inExt[2] || intPoint[1] > inExt[3] ||
             intPoint[2] < inExt[4] || intPoint[2] > inExt[5] )
          {
          for (int i = 0; i < numscalars; i++)
            {
            *outPtr++ = (T)100;
            }
          }
        else
          {
          //T *inPtr = (T *) (inData->GetArrayPointer(inScalars, intPoint));


          // compute the index of the vector.
          vtkIdType idx = ((intPoint[0] - inExt[0]) * scalarIncs[0]
                         + (intPoint[1] - inExt[2]) * scalarIncs[1]
                         + (intPoint[2] - inExt[4]) * scalarIncs[2]);

          if ( idx < 0 || idx > scalarsMaxId )
            {
            vtkErrorWithObjectMacro(self, "Bad index calculation!");
            }

          inPtr = baseInPtr + idx;
          for (int i = 0; i < numscalars; i++)
            {
            *outPtr++ = *inPtr++;
            }
          }
        }
      outPtr = (T *)((char *)outPtr + outIncY*scalarSize);
      }
    outPtr = (T *)((char *)outPtr + outIncZ*scalarSize);
    }

}

//----------------------------------------------------------------------------
// This method is passed a input and output region, and executes the filter
// algorithm to fill the output from the input.
// It just executes a switch statement to call the correct function for
// the regions data types.
void vtkImageSlice::ThreadedRequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *vtkNotUsed(outputVector),
  vtkImageData ***inData,
  vtkImageData **outData,
  int outExt[6], int id)
{

  vtkDebugMacro(<< "Execute: inData = " << inData[0][0]
                      << ", outData = " << outData[0]);

  // this filter expects that input is the same type as output.
  if (inData[0][0]->GetScalarType() != outData[0]->GetScalarType())
    {
    vtkErrorMacro(<< "Execute: input ScalarType, "
                  << inData[0][0]->GetScalarType()
                  << ", must match out ScalarType "
                  << outData[0]->GetScalarType());
    return;
    }

  // Get the output pointer
  void *outPtr = outData[0]->GetScalarPointerForExtent(outExt);

  switch (inData[0][0]->GetScalarType())
    {
    vtkTemplateMacro( 
      vtkImageSliceExecute(this, inData[0][0],  outData[0], (VTK_TT *)outPtr, outExt, id) );
    default: 
      {
      vtkErrorMacro(<< "Execute: Unknown ScalarType\n");
      return;
      }
    }
}

