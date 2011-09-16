/*=auto=========================================================================

(c) Copyright 2001 Massachusetts Institute of Technology 

Permission is hereby granted, without payment, to copy, modify, display 
and distribute this software and its documentation, if any, for any purpose, 
provided that the above copyright notice and the following three paragraphs 
appear on all copies of this software.  Use of this software constitutes 
acceptance of these terms and conditions.

IN NO EVENT SHALL MIT BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, 
INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE 
AND ITS DOCUMENTATION, EVEN IF MIT HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

MIT SPECIFICALLY DISCLAIMS ANY EXPRESS OR IMPLIED WARRANTIES INCLUDING, 
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR 
A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.

THE SOFTWARE IS PROVIDED "AS IS."  MIT HAS NO OBLIGATION TO PROVIDE 
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================auto=*/
// I had to alter the original 
/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageLabelPropagation.cxx,v $
  Language:  C++
  Date:      $Date: 2007/06/22 06:37:16 $
  Version:   $Revision: 1.1 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImageLabelPropagation.h"

#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include <math.h>

vtkCxxRevisionMacro(vtkImageLabelPropagation, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkImageLabelPropagation);

//----------------------------------------------------------------------------
// This defines the default values for the EDT parameters 
vtkImageLabelPropagation::vtkImageLabelPropagation()
{
  this->MaximumDistance = 5000;
  this->Initialize = 1;
  this->ConsiderAnisotropy = 1;
  this->PropagatedMap = vtkImageData::New();
}

//----------------------------------------------------------------------------
vtkImageLabelPropagation::~vtkImageLabelPropagation() { 
  this->PropagatedMap->Delete();
}

//----------------------------------------------------------------------------
// This extent of the components changes to real and imaginary values.
int vtkImageLabelPropagation::IterativeRequestInformation(
  vtkInformation* vtkNotUsed(input), vtkInformation* output)
{
  vtkDataObject::SetPointDataActiveScalarInfo(output, VTK_FLOAT, 1);
  return 1;
}

//----------------------------------------------------------------------------
// This method tells the superclass that the whole input array is needed
// to compute any output region.
int vtkImageLabelPropagation::IterativeRequestUpdateExtent(
  vtkInformation* input, vtkInformation* vtkNotUsed(output) )
{
  int *wExt = input->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT());
  input->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),wExt,6);

  return 1;
}

//----------------------------------------------------------------------------
// This templated execute method handles any type input, but the output
// is always floats.
template <class TT>
void vtkImageLabelPropagationCopyData(vtkImageLabelPropagation *self,
                                      vtkImageData *inData, TT *inPtr,
                                      vtkImageData *outData, int outExt[6], 
                                       float *outPtr )
{

  vtkIdType inInc0, inInc1, inInc2;
  TT *inPtr0, *inPtr1, *inPtr2;

  int outMin0, outMax0, outMin1, outMax1, outMin2, outMax2;
  vtkIdType outInc0, outInc1, outInc2;
  float *outPtr0, *outPtr1, *outPtr2;
  
  int idx0, idx1, idx2;
  
  // Reorder axes - I do not know what this is worse since we are running the same way through it 
  // I think  below it should be different 
  self->PermuteExtent(outExt, outMin0,outMax0,outMin1,outMax1,outMin2,outMax2);
  self->PermuteIncrements(inData->GetIncrements(), inInc0, inInc1, inInc2);
  self->PermuteIncrements(outData->GetIncrements(), outInc0, outInc1, outInc2);


  // Temporary results - for propagation
  // vtkImageData *PropagatedMapCopy = vtkImageData::New();
  //   PropagatedMapCopy->DeepCopy(self->GetPropagatedMap());

  inPtr2 = inPtr;
  outPtr2 = outPtr;
  for (idx2 = outMin2; idx2 <= outMax2; ++idx2)
    {
    inPtr1 = inPtr2;
    outPtr1 = outPtr2;
    for (idx1 = outMin1; idx1 <= outMax1; ++idx1)
      {
      inPtr0 = inPtr1;
      outPtr0 = outPtr1;

      for (idx0 = outMin0; idx0 <= outMax0; ++idx0)
        {
        *outPtr0 = *inPtr0 ;
        inPtr0 += inInc0;
        outPtr0 += outInc0;
        }
      inPtr1 += inInc1;
      outPtr1 += outInc1;
      }
    inPtr2 += inInc2;
    outPtr2 += outInc2;
    }
}

