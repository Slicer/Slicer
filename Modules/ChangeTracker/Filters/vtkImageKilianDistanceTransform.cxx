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
  Module:    $RCSfile: vtkImageKilianDistanceTransform.cxx,v $
  Language:  C++
  Date:      $Date: 2004/11/10 00:11:52 $
  Version:   $Revision: 1.1 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImageKilianDistanceTransform.h"

#include "vtkImageData.h"
#include "vtkObjectFactory.h"

#include <math.h>

vtkCxxRevisionMacro(vtkImageKilianDistanceTransform, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkImageKilianDistanceTransform);

//----------------------------------------------------------------------------
// This defines the default values for the EDT parameters 
vtkImageKilianDistanceTransform::vtkImageKilianDistanceTransform()
{
  this->MaximumDistance = VTK_INT_MAX;
  this->Initialize = 1;
  this->ConsiderAnisotropy = 1;
  this->Algorithm = VTK_EMKILIAN_EDT_SAITO;
  this->InsideBoundaryZero = 0;
  this->ObjectValue = 0;
  this->DistanceFormat = VTK_EMKILIAN_EDT_EUCLIDEAN;
  this->SignedDistanceMap = 0;

}
//----------------------------------------------------------------------------
// This extent of the components changes to real and imaginary values.
void vtkImageKilianDistanceTransform::ExecuteInformation(vtkImageData *vtkNotUsed(input), 
                                                   vtkImageData *output)
{
  output->SetNumberOfScalarComponents(1);
  output->SetScalarType(VTK_FLOAT);
}


//----------------------------------------------------------------------------
// This method tells the superclass that the whole input array is needed
// to compute any output region.
void vtkImageKilianDistanceTransform::ComputeInputUpdateExtent(int inExt[6], 
                                                         int outExt[6])
{
  memcpy(inExt, outExt, 6 * sizeof(int));

  // Assumes that the input update extent has been initialized to output ...
  if ( this->GetInput() != NULL ) 
    {
    this->GetInput()->GetWholeExtent(inExt);
    } 
  else 
    {
    vtkErrorMacro( "Input is NULL" );
    }
}

//----------------------------------------------------------------------------
// This templated execute method handles any type input, but the output
// is always floats.
template <class TT>
void vtkImageKilianDistanceTransformCopyData(vtkImageKilianDistanceTransform *self,
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
  
  // Reorder axes
  self->PermuteExtent(outExt, outMin0,outMax0,outMin1,outMax1,outMin2,outMax2);
  self->PermuteIncrements(inData->GetIncrements(), inInc0, inInc1, inInc2);
  self->PermuteIncrements(outData->GetIncrements(), outInc0, outInc1, outInc2);
  
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
void vtkImageKilianDistanceTransformInitialize(vtkImageKilianDistanceTransform *self,
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
  
  if ( self->GetInitialize() == 1 ) 
    // Initialization required. Input image is only used as binary mask, 
    // so all non-zero values are set to maxDist
    //
    {      
    maxDist                = self->GetMaximumDistance();
    int objectValue        = self->GetObjectValue();
    int insideBoundaryZero = self->GetInsideBoundaryZero();
    int signedDistanceMap  = self->GetSignedDistanceMap();

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
        if( *inPtr0 != objectValue ) {
          // If we calculate signed distance map also calculate the distance outside the object
              // At the end of the iteration we will then turn the inside negative  
          if (signedDistanceMap) {
        if (insideBoundaryZero) *outPtr0 = maxDist;
        else {
          // if outside boundary should be set to zero we first have to find out if the voxel is a boundary or not => if not set to max distance  
          if (IsOutsideBoundary(idx0, outMin0,outMax0, inInc0, inPtr0, idx1, outMin1, outMax1, inInc1, idx2, outMin2, outMax2, inInc2, objectValue)) *outPtr0 = 0;
          else *outPtr0 = maxDist;
        }
          } else *outPtr0 = 0; // it is not a signed distance map -> everything outside is set to zero 
        }
        else {
        if (insideBoundaryZero && IsInsideBoundary(idx0, outMin0,outMax0, inInc0, inPtr0, idx1, outMin1, outMax1, inInc1, idx2, outMin2, outMax2, inInc2, objectValue)) *outPtr0 = 0;
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
    vtkImageKilianDistanceTransformCopyData( self, 
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
void vtkImageKilianDistanceTransformExecuteSaito(vtkImageKilianDistanceTransform *self,
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
  
  inSize0 = outMax0 - outMin0 + 1;  
  maxDist = self->GetMaximumDistance();

  buff= (float *)calloc(outMax0+1,sizeof(float));
    
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
    for (idx2 = outMin2; idx2 <= outMax2; ++idx2)
      {
      outPtr1 = outPtr2;
      for (idx1 = outMin1; idx1 <= outMax1; ++idx1)
        {
        outPtr0 = outPtr1;
        df= inSize0 ;
        for (idx0 = outMin0; idx0 <= outMax0; ++idx0)
          {
          if(*outPtr0 != 0)
            {
            df++ ;
            if(sq[df] < *outPtr0) {*outPtr0 = sq[df];}
            }
          else 
            {
            df=0;
            }
          outPtr0 += outInc0;
          }
              
        outPtr0 -= outInc0;
        df= inSize0 ;
        for (idx0 = outMax0; idx0 >= outMin0; --idx0)
          {
          if(*outPtr0 != 0)
            {
            df++ ;
            if(sq[df] < *outPtr0) {*outPtr0 = sq[df];}
            }
          else 
            {
            df=0;
            }
          outPtr0 -= outInc0;
          }
              
        outPtr1 += outInc1;
        }
      outPtr2 += outInc2;
      }      
    }
  else // next iterations are all identical. 
    {     
    outPtr2 = outPtr;
    for (idx2 = outMin2; idx2 <= outMax2; ++idx2)
      {
      outPtr1 = outPtr2;
      for (idx1 = outMin1; idx1 <= outMax1; ++idx1)
        {
        outPtr0 = outPtr1;
              
        // Buffer current values 
        for (idx0 = outMin0; idx0 <= outMax0; ++idx0)
          {
          buff[idx0]= *outPtr0;
          outPtr0 += outInc0;
          }
              
        // forward scan 
        a=0; buffer=buff[ outMin0 ];
        outPtr0 = outPtr1;
        outPtr0 += outInc0;
              
        for (idx0 = outMin0+1; idx0 <= outMax0; ++idx0)
          {
          if(a>0) {a--;}
          if(buff[idx0]>buffer+sq[1]) 
            {
            b=(int)(floor)((((buff[idx0]-buffer)/spacing)-1)/2); 
            if((idx0+b)>outMax0) {b=(outMax0)-idx0;}
                      
            for(n=a;n<=b;n++) 
              {
              m=buffer+sq[n+1];
              if(buff[idx0+n]<=m) {n=b;}
              else if(m<*(outPtr0+n*outInc0)) {*(outPtr0+n*outInc0)=m;}
              }
            a=b; 
            }
          else
            {
            a=0;
            }
                  
          buffer=buff[idx0];
          outPtr0 += outInc0;
          }
              
        outPtr0 -= 2*outInc0;
        a=0;
        buffer=buff[outMax0];
    
        for(idx0=outMax0-1;idx0>=outMin0; --idx0) 
          {
          if(a>0) {a--;}
          if(buff[idx0]>buffer+sq[1]) 
            {
            b=(int)(floor)((((buff[idx0]-buffer)/spacing)-1)/2); 
            if((idx0-b)<outMin0) {b=idx0-outMin0;}
                    
            for(n=a;n<=b;n++) 
              {
              m=buffer+sq[n+1];
              if(buff[idx0-n]<=m) {n=b;}
              else if(m<*(outPtr0-n*outInc0)) {*(outPtr0-n*outInc0)=m;}
              }
            a=b;  
            }
          else
            {
            a=0;
            }
          
          buffer=buff[idx0];
          outPtr0 -= outInc0;
          }
        outPtr1 += outInc1;
        }
      outPtr2 += outInc2;            
      } 
    }
  
  free(buff);
  free(sq);
}


//----------------------------------------------------------------------------
// Execute Saito's algorithm, modified for Cache Efficiency
//
void vtkImageKilianDistanceTransformExecuteSaitoCached(vtkImageKilianDistanceTransform *self,
                                           vtkImageData *outData, int outExt[6], float *outPtr )
{
  
  int outMin0, outMax0, outMin1, outMax1, outMin2, outMax2;
  vtkIdType outInc0, outInc1, outInc2;
  float *outPtr0, *outPtr1, *outPtr2;
  //
  int idx0, idx1, idx2, inSize0;
  
  float maxDist;
  
  float *sq;
  float *buff,*temp,buffer;
  int df,a,b,n;
  float m;
  
  float spacing;
  
  // Reorder axes (The outs here are just placeholdes
  self->PermuteExtent(outExt, outMin0,outMax0,outMin1,outMax1,outMin2,outMax2);
  self->PermuteIncrements(outData->GetIncrements(), outInc0, outInc1, outInc2);
  
  inSize0 = outMax0 - outMin0 + 1;  
  maxDist = self->GetMaximumDistance();

  buff= (float *)calloc(outMax0+1,sizeof(float));
  temp= (float *)calloc(outMax0+1,sizeof(float));
    
  // precompute sq[]. Anisotropy is handled here by using Spacing information
  
  sq = (float *)calloc(inSize0*2+2,sizeof(float));
  for(df=2*inSize0+1;df>inSize0;df--) sq[df]=maxDist;
  
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
    for (idx2 = outMin2; idx2 <= outMax2; ++idx2)
      {
      outPtr1 = outPtr2;
      for (idx1 = outMin1; idx1 <= outMax1; ++idx1)
        {
        outPtr0 = outPtr1;
        df= inSize0 ;
        // foreward 
        for (idx0 = outMin0; idx0 <= outMax0; ++idx0)
          {
          if(*outPtr0 != 0)
            {
            df++ ;
        // if df > inSize => look in buffer if last row has a lower value 
        // if df < insize => just calculate the distance to the boundary of the object in this row 
            if(sq[df] < *outPtr0) {*outPtr0 = sq[df];}
            }
          else 
            {
          // Reset the distance back to zero because we are outside the object  
            df=0;
            }
                  
          outPtr0 += outInc0;
          }
              
        outPtr0 -= outInc0;
        df= inSize0 ;
        // backwards
        for (idx0 = outMax0; idx0 >= outMin0; --idx0)
          {
          if(*outPtr0 != 0)
            {
            df++ ;
            if(sq[df] < *outPtr0) {*outPtr0 = sq[df];}
            }
          else 
            {
            df=0;
            }
          
          outPtr0 -= outInc0;
          }
              
        outPtr1 += outInc1;
        }
      outPtr2 += outInc2;
      }      
    }
  else // next iterations are all identical. 
    {     
    outPtr2 = outPtr;
    for (idx2 = outMin2; idx2 <= outMax2; ++idx2)
      {
      outPtr1 = outPtr2;
      for (idx1 = outMin1; idx1 <= outMax1; ++idx1)
        {
        // Buffer current values 
        outPtr0 = outPtr1;
        for (idx0 = outMin0; idx0 <= outMax0; ++idx0)
          {
          temp[idx0] = buff[idx0]= *outPtr0;
          outPtr0 += outInc0;
          }
              
        // forward scan - update neighbors 
        a=0; buffer=buff[ outMin0 ];
        outPtr0 = temp ;
        outPtr0 ++;
              
        for (idx0 = outMin0+1; idx0 <= outMax0; ++idx0)
          {
          if(a>0) {a--;}
          
          if(buff[idx0]>buffer+sq[1]) 
            {
            b=(int)(floor)((((buff[idx0]-buffer)/spacing)-1)/2); 
            if((idx0+b)>outMax0) {b=(outMax0)-idx0;}
                      
            for(n=a;n<=b;n++) 
              {
              m=buffer+sq[n+1];
              if(buff[idx0+n]<=m) {n=b;}  
              else if(m<*(outPtr0+n)) {*(outPtr0+n)=m;}
              }
            a=b; 
            }
          else
            {
            a=0;
            }
                  
          buffer=buff[idx0];
          outPtr0 ++;
          }
              
        // backward scan

        outPtr0 -= 2;
        a=0;
        buffer=buff[outMax0];
    
        for(idx0=outMax0-1;idx0>=outMin0; --idx0) 
          {
          if(a>0) {a--;}
          if(buff[idx0]>buffer+sq[1]) 
            {
            b=(int)(floor)((((buff[idx0]-buffer)/spacing)-1)/2); 
            if((idx0-b)<outMin0) b=idx0-outMin0;
                    
            for(n=a;n<=b;n++) 
              {
              m=buffer+sq[n+1];
              if(buff[idx0-n]<=m) {n=b;}
              else if(m<*(outPtr0-n)) {*(outPtr0-n)=m;}
              }
            a=b;  
            }
          else
            {
            a=0;
            }
          buffer=buff[idx0];
          outPtr0 --;
          }

        // Unbuffer current values 
        outPtr0 = outPtr1;
        for (idx0 = outMin0; idx0 <= outMax0; ++idx0)
          {
          *outPtr0 = temp[idx0];
          outPtr0 += outInc0;
          }

        outPtr1 += outInc1;
        }
      outPtr2 += outInc2;            
      } 
    }
  
  free(buff);
  free(temp);
  free(sq);
}

// Turns the outside negative 
template <class T> 
void vtkImageKilianDistanceTransformDefineSignedDistanceMap(vtkImageKilianDistanceTransform *self,
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

  int objectValue        = self->GetObjectValue();

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
      if ( (*inPtr0 != T(objectValue)) && (*outPtr0 > 0.0))  *outPtr0 = -*outPtr0;
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
void vtkImageKilianDistanceTransform::AllocateOutputScalars(vtkImageData *outData)
{
  outData->SetExtent(outData->GetWholeExtent());
  outData->AllocateScalars();
}
//----------------------------------------------------------------------------
// This method is passed input and output Datas, and executes the DistanceTransform
// algorithm to fill the output from the input.
void vtkImageKilianDistanceTransform::IterativeExecuteData(vtkImageData *inData,
                                                     vtkImageData *outData)
{
  void *inPtr;
  void *outPtr;
  vtkDebugMacro(<<"Executing image euclidean distance");
  
  int outExt[6];
  outData->GetWholeExtent( outExt );
  
  inPtr = inData->GetScalarPointerForExtent(inData->GetUpdateExtent());
  outPtr = outData->GetScalarPointer();
  
  // this filter expects that the output be floats.
  if (outData->GetScalarType() != VTK_FLOAT)
    {
    vtkErrorMacro(<< "Execute: Output must be be type float.");
    return;
    }
  
  // this filter expects input to have 1 components
  if (outData->GetNumberOfScalarComponents() != 1 )
    {
    vtkErrorMacro(<< "Execute: Cannot handle more than 1 components");
    return;
    }
  
  if ( this->GetIteration() == 0 )
    {
    switch (inData->GetScalarType())
      {
      vtkTemplateMacro6(vtkImageKilianDistanceTransformInitialize,
                        this, 
                        inData, (VTK_TT *)(inPtr), 
                        outData, outExt, (float *)(outPtr) );
      default:
        vtkErrorMacro(<< "Execute: Unknown ScalarType");
        return;
      } 
    } 
  else 
    { 
    if( inData != outData )
      switch (inData->GetScalarType())
        {    
        vtkTemplateMacro6(vtkImageKilianDistanceTransformCopyData,
                          this, 
                          inData, (VTK_TT *)(inPtr), 
                          outData, outExt, (float *)(outPtr) );
        }
    }
  

  // Call the specific algorithms. 
  switch( this->GetAlgorithm() ) 
    {
    case VTK_EMKILIAN_EDT_SAITO:
      vtkImageKilianDistanceTransformExecuteSaito( this, outData, outExt, (float *)(outPtr) );
      break;
    case VTK_EMKILIAN_EDT_SAITO_CACHED:
      vtkImageKilianDistanceTransformExecuteSaitoCached( this, outData, outExt, (float *)(outPtr) );
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown Algorithm");
    }
 
  // Runs through it three times - when it goes through the last time just go through here
  if ( this->GetIteration() == 2.0 ) {

    // Calculte Square root values 
    if (this->GetDistanceFormat() == VTK_EMKILIAN_EDT_SQUARE_ROOT) {
      int outMin0, outMax0, outMin1, outMax1, outMin2, outMax2;
      int outInc0, outInc1, outInc2;
      float *outPtr0, *outPtr1, *outPtr2;
      outMin0 = outExt[0]; outMax0 = outExt[1]; outMin1 = outExt[2]; outMax1 = outExt[3]; outMin2 = outExt[4] ; outMax2= outExt[5];
      outInc0 = outData->GetIncrements()[0]; outInc1 = outData->GetIncrements()[1]; outInc2 = outData->GetIncrements()[2];
      
      outPtr2 = (float*) outPtr;
      for (int idx2 = outMin2; idx2 <= outMax2; ++idx2) {
    outPtr1 = outPtr2;
    for (int idx1 = outMin1; idx1 <= outMax1; ++idx1) {
      outPtr0 = outPtr1;
      for (int idx0 = outMin0; idx0 <= outMax0; ++idx0) {
        if ( *outPtr0) *outPtr0 = float(sqrt((double)*outPtr0));
        outPtr0 += outInc0;
      }
      outPtr1 += outInc1;
    }
    outPtr2 += outInc2;
      }
    }
    if (this->GetSignedDistanceMap()) {
      // be carefull - we need intial input - vtkImageIteration changes it every time 
      vtkImageData *OriginalInData = this->IterationData[0];


      switch (OriginalInData->GetScalarType())
    {
      vtkTemplateMacro6(vtkImageKilianDistanceTransformDefineSignedDistanceMap,
                this, OriginalInData, (VTK_TT *)(OriginalInData->GetScalarPointerForExtent(OriginalInData->GetUpdateExtent())), outData, outExt, (float *)(outPtr));
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    } 
    }
  }

  this->UpdateProgress((this->GetIteration()+1.0)/3.0);
}


//----------------------------------------------------------------------------
// For streaming and threads.  Splits output update extent into num pieces.
// This method needs to be called num times.  Results must not overlap for
// consistent starting extent.  Subclass can override this method.
// This method returns the number of peices resulting from a successful split.
// This can be from 1 to "total".  
// If 1 is returned, the extent cannot be split.
int vtkImageKilianDistanceTransform::SplitExtent(int splitExt[6], int startExt[6], 
                             int num, int total)
{
  int splitAxis;
  int min, max;

  vtkDebugMacro("SplitExtent: ( " << startExt[0] << ", " << startExt[1] << ", "
                << startExt[2] << ", " << startExt[3] << ", "
                << startExt[4] << ", " << startExt[5] << "), " 
                << num << " of " << total);

  // start with same extent
  memcpy(splitExt, startExt, 6 * sizeof(int));

  splitAxis = 2;
  min = startExt[4];
  max = startExt[5];
  while ((splitAxis == this->Iteration) || (min == max))
    {
    splitAxis--;
    if (splitAxis < 0)
      { // cannot split
      vtkDebugMacro("  Cannot Split");
      return 1;
      }
    min = startExt[splitAxis*2];
    max = startExt[splitAxis*2+1];
    }

  // determine the actual number of pieces that will be generated
  if ((max - min + 1) < total)
    {
    total = max - min + 1;
    }
  
  if (num >= total)
    {
    vtkDebugMacro("  SplitRequest (" << num 
                  << ") larger than total: " << total);
    return total;
    }
  
  // determine the extent of the piece
  splitExt[splitAxis*2] = min + (max - min + 1)*num/total;
  if (num == total - 1)
    {
    splitExt[splitAxis*2+1] = max;
    }
  else
    {
    splitExt[splitAxis*2+1] = (min-1) + (max - min + 1)*(num+1)/total;
    }
  
  vtkDebugMacro("  Split Piece: ( " <<splitExt[0]<< ", " <<splitExt[1]<< ", "
                << splitExt[2] << ", " << splitExt[3] << ", "
                << splitExt[4] << ", " << splitExt[5] << ")");
  fflush(stderr);

  return total;
}

void vtkImageKilianDistanceTransform::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Initialize: " 
     << (this->Initialize ? "On\n" : "Off\n");
  
  os << indent << "Consider Anisotropy: " 
     << (this->ConsiderAnisotropy ? "On\n" : "Off\n");
  
  os << indent << "Initialize: " << this->Initialize << "\n";
  os << indent << "Maximum Distance: " << this->MaximumDistance << "\n";

  os << indent << "Algorithm: ";
  if ( this->Algorithm == VTK_EMKILIAN_EDT_SAITO )
    {
    os << "Saito\n";
    }
  else 
    {
    os << "Saito Cached\n";
    }

  os << indent << "ObjectValue :" << this->ObjectValue<<"\n";
  os << indent << "Zero Boundary Value:" << (this->InsideBoundaryZero ? "Inside\n" : "Outside\n");
  os << indent << "DistanceFormat:" << (this->DistanceFormat == VTK_EMKILIAN_EDT_SQUARE_ROOT? "Square Root\n" : "Euclidean\n");
  os << indent << "SignedDistanceMap:" << (this->SignedDistanceMap ? "On\n" : "Off\n");

}
  

