// Checks if the voxel is inside boundary or not 
template <class T>
int IsInsideBoundary(int idx0, int outMin0, int outMax0, int inInc0, T* inPtr0, int idx1, int outMin1, int outMax1, int inInc1, int idx2, int outMin2, int outMax2, int inInc2, int objectLabel) {
  // Could leave this line away but just leave it for completness 
  if (*inPtr0 != T(objectLabel)) return 0;
  // X Axis
  if ((idx0 > outMin0) && (*(inPtr0-inInc0) != T(objectLabel))) return 1;
  if ((idx0 < outMax0) && (*(inPtr0+inInc0) != T(objectLabel))) return 1;

  // Y Axis
  if ((idx1 > outMin1) && (*(inPtr0-inInc1) != T(objectLabel))) return 1;
  if ((idx1 < outMax1) && (*(inPtr0+inInc1) != T(objectLabel))) return 1;

  // Z Axis  
  if ((idx2 > outMin2) && (*(inPtr0-inInc2) != T(objectLabel))) return 1;
  if ((idx2 < outMax2) && (*(inPtr0+inInc2) != T(objectLabel))) return 1;
  
  // It is not at the boundary
  return 0;
}

// Checks if the voxel is inside boundary or not 
template <class T>
int IsOutsideBoundary(int idx0, int outMin0, int outMax0, int inInc0, T* inPtr0, int idx1, int outMin1, int outMax1, int inInc1, int idx2, int outMin2, int outMax2, int inInc2, int objectLabel) {
  // Could leave this line away but just leave it for completness 
  if (*inPtr0 == T(objectLabel)) return 0;
  // X Axis
  if ((idx0 > outMin0) && (*(inPtr0-inInc0) == T(objectLabel))) return 1;
  if ((idx0 < outMax0) && (*(inPtr0+inInc0) == T(objectLabel))) return 1;

  // Y Axis
  if ((idx1 > outMin1) && (*(inPtr0-inInc1) == T(objectLabel))) return 1;
  if ((idx1 < outMax1) && (*(inPtr0+inInc1) == T(objectLabel))) return 1;

  // Z Axis  
  if ((idx2 > outMin2) && (*(inPtr0-inInc2) == T(objectLabel))) return 1;
  if ((idx2 < outMax2) && (*(inPtr0+inInc2) == T(objectLabel))) return 1;
  
  // It is not at the boundary
  return 0;
}

//----------------------------------------------------------------------------
// This templated execute method handles any type input, but the output
// is always floats.
template <class T>
void vtkImageLabelPropagationInitialize(vtkImageLabelPropagation *self,
                                         vtkImageData *inData, T *inPtr,
                                         vtkImageData *outData, 
                                         int outExt[6], float *outPtr )
{
  vtkIdType inInc0, inInc1, inInc2;
  T *inPtr0, *inPtr1, *inPtr2;

  int outMin0, outMax0, outMin1, outMax1, outMin2, outMax2;
  vtkIdType outInc0, outInc1, outInc2;
  float *outPtr0, *outPtr1, *outPtr2;
  
  int idx0, idx1, idx2;
  float maxDist;
  
  // Reorder axes
  self->PermuteExtent(outExt, outMin0,outMax0,outMin1,outMax1,outMin2,outMax2);
  self->PermuteIncrements(inData->GetIncrements(), inInc0, inInc1, inInc2);
  self->PermuteIncrements(outData->GetIncrements(), outInc0, outInc1, outInc2);

  int insideBoundaryZero = 1;
  int signedDistanceMap = 1;
  if ( self->GetInitialize() == 1 ) 
    // Initialization required. Input image is only used as binary mask, 
    // so all non-zero values are set to maxDist
    //
    {      
      self->GetPropagatedMap()->DeepCopy(inData);
      maxDist                = self->GetMaximumDistance();

    inPtr2 = inPtr;
    outPtr2 = outPtr;
    for (idx2 = outMin2; idx2 <= outMax2; ++idx2)
      {
      inPtr1 = inPtr2;
      outPtr1 = outPtr2;
      for (idx1 = outMin1; idx1 <= outMax1; ++idx1)
        {
        inPtr0 = inPtr1;
        outPtr0 = outPtr1;
              
        for (idx0 = outMin0; idx0 <= outMax0; ++idx0)
          {
        if( !*inPtr0  ) {
          // If we calculate signed distance map also calculate the distance outside the object
              // At the end of the iteration we will then turn the inside negative  
          if (signedDistanceMap) {
            if (insideBoundaryZero) *outPtr0 = maxDist;
        else {
          // if outside boundary should be set to zero we first have to find out if the voxel is a boundary or not => if not set to max distance  
          if (IsOutsideBoundary(idx0, outMin0,outMax0, inInc0, inPtr0, idx1, outMin1, outMax1, inInc1, idx2, outMin2, outMax2, inInc2,short(*inPtr0)   )) *outPtr0 = 0;
          else *outPtr0 = maxDist;
        }
          } else *outPtr0 = 0; // it is not a signed distance map -> everything outside is set to zero 
        }
        else {
          // cout << *inPtr0 << " " ;

          // if (insideBoundaryZero && IsInsideBoundary(idx0, outMin0,outMax0, inInc0, inPtr0, idx1, outMin1, outMax1, inInc1, idx2, outMin2, outMax2, inInc2, objectValue)) *outPtr0 = 0;
        if (insideBoundaryZero && IsInsideBoundary(idx0, outMin0,outMax0, inInc0, inPtr0, idx1, outMin1, outMax1, inInc1, idx2, outMin2, outMax2, inInc2, short(*inPtr0))) *outPtr0 = 0;
        else *outPtr0 = maxDist;
      }
          inPtr0 += inInc0;
          outPtr0 += outInc0;
          }
              
        inPtr1 += inInc1;
        outPtr1 += outInc1;
        }
      inPtr2 += inInc2;
      outPtr2 += outInc2;
      }
    }
  else   
    // No initialization required. We just copy inData to outData.
    {
    vtkImageLabelPropagationCopyData( self, 
                                       inData, (T *)(inPtr), 
                                       outData, outExt, (float *)(outPtr) );
    }
}

//----------------------------------------------------------------------------
// Execute Saito's algorithm.
//
// T. Saito and J.I. Toriwaki. New algorithms for Euclidean distance 
// transformations of an n-dimensional digitised picture with applications.
// Pattern Recognition, 27(11). pp. 1551--1565, 1994. 
// 
// Notations stay as close as possible to those used in the paper.
//
template <class T> 
void vtkImageLabelPropagationExecuteSaito(vtkImageLabelPropagation *self,vtkImageData *PropagatedMap, T* proPtr,
                                          vtkImageData *outData, int outExt[6], float *outPtr )
{ 
  
  int outMin0, outMax0, outMin1, outMax1, outMin2, outMax2;
  vtkIdType outInc0, outInc1, outInc2;
  float *outPtr0, *outPtr1, *outPtr2;
  int idx0, idx1, idx2, inSize0;
  float maxDist;
  float *sq;
  float *buff,buffer;
  int df,a,b,n;
  float m;
  float spacing;
  
  // Reorder axes (The outs here are just placeholdes
  self->PermuteExtent(outExt, outMin0,outMax0,outMin1,outMax1,outMin2,outMax2);
  self->PermuteIncrements(outData->GetIncrements(), outInc0, outInc1, outInc2);

  vtkIdType proInc0, proInc1, proInc2;
  self->PermuteIncrements(PropagatedMap->GetIncrements(), proInc0, proInc1, proInc2);

  T *proPtr0,*proPtr1,*proPtr2;
  T proLabel;

  inSize0 = outMax0 - outMin0 + 1;  
  maxDist = self->GetMaximumDistance();

  buff= (float *)calloc(outMax0+1,sizeof(float));
  T* label_buff= (T *)calloc(outMax0+1,sizeof(T));
  T  label_buffer;
    
  // precompute sq[]. Anisotropy is handled here by using Spacing information
  
  sq = (float *)calloc(inSize0*2+2,sizeof(float));
  for(df=2*inSize0+1;df>inSize0;df--)
    {
    sq[df]=maxDist;
    }
  
  if ( self->GetConsiderAnisotropy() )
    {
    spacing = outData->GetSpacing()[ self->GetIteration() ];
    }
  else
    {
    spacing = 1;
    }
  
  spacing*=spacing;
  
  for(df=inSize0;df>=0;df--) 
    {
    sq[df]=df*df*spacing;
    }
  
  if ( self->GetIteration() == 0 ) 
    {
    outPtr2 = outPtr;
    proPtr2 = proPtr;

    for (idx2 = outMin2; idx2 <= outMax2; ++idx2)
      {
      outPtr1 = outPtr2;
      proPtr1 = proPtr2;
      for (idx1 = outMin1; idx1 <= outMax1; ++idx1)
        {
        outPtr0 = outPtr1;
    proPtr0 = proPtr1;
        df= inSize0 ;
        for (idx0 = outMin0; idx0 <= outMax0; ++idx0)
          {
          if(*outPtr0 != 0)
            {
            df++ ;
            if(sq[df] < *outPtr0) {
          *outPtr0 = sq[df];
              *proPtr0 = proLabel;
            }
            }
          else 
            {
            df=0;
        proLabel = *proPtr0;
        // df is an index for the distance counted from this label 
            }
          outPtr0 += outInc0;
      proPtr0 += proInc0;
      
          }

    // Go backwards
              
        outPtr0 -= outInc0;
    proPtr0 -= proInc0;

        df= inSize0 ;
        for (idx0 = outMax0; idx0 >= outMin0; --idx0)
          {
          if(*outPtr0 != 0)
            {
            df++ ;
            if(sq[df] < *outPtr0) {
          *outPtr0 = sq[df];
          *proPtr0 = proLabel;
        }
            }
          else 
            {
            df=0;
        proLabel = *proPtr0;
            }
          outPtr0 -= outInc0;
          proPtr0 -= proInc0;
          }
              
        outPtr1 += outInc1;
    proPtr1 += proInc1;
        }
      outPtr2 += outInc2;
      proPtr2 += proInc2;
      }      
    }
  else // next iterations are all identical. 
    {     
    outPtr2 = outPtr;
    proPtr2 = proPtr;
    for (idx2 = outMin2; idx2 <= outMax2; ++idx2)
      {
      outPtr1 = outPtr2;
      proPtr1 = proPtr2;
      for (idx1 = outMin1; idx1 <= outMax1; ++idx1)
        {
        outPtr0 = outPtr1;
        proPtr0 = proPtr1;
      
        // Buffer current values 
        for (idx0 = outMin0; idx0 <= outMax0; ++idx0)
          {
          buff[idx0]= *outPtr0;
          label_buff[idx0] = *proPtr0; 
          outPtr0 += outInc0;
          proPtr0 += proInc0;
          }
              
        // forward scan 
        a=0; 
        buffer=buff[ outMin0 ];

    label_buffer=label_buff[ outMin0 ];
        
        outPtr0 = outPtr1;
    proPtr0 = proPtr1;
        outPtr0 += outInc0;
        proPtr0 += proInc0;
              
        for (idx0 = outMin0+1; idx0 <= outMax0; ++idx0)
          {
          if(a>0) {a--;}
          if(buff[idx0]>buffer+sq[1]) 
            {
            b=(int)(floor)((((buff[idx0]-buffer)/spacing)-1)/2); 
            if((idx0+b)>outMax0) {b=(outMax0)-idx0;}
                      
            for(n=a;n<=b;n++) 
              {
          // m = distance from current voxel - where the current voxel is defined as idx0 -1
              m=buffer+sq[n+1];
              if(buff[idx0+n]<=m) {n=b;}
              else if(m<*(outPtr0+n*outInc0)) { 
        *(outPtr0+n*outInc0) = m;
            *(proPtr0+n*proInc0) = label_buffer  ;
          }
              }
            a=b; 
            }
          else
            {
            a=0;
            }
                  
          buffer=buff[idx0];
          label_buffer=label_buff[idx0];
          outPtr0 += outInc0;
      proPtr0 += proInc0;
          }
              
        outPtr0 -= 2*outInc0;
        proPtr0 -= 2*proInc0;
        a=0;
        buffer=buff[outMax0];
        label_buffer=label_buff[outMax0];
    
        for(idx0=outMax0-1;idx0>=outMin0; --idx0) 
          {
          if(a>0) {a--;}
          if(buff[idx0]>buffer+sq[1]) 
            {
            b=(int)(floor)((((buff[idx0]-buffer)/spacing)-1)/2); 
            if((idx0-b)<outMin0) {b=idx0-outMin0;}
                    
            for(n=a;n<=b;n++) 
              {
        // m = distance from current voxel - where the current voxel is defined as idx0 +1
              m=buffer+sq[n+1];
              if(buff[idx0-n]<=m) {n=b;}
              else if(m<*(outPtr0-n*outInc0)) {
        *(outPtr0-n*outInc0)= m;
        *(proPtr0-n*proInc0)= label_buffer;
          }
              }
            a=b;  
            }
          else
            {
            a=0;
            }
          
          buffer=buff[idx0];
          label_buffer=label_buff[idx0];
          outPtr0 -= outInc0;
      proPtr0 -= proInc0;
          }
        outPtr1 += outInc1;
        proPtr1 += proInc1;
        }
      outPtr2 += outInc2;            
      proPtr2 += proInc2;            
      } 
    }
  
  free(buff);
  free(label_buff);
  free(sq);
}


// Turns the outside negative 
template <class T> 
void vtkImageLabelPropagationDefineSignedDistanceMap(vtkImageLabelPropagation *self,
                                vtkImageData *inData, T *inPtr,
                                vtkImageData *outData, 
                                int outExt[6], float *outPtr )
{
  int inInc0, inInc1, inInc2;
  T *inPtr0, *inPtr1, *inPtr2;
  int outMin0, outMax0, outMin1, outMax1, outMin2, outMax2;
  int outInc0, outInc1, outInc2;
  float *outPtr0, *outPtr1, *outPtr2;
  outMin0 = outExt[0]; outMax0 = outExt[1]; outMin1 = outExt[2]; outMax1 = outExt[3]; outMin2 = outExt[4] ; outMax2= outExt[5];
  outInc0 = outData->GetIncrements()[0]; outInc1 = outData->GetIncrements()[1]; outInc2 = outData->GetIncrements()[2];
  inInc0  = inData->GetIncrements()[0];  inInc1  = inData->GetIncrements()[1];  inInc2  = inData->GetIncrements()[2];

  inPtr2 = inPtr;
  outPtr2 = outPtr;

  for (int idx2 = outMin2; idx2 <= outMax2; ++idx2) {
    inPtr1 = inPtr2;
    outPtr1 = outPtr2;
    for (int idx1 = outMin1; idx1 <= outMax1; ++idx1)
      {
    inPtr0 = inPtr1;
    outPtr0 = outPtr1;
    
    for (int idx0 = outMin0; idx0 <= outMax0; ++idx0) {
      if ( (!*inPtr0) && (*outPtr0 > 0.0))  *outPtr0 = -*outPtr0;
      inPtr0 += inInc0;
      outPtr0 += outInc0;
    }
    inPtr1 += inInc1;
    outPtr1 += outInc1;
      }
    inPtr2 += inInc2;
    outPtr2 += outInc2;
  }
}

//----------------------------------------------------------------------------
void vtkImageLabelPropagation::AllocateOutputScalars(vtkImageData *outData)
{
  outData->SetExtent(outData->GetWholeExtent());
  outData->AllocateScalars();
}
//----------------------------------------------------------------------------
// This method is passed input and output Datas, and executes the DistanceTransform
// algorithm to fill the output from the input.
int vtkImageLabelPropagation::IterativeRequestData( vtkInformation* vtkNotUsed( request ),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{

  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkImageData *inData = vtkImageData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkImageData *outData = vtkImageData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  this->AllocateOutputScalars(outData);

  void *inPtr;
  void *outPtr;
  vtkDebugMacro(<<"Executing Label Probagation");
  
  int outExt[6];
  outData->GetWholeExtent( outExt );
  
  inPtr = inData->GetScalarPointerForExtent(inData->GetUpdateExtent());
  outPtr = outData->GetScalarPointer();

  // this filter expects that the output be floats.
  if (outData->GetScalarType() != VTK_FLOAT)
    {
    vtkErrorMacro(<< "Execute: Output must be be type float.");
    return 1;
    }
  
  // this filter expects input to have 1 components
  if (outData->GetNumberOfScalarComponents() != 1 )
    {
    vtkErrorMacro(<< "Execute: Cannot handle more than 1 components");
    return 1;
    }
  
  if ( this->GetIteration() == 0 )
    {
    switch (inData->GetScalarType())
      {
      vtkTemplateMacro6(vtkImageLabelPropagationInitialize,
                        this, 
                        inData, (VTK_TT *)(inPtr), 
                        outData, outExt, (float *)(outPtr) );
      default:
        vtkErrorMacro(<< "Execute: Unknown ScalarType");
        return 1;
      } 
     
    } 
  else 
    { 
    if( inData != outData )
      switch (inData->GetScalarType())
        {    
        vtkTemplateMacro6(vtkImageLabelPropagationCopyData,
                          this, 
                          inData, (VTK_TT *)(inPtr), 
                          outData, outExt, (float *)(outPtr) );
        }
    }
  
  // Call the specific algorithms. 
  void *proPtr = this->PropagatedMap->GetScalarPointerForExtent(this->PropagatedMap->GetUpdateExtent());

  switch (this->PropagatedMap->GetScalarType())
  {
    vtkTemplateMacro(vtkImageLabelPropagationExecuteSaito( this, this->PropagatedMap, (VTK_TT *) proPtr, outData, outExt, (float *)(outPtr)));
    default:
    vtkErrorMacro(<< "Execute: Unknown ScalarType");
    return 1;
  } 

 
  // Runs through it three times - when it goes through the last time just go through here
  if ( this->GetIteration() == 2.0 ) {

    // if (this->GetSignedDistanceMap()) {
    if (1) {
      // be carefull - we need intial input - vtkImageIteration changes it every time 
      vtkImageData *OriginalInData = this->IterationData[0];


      switch (OriginalInData->GetScalarType())
    {
      vtkTemplateMacro6(vtkImageLabelPropagationDefineSignedDistanceMap,
                this, OriginalInData, (VTK_TT *)(OriginalInData->GetScalarPointerForExtent(OriginalInData->GetUpdateExtent())), outData, outExt, (float *)(outPtr));
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return 1;
    } 
    }
  }

  this->UpdateProgress((this->GetIteration()+1.0)/3.0);
  return 1;
}

void vtkImageLabelPropagation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Initialize: " 
     << (this->Initialize ? "On\n" : "Off\n");
  os << indent << "Initialize: " << this->Initialize << "\n";
  os << indent << "Maximum Distance: " << this->MaximumDistance << "\n";
  os << indent << "Consider Anisotropy: " 
     << (this->ConsiderAnisotropy ? "On\n" : "Off\n");  
}
  

















