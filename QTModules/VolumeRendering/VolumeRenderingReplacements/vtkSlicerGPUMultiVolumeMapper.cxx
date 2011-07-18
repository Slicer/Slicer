/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicerGPUMultiVolumeMapper.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSlicerGPUMultiVolumeMapper.h"
#include "vtkSlicerVolumeRenderingFactory.h"

#include "vtkDataArray.h"
#include "vtkMultiThreader.h"
#include "vtkRenderer.h"
#include "vtkVolume.h"
#include "vtkCamera.h"
#include "vtkMath.h"
#include "vtkPointData.h"
#include "vtkImageData.h"
#include "vtkColorTransferFunction.h"
#include "vtkPiecewiseFunction.h"
#include "vtkVolumeProperty.h"
#include "vtkMatrix4x4.h"
#include "vtkCommand.h"
#include "vtkExecutive.h"

vtkCxxRevisionMacro(vtkSlicerGPUMultiVolumeMapper, "$Revision: 1.6 $");

//----------------------------------------------------------------------------
// Needed when we don't use the vtkStandardNewMacro.
vtkInstantiatorNewMacro(vtkSlicerGPUMultiVolumeMapper);
//----------------------------------------------------------------------------

//
//  1 component three input volumes (A, B, and LabelMap):
//             R            G             B             A
//  volume 1   scalar.A     scalar.B      Label              
//  

template <class T>
void vtkSlicerGPUMultiVolumeMapperComputeScalars( T *dataPtr,
                                                T *dataPtr1,
                                                T *dataPtr2,
                                               vtkSlicerGPUMultiVolumeMapper *me,
                                               float offset, float scale, float offset1, float scale1,
                                               unsigned char *volume1)
{
  T              *inPtr;
  T              *inPtr1;
  T              *inPtr2;
  unsigned char  *outPtr;
  int             i, j, k;
  int             idx;

  int   inputDimensions[3];
  double inputSpacing[3];
  me->GetNthInput(0)->GetDimensions( inputDimensions );
  me->GetNthInput(0)->GetSpacing( inputSpacing );

  int   outputDimensions[3];
  float outputSpacing[3];
  me->GetVolumeDimensions( outputDimensions );
  me->GetVolumeSpacing( outputSpacing );

  double wx, wy, wz;
  double fx, fy, fz;
  int x, y, z;

  double sampleRate[3];
  sampleRate[0] = (double)outputSpacing[0] / (double)inputSpacing[0];
  sampleRate[1] = (double)outputSpacing[1] / (double)inputSpacing[1];
  sampleRate[2] = (double)outputSpacing[2] / (double)inputSpacing[2];

  // This is the case where no interpolation is needed
  if ( inputDimensions[0] == outputDimensions[0] &&
       inputDimensions[1] == outputDimensions[1] &&
       inputDimensions[2] == outputDimensions[2] )
  {
    int size = outputDimensions[0] * outputDimensions[1] * outputDimensions[2];

    inPtr = dataPtr;
    inPtr1 = dataPtr1;
    inPtr2 = dataPtr2;

    {
      outPtr = volume1;
      for ( i = 0; i < size; i++ )
      {
        //0
        idx = static_cast<unsigned char>((*(inPtr++) + offset) * scale);
        *(outPtr++) = idx;
        
        //1
        if (inPtr1)
        {
            idx = static_cast<unsigned char>((*(inPtr1++) + offset1) * scale1);
            *(outPtr++) = idx;
        }
        else
            *(outPtr++) = 0;
        
        //2    
        if (inPtr2)
        {
            idx = static_cast<unsigned char>((*(inPtr2++) + offset) * scale);
            *(outPtr++) = idx;
        }
        else
            *(outPtr++) = 0;
        
        //3    
        *(outPtr++) = 0;
       } 
     }  
  }
  // The sizes are different and interpolation is required
  else
  {
    outPtr  = volume1;
 
    for ( k = 0; k < outputDimensions[2]; k++ )
      {
      fz = k * sampleRate[2];
      fz = (fz >= inputDimensions[2]-1)?(inputDimensions[2]-1.001):(fz);
      z  = vtkMath::Floor( fz );
      wz = fz - z;
      for ( j = 0; j < outputDimensions[1]; j++ )
        {
        fy = j * sampleRate[1];
        fy = (fy >= inputDimensions[1]-1)?(inputDimensions[1]-1.001):(fy);
        y  = vtkMath::Floor( fy );
        wy = fy - y;
        for ( i = 0; i < outputDimensions[0]; i++ )
          {
          fx = i * sampleRate[0];
          fx = (fx >= inputDimensions[0]-1)?(inputDimensions[0]-1.001):(fx);
          x  = vtkMath::Floor( fx );
          wx = fx - x;

          inPtr = dataPtr + ( z*inputDimensions[0]*inputDimensions[1] + y*inputDimensions[0] + x );
          inPtr1 = dataPtr1;
          if (inPtr1)
            inPtr1 += z*inputDimensions[0]*inputDimensions[1] + y*inputDimensions[0] + x;
          inPtr2 = dataPtr2;
          if (inPtr2)
            inPtr2 += z*inputDimensions[0]*inputDimensions[1] + y*inputDimensions[0] + x;
          
          {
            float A, B, C, D, E, F, G, H;
            A = static_cast<float>(*(inPtr));
            B = static_cast<float>(*(inPtr+1));
            C = static_cast<float>(*(inPtr+inputDimensions[0]));
            D = static_cast<float>(*(inPtr+inputDimensions[0]+1));
            E = static_cast<float>(*(inPtr+inputDimensions[0]*inputDimensions[1]));
            F = static_cast<float>(*(inPtr+inputDimensions[0]*inputDimensions[1]+1));
            G = static_cast<float>(*(inPtr+inputDimensions[0]*inputDimensions[1]+inputDimensions[0]));
            H = static_cast<float>(*(inPtr+inputDimensions[0]*inputDimensions[1]+inputDimensions[0]+1));

            float val = 
              (1.0-wx)*(1.0-wy)*(1.0-wz)*A +
              (    wx)*(1.0-wy)*(1.0-wz)*B +
              (1.0-wx)*(    wy)*(1.0-wz)*C +
              (    wx)*(    wy)*(1.0-wz)*D +
              (1.0-wx)*(1.0-wy)*(    wz)*E +
              (    wx)*(1.0-wy)*(    wz)*F +
              (1.0-wx)*(    wy)*(    wz)*G +
              (    wx)*(    wy)*(    wz)*H;

            idx = static_cast<unsigned char>((val + offset)*scale);
            *(outPtr++) = idx;
            
            if (inPtr1)
            {
                A = static_cast<float>(*(inPtr1));
                B = static_cast<float>(*(inPtr1+1));
                C = static_cast<float>(*(inPtr1+inputDimensions[0]));
                D = static_cast<float>(*(inPtr1+inputDimensions[0]+1));
                E = static_cast<float>(*(inPtr1+inputDimensions[0]*inputDimensions[1]));
                F = static_cast<float>(*(inPtr1+inputDimensions[0]*inputDimensions[1]+1));
                G = static_cast<float>(*(inPtr1+inputDimensions[0]*inputDimensions[1]+inputDimensions[0]));
                H = static_cast<float>(*(inPtr1+inputDimensions[0]*inputDimensions[1]+inputDimensions[0]+1));

                val = 
                  (1.0-wx)*(1.0-wy)*(1.0-wz)*A +
                  (    wx)*(1.0-wy)*(1.0-wz)*B +
                  (1.0-wx)*(    wy)*(1.0-wz)*C +
                  (    wx)*(    wy)*(1.0-wz)*D +
                  (1.0-wx)*(1.0-wy)*(    wz)*E +
                  (    wx)*(1.0-wy)*(    wz)*F +
                  (1.0-wx)*(    wy)*(    wz)*G +
                  (    wx)*(    wy)*(    wz)*H;

                idx = static_cast<unsigned char>((val + offset1)*scale1);
                *(outPtr++) = idx;
            }
            else
                *(outPtr++) = 0;
            
            if (inPtr2)
            {            
                A = static_cast<float>(*(inPtr1));
                B = static_cast<float>(*(inPtr1+1));
                C = static_cast<float>(*(inPtr1+inputDimensions[0]));
                D = static_cast<float>(*(inPtr1+inputDimensions[0]+1));
                E = static_cast<float>(*(inPtr1+inputDimensions[0]*inputDimensions[1]));
                F = static_cast<float>(*(inPtr1+inputDimensions[0]*inputDimensions[1]+1));
                G = static_cast<float>(*(inPtr1+inputDimensions[0]*inputDimensions[1]+inputDimensions[0]));
                H = static_cast<float>(*(inPtr1+inputDimensions[0]*inputDimensions[1]+inputDimensions[0]+1));

                val = 
                  (1.0-wx)*(1.0-wy)*(1.0-wz)*A +
                  (    wx)*(1.0-wy)*(1.0-wz)*B +
                  (1.0-wx)*(    wy)*(1.0-wz)*C +
                  (    wx)*(    wy)*(1.0-wz)*D +
                  (1.0-wx)*(1.0-wy)*(    wz)*E +
                  (    wx)*(1.0-wy)*(    wz)*F +
                  (1.0-wx)*(    wy)*(    wz)*G +
                  (    wx)*(    wy)*(    wz)*H;

                idx = static_cast<unsigned char>((val + offset)*scale);
                *(outPtr++) = idx;
            }
            else
                *(outPtr++) = 0;
                
            *(outPtr++) = 0;
          }
        }
      }
    }
  }
}

VTK_THREAD_RETURN_TYPE vtkSlicerGPUMultiVolumeMapperComputeGradients( void *arg)
{
  GPUGradientsArgsType *pArgs = (GPUGradientsArgsType *)(((vtkMultiThreader::ThreadInfo *)arg)->UserData);
  
  float *dataPtr = pArgs->dataPtr;
  vtkSlicerGPUMultiVolumeMapper *me = pArgs->me;
  double scalarRange[2];
  scalarRange[0] = pArgs->scalarRange[0];
  scalarRange[1] = pArgs->scalarRange[1];
  unsigned char *volume = pArgs->volume;
  
  int                 x, y, z;
  int                 offset, outputOffset;
  int                 x_start, x_limit;
  int                 y_start, y_limit;
  int                 z_start, z_limit;
  float               *dptr;
  float               n[3], t;
  float               gvalue;
  float               zeroNormalThreshold;
  int                 xlow, xhigh;
  double              aspect[3];
  unsigned char       *outPtr;
  unsigned char       *normals;
  double              floc[3];
  int                 loc[3];

  float outputSpacing[3];
  me->GetVolumeSpacing( outputSpacing );

  double spacing[3];
  me->GetInput()->GetSpacing( spacing );

  double sampleRate[3];
  sampleRate[0] = (double)outputSpacing[0] / (double)spacing[0];
  sampleRate[1] = (double)outputSpacing[1] / (double)spacing[1];
  sampleRate[2] = (double)outputSpacing[2] / (double)spacing[2];
 
  int components = me->GetInput()->GetNumberOfScalarComponents();
 
  int dim[3];
  me->GetInput()->GetDimensions(dim);

  int outputDim[3];
  me->GetVolumeDimensions( outputDim );

  double avgSpacing = ((double)spacing[0] +
                       (double)spacing[1] + 
                       (double)spacing[2]) / 3.0;

  // adjust the aspect
  aspect[0] = (double)spacing[0] * 2.0 / avgSpacing;
  aspect[1] = (double)spacing[1] * 2.0 / avgSpacing;
  aspect[2] = (double)spacing[2] * 2.0 / avgSpacing;
  
  //avoid division inside for loop
  aspect[0] = 1.0/aspect[0];
  aspect[1] = 1.0/aspect[1];
  aspect[2] = 1.0/aspect[2];
  
  float               scale;
  scale = 255.0 / (0.25*(scalarRange[1] - scalarRange[0]));

  // Get the length at or below which normals are considered to
  // be "zero"
  zeroNormalThreshold =.001 * (scalarRange[1] - scalarRange[0]);

  int thread_id    = ((vtkMultiThreader::ThreadInfo *)(arg))->ThreadID;
  int thread_count = ((vtkMultiThreader::ThreadInfo *)(arg))->NumberOfThreads;
  
  x_start = 0;
  x_limit = outputDim[0];
  y_start = 0;
  y_limit = outputDim[1];
  z_start = (int)(( (float)thread_id / (float)thread_count ) *
                  outputDim[2] );
  z_limit = (int)(( (float)(thread_id + 1) / (float)thread_count ) *
                  outputDim[2] );

  // Do final error checking on limits - make sure they are all within bounds
  // of the scalar input

  x_start = (x_start<0)?(0):(x_start);
  y_start = (y_start<0)?(0):(y_start);
  z_start = (z_start<0)?(0):(z_start);

  x_limit = (x_limit>dim[0])?(outputDim[0]):(x_limit);
  y_limit = (y_limit>dim[1])?(outputDim[1]):(y_limit);
  z_limit = (z_limit>dim[2])?(outputDim[2]):(z_limit);

  normals = volume;
  
  double wx, wy, wz;

  // Loop through all the data and compute the encoded normal and
  // gradient magnitude for each scalar location
  for ( z = z_start; z < z_limit; z++ )
  {
    floc[2] = z*sampleRate[2];
    floc[2] = (floc[2]>=(dim[2]-1))?(dim[2]-1.001):(floc[2]);
    loc[2]  = vtkMath::Floor(floc[2]);
    wz = floc[2] - loc[2];

    for ( y = y_start; y < y_limit; y++ )
    {
      floc[1] = y*sampleRate[1];
      floc[1] = (floc[1]>=(dim[1]-1))?(dim[1]-1.001):(floc[1]);
      loc[1]  = vtkMath::Floor(floc[1]);
      wy = floc[1] - loc[1];

      xlow = x_start;
      xhigh = x_limit;
      outputOffset = z * outputDim[0] * outputDim[1] + y * outputDim[0] + xlow;

      // Set some pointers
      outPtr = normals + 4*outputOffset;

      for ( x = xlow; x < xhigh; x++ )
      {
        floc[0] = x*sampleRate[0];
        floc[0] = (floc[0]>=(dim[0]-1))?(dim[0]-1.001):(floc[0]);
        loc[0]  = vtkMath::Floor(floc[0]);
        wx = floc[0] - loc[0];

        offset = loc[2] * dim[0] * dim[1] + loc[1] * dim[0] + loc[0];

        dptr = dataPtr + components*offset + components - 1;

        // Use a central difference method if possible,
        // otherwise use a forward or backward difference if
        // we are on the edge
        int sampleOffset[6];
        sampleOffset[0] = (loc[0]<1)        ?(0):(-components);
        sampleOffset[1] = (loc[0]>=dim[0]-2)?(0):( components);
        sampleOffset[2] = (loc[1]<1)        ?(0):(-components*dim[0]);
        sampleOffset[3] = (loc[1]>=dim[1]-2)?(0):( components*dim[0]);
        sampleOffset[4] = (loc[2]<1)        ?(0):(-components*dim[0]*dim[1]);
        sampleOffset[5] = (loc[2]>=dim[2]-2)?(0):( components*dim[0]*dim[1]);

        float sample[6];
        for ( int i = 0; i < 6; i++ )
        {
          float A, B, C, D, E, F, G, H;
          float *samplePtr = dptr + sampleOffset[i];

          A = static_cast<float>(*(samplePtr));
          B = static_cast<float>(*(samplePtr+components));
          C = static_cast<float>(*(samplePtr+components*dim[0]));
          D = static_cast<float>(*(samplePtr+components*dim[0]+components));
          E = static_cast<float>(*(samplePtr+components*dim[0]*dim[1]));
          F = static_cast<float>(*(samplePtr+components*dim[0]*dim[1]+components));
          G = static_cast<float>(*(samplePtr+components*dim[0]*dim[1]+components*dim[0]));
          H = static_cast<float>(*(samplePtr+components*dim[0]*dim[1]+components*dim[0]+components));

          sample[i] = 
            (1.0-wx)*(1.0-wy)*(1.0-wz)*A +
            (    wx)*(1.0-wy)*(1.0-wz)*B +
            (1.0-wx)*(    wy)*(1.0-wz)*C +
            (    wx)*(    wy)*(1.0-wz)*D +
            (1.0-wx)*(1.0-wy)*(    wz)*E +
            (    wx)*(1.0-wy)*(    wz)*F +
            (1.0-wx)*(    wy)*(    wz)*G +
            (    wx)*(    wy)*(    wz)*H;
        }

        n[0] = ((sampleOffset[0]==0 || sampleOffset[1]==0)?(2.0):(1.0))*(sample[0] -sample[1]);
        n[1] = ((sampleOffset[2]==0 || sampleOffset[3]==0)?(2.0):(1.0))*(sample[2] -sample[3]);
        n[2] = ((sampleOffset[4]==0 || sampleOffset[5]==0)?(2.0):(1.0))*(sample[4] -sample[5]);

        // Take care of the aspect ratio of the data
        // Scaling in the vtkVolume is isotropic, so this is the
        // only place we have to worry about non-isotropic scaling.
        n[0] *= aspect[0];
        n[1] *= aspect[1];
        n[2] *= aspect[2];

        // Compute the gradient magnitude
        t = sqrt( (double)( n[0]*n[0] + 
                            n[1]*n[1] + 
                            n[2]*n[2] ) );

        // Encode this into an 4 bit value 
        gvalue = t * scale; 

        gvalue = (gvalue<0.0)?(0.0):(gvalue);
        gvalue = (gvalue>255.0)?(255.0):(gvalue);

        *(outPtr+3) = static_cast<unsigned char>(gvalue + 0.5);

        // Normalize the gradient direction
        if ( t > zeroNormalThreshold )
        {
          float t_rev = 1.0/t;
          n[0] *= t_rev;
          n[1] *= t_rev;
          n[2] *= t_rev;
        }
        else
        {
          n[0] = n[1] = n[2] = 0.0;
        }

        int nx = static_cast<int>((n[0] * 0.5 + 0.5)*255.0 + 0.5);
        int ny = static_cast<int>((n[1] * 0.5 + 0.5)*255.0 + 0.5);
        int nz = static_cast<int>((n[2] * 0.5 + 0.5)*255.0 + 0.5);

        nx = (nx<0)?(0):(nx);
        ny = (ny<0)?(0):(ny);
        nz = (nz<0)?(0):(nz);

        nx = (nx>255)?(255):(nx);
        ny = (ny>255)?(255):(ny);
        nz = (nz>255)?(255):(nz);

        *(outPtr  ) = nx;
        *(outPtr+1) = ny;
        *(outPtr+2) = nz;

        outPtr += 4;
      }
    }
    if ( z%8 == 7 )
    {
      float args[1];
      args[0] = static_cast<float>(z - z_start) / static_cast<float>(z_limit - z_start - 1);

      if (thread_id == 0)
        me->InvokeEvent( vtkCommand::VolumeMapperComputeGradientsProgressEvent, args );
    }
  }

  {
    float args[1] = {1.0f};

    if (thread_id == 0)
      me->InvokeEvent( vtkCommand::VolumeMapperComputeGradientsProgressEvent, args );
  }

  return VTK_THREAD_RETURN_VALUE;
}

vtkSlicerGPUMultiVolumeMapper::vtkSlicerGPUMultiVolumeMapper()
{
  // The input used when creating the textures
  this->SavedTextureInput             = NULL;
  this->SavedTextureInput2nd          = NULL;
  
  this->SavedRGBFunction              = NULL;
  this->SavedGrayFunction             = NULL;
  this->SavedScalarOpacityFunction    = NULL;
  this->SavedColorChannels            = 0;
  this->SavedScalarOpacityDistance    = 0;
  
  this->SavedRGBFunction2nd              = NULL;
  this->SavedGrayFunction2nd             = NULL;
  this->SavedScalarOpacityFunction2nd    = NULL;
  this->SavedColorChannels2nd            = 0;
  this->SavedScalarOpacityDistance2nd    = 0;
  
  this->Volume1                       = NULL;
  this->Volume2                       = NULL;
  this->Volume3                       = NULL;
  
  this->VolumeSize                    = 0;
  
  this->Framerate                    = 5.0f;
  
  this->Threader               = vtkMultiThreader::New();
  
  this->GradientsArgs                = NULL;
}

vtkSlicerGPUMultiVolumeMapper::~vtkSlicerGPUMultiVolumeMapper()
{
  if (this->Volume1)
  {
    delete [] this->Volume1;
    this->Volume1 = NULL;
  }

  if (this->Volume2)
  {
    delete [] this->Volume2;
    this->Volume2 = NULL;
  }
  
  if (this->Volume3)
  {
    delete [] this->Volume3;
    this->Volume3 = NULL;
  }
        
  if (this->GradientsArgs)
  {
    delete this->GradientsArgs;
    this->GradientsArgs = NULL;
  }
        
  if (this->Threader)
  {
    this->Threader->Delete();
    this->Threader = NULL;
  }
}

vtkSlicerGPUMultiVolumeMapper *vtkSlicerGPUMultiVolumeMapper::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkSlicerVolumeRenderingFactory::CreateInstance("vtkSlicerGPUMultiVolumeMapper");
  return (vtkSlicerGPUMultiVolumeMapper*)ret;
}

int vtkSlicerGPUMultiVolumeMapper::UpdateVolumes(vtkVolume *vtkNotUsed(vol))
{
  int needToUpdate = 0;

  // Get the image data
  vtkImageData *input = this->GetNthInput(0);
  input->Update();
  
  vtkImageData *input1 = this->GetNthInput(1);
  unsigned long input1MTime = this->SavedTextureMTime2nd.GetMTime();
  if (input1)
  {
    input1->Update();
    input1MTime = input1->GetMTime();
  }
  
  // Has the volume changed in some way?
  if ( this->SavedTextureInput != input || this->SavedTextureMTime.GetMTime() < input->GetMTime() ||
       this->SavedTextureInput2nd != input1 || this->SavedTextureMTime2nd.GetMTime() < input1MTime )
    {
    needToUpdate = 1;
    }
 
  if ( !needToUpdate )
    {
    return 0;
    }
 
  this->SavedTextureInput = input;
  this->SavedTextureMTime.Modified();
  
  this->SavedTextureInput2nd = input1;
  this->SavedTextureMTime2nd.Modified();
  
  // How big does the Volume need to be?
  int dim[3];
  input->GetDimensions(dim);

  int powerOfTwoDim[3];
  input->GetDimensions(powerOfTwoDim);
  
  //non-power-of-two texture offically supported since OpenGL 2.0
  while ( ! this->IsTextureSizeSupported( powerOfTwoDim ) )
  {
    if ( powerOfTwoDim[0] >= powerOfTwoDim[1] &&
          powerOfTwoDim[0] >= powerOfTwoDim[2] )
    {
      powerOfTwoDim[0]--;
    }
    else if ( powerOfTwoDim[1] >= powerOfTwoDim[0] &&
              powerOfTwoDim[1] >= powerOfTwoDim[2] )
    {
      powerOfTwoDim[1]--;
    }
    else
    {
      powerOfTwoDim[2]--;
    }
  }
  
  int neededSize = powerOfTwoDim[0] * powerOfTwoDim[1] * powerOfTwoDim[2];
 
  // What is the spacing?
  double spacing[3];
  input->GetSpacing(spacing);
 
  // allocate memory
  {
    if (this->Volume1)
        delete [] this->Volume1;
//    if (this->Volume2)
//        delete [] this->Volume2;
//    if (this->Volume3)
//        delete [] this->Volume3;
        
    this->Volume1 = new unsigned char [4*neededSize];
//    this->Volume2 = new unsigned char [4*neededSize];
//    this->Volume3 = new unsigned char [4*neededSize];
    
    this->VolumeSize       = neededSize;
  }
 
  // Find the scalar range
  double scalarRange[2];
  double scalarRange1[2];
//  double scalarRange2[2];
  
  input->GetPointData()->GetScalars()->GetRange(scalarRange, 0);
  
  if (input1)
    input1->GetPointData()->GetScalars()->GetRange(scalarRange1, 0);
  else
    input->GetPointData()->GetScalars()->GetRange(scalarRange1, 0);
  
  // Is the difference between max and min less than 4096? If so, and if
  // the data is not of float or double type, use a simple offset mapping.
  // If the difference between max and min is 4096 or greater, or the data
  // is of type float or double, we must use an offset / scaling mapping.
  // In this case, the array size will be 4096 - we need to figure out the 
  // offset and scale factor.
  float offset, offset1;
  float scale, scale1;
 
  int arraySizeNeeded, arraySizeNeeded1;
 
  int scalarType = input->GetScalarType();

  if ( scalarType == VTK_FLOAT ||
       scalarType == VTK_DOUBLE ||
       scalarRange[1] - scalarRange[0] > 255 )
    {
    arraySizeNeeded = 256;
    offset          = -scalarRange[0];
    scale           = 255.0 / (scalarRange[1] - scalarRange[0]);
    }
  else
    {
    arraySizeNeeded = (int)(scalarRange[1] - scalarRange[0] + 1);
    offset          = -scalarRange[0]; 
    scale           = 1.0;
   }

  arraySizeNeeded1 = arraySizeNeeded;
  offset1 = offset;
  scale1 = scale;
  
  if (input1)
  {
    scalarType = input1->GetScalarType();

    if ( scalarType == VTK_FLOAT ||
        scalarType == VTK_DOUBLE ||
        scalarRange1[1] - scalarRange1[0] > 255 )
     {
     arraySizeNeeded1 = 256;
     offset1          = -scalarRange1[0];
     scale1           = 255.0 / (scalarRange1[1] - scalarRange1[0]);
     }
    else
     {
     arraySizeNeeded1 = (int)(scalarRange1[1] - scalarRange1[0] + 1);
     offset1          = -scalarRange1[0]; 
     scale1           = 1.0;
    } 
  }
  
  this->ScalarOffset = offset;
  this->ScalarScale = scale;
  
  this->ColorTableSize = arraySizeNeeded;

  this->ScalarOffset2nd = offset1;
  this->ScalarScale2nd = scale1;
  
  this->ColorTableSize2nd = arraySizeNeeded1;
  
  // Save the volume size
  this->VolumeDimensions[0] = powerOfTwoDim[0];
  this->VolumeDimensions[1] = powerOfTwoDim[1];
  this->VolumeDimensions[2] = powerOfTwoDim[2];
 
  // Compute the new spacing
  this->VolumeSpacing[0] = 
    (static_cast<double>(dim[0])-1.01)*(double)spacing[0] / static_cast<double>(this->VolumeDimensions[0]-1);
  this->VolumeSpacing[1] = 
    (static_cast<double>(dim[1])-1.01)*(double)spacing[1] / static_cast<double>(this->VolumeDimensions[1]-1);
  this->VolumeSpacing[2] = 
    (static_cast<double>(dim[2])-1.01)*(double)spacing[2] / static_cast<double>(this->VolumeDimensions[2]-1);

  // Transfer the input volume to the RGBA volume
  void *dataPtr = input->GetScalarPointer();
  void *dataPtr1 = NULL;
  void *dataPtr2 = NULL;
  
  if (input1)
     dataPtr1 = input1->GetScalarPointer();
     
  switch ( scalarType )
    {
    vtkTemplateMacro(
      vtkSlicerGPUMultiVolumeMapperComputeScalars(
        (VTK_TT *)(dataPtr), (VTK_TT*)(dataPtr1), (VTK_TT*)(dataPtr2),
        this, offset, scale, offset1, scale1,
        this->Volume1));
    }
/*
  int dataPtrSize = dim[0]*dim[1]*dim[2];
  float* floatDataPtr = new float[dataPtrSize];  
  
  //copy out scalar as float array for multithreading
  //or maybe later multithreading with template...
  CopyToFloatBuffer(input, floatDataPtr, dataPtrSize);
  
  if (this->GradientsArgs)
        delete this->GradientsArgs;
  
  this->GradientsArgs = new GPUGradientsArgsType;
  
  this->GradientsArgs->dataPtr = floatDataPtr;
  this->GradientsArgs->me = this;
  this->GradientsArgs->scalarRange[0] = scalarRange[0];
  this->GradientsArgs->scalarRange[1] = scalarRange[1];
  this->GradientsArgs->volume = this->Volume2;
  
  this->Threader->SetSingleMethod( vtkSlicerGPUMultiVolumeMapperComputeGradients, (void *)(this->GradientsArgs) );
  this->Threader->SingleMethodExecute();

  delete [] floatDataPtr;
  floatDataPtr = new float[dataPtrSize];
  
  //handle second volume
  if (input1)
  {
    CopyToFloatBuffer(input1, floatDataPtr, dataPtrSize);
    this->GradientsArgs->scalarRange[0] = scalarRange1[0];
    this->GradientsArgs->scalarRange[1] = scalarRange1[1];
    this->GradientsArgs->volume = this->Volume3;
    
    this->Threader->SetSingleMethod( vtkSlicerGPUMultiVolumeMapperComputeGradients, (void *)(this->GradientsArgs) );
    this->Threader->SingleMethodExecute();
  }
  
  delete [] floatDataPtr;
*/  
  return 1;
}

void vtkSlicerGPUMultiVolumeMapper::CopyToFloatBuffer(vtkImageData* input, float* floatDataPtr, int dataPtrSize)
{
  int scalarType = input->GetScalarType();
  
  switch(scalarType)
  {
  case VTK_SIGNED_CHAR:
  case VTK_CHAR:
    {
      char* tempDataPtr = (char*)input->GetScalarPointer();
      for (int i = 0; i < dataPtrSize; i++)
        floatDataPtr[i] = (float)tempDataPtr[i];
    }
    break;
  case VTK_UNSIGNED_CHAR:
    {
      unsigned char* tempDataPtr = (unsigned char*)input->GetScalarPointer();
      for (int i = 0; i < dataPtrSize; i++)
        floatDataPtr[i] = (float)tempDataPtr[i];
    }
    break;
  case VTK_SHORT:
    {
      short* tempDataPtr = (short*)input->GetScalarPointer();
      for (int i = 0; i < dataPtrSize; i++)
        floatDataPtr[i] = (float)tempDataPtr[i];
    }    
    break;
  case VTK_UNSIGNED_SHORT:
    {
      unsigned short* tempDataPtr = (unsigned short*)input->GetScalarPointer();
      for (int i = 0; i < dataPtrSize; i++)
        floatDataPtr[i] = (float)tempDataPtr[i];
    }    
    break;
  case VTK_INT:
    {
      int* tempDataPtr = (int*)input->GetScalarPointer();
      for (int i = 0; i < dataPtrSize; i++)
        floatDataPtr[i] = (float)tempDataPtr[i];
    }    
    break;
  case VTK_UNSIGNED_INT:
    {
      unsigned int* tempDataPtr = (unsigned int*)input->GetScalarPointer();
      for (int i = 0; i < dataPtrSize; i++)
        floatDataPtr[i] = (float)tempDataPtr[i];
    }    
    break;
  case VTK_DOUBLE:
    {
      double* tempDataPtr = (double*)input->GetScalarPointer();
      for (int i = 0; i < dataPtrSize; i++)
        floatDataPtr[i] = (float)tempDataPtr[i];
    }    
    break;
  case VTK_FLOAT:
    {
      float* tempDataPtr = (float*)input->GetScalarPointer();
      for (int i = 0; i < dataPtrSize; i++)
        floatDataPtr[i] = (float)tempDataPtr[i];
    }    
    break;
  case VTK_LONG:
    {
      long* tempDataPtr = (long*)input->GetScalarPointer();
      for (int i = 0; i < dataPtrSize; i++)
        floatDataPtr[i] = (float)tempDataPtr[i];
    }    
    break;
  }
}

int vtkSlicerGPUMultiVolumeMapper::UpdateColorLookup( vtkVolume *vol )
{
  int needToUpdate = 0;

  // Get the image data
  vtkImageData *input = this->GetNthInput(0);
  input->Update();
  
  vtkImageData *input1 = this->GetNthInput(1);
  unsigned long input1MTime = this->SavedTextureMTime2nd.GetMTime();
  if (input1)
  {
    input1->Update();
    input1MTime = input1->GetMTime();
  }
  
  // Has the volume changed in some way?
  if ( this->SavedTextureInput != input || this->SavedColorOpacityMTime.GetMTime() < input->GetMTime() ||
       this->SavedTextureInput2nd != input1 || this->SavedColorOpacityMTime2nd.GetMTime() < input1MTime )
  {
    needToUpdate = 1;
  }
  
  vtkColorTransferFunction *rgbFunc  = NULL;
  vtkPiecewiseFunction     *grayFunc = NULL;
  
  vtkColorTransferFunction *rgbFunc1  = NULL;
  vtkPiecewiseFunction     *grayFunc1 = NULL;
 
  // How many color channels for this component?
  int colorChannels = vol->GetProperty()->GetColorChannels(0);
  int colorChannels1 = vol->GetProperty()->GetColorChannels(1);

  // Has the number of color channels changed?
  if ( this->SavedColorChannels != colorChannels )
  {
    needToUpdate = 1;
  }

  // Has the color transfer function changed in some way,
  // and we are using it?
  if ( colorChannels == 3 )
  {
    rgbFunc  = vol->GetProperty()->GetRGBTransferFunction(0);
    if ( this->SavedRGBFunction != rgbFunc ||
         this->SavedColorOpacityMTime.GetMTime() < rgbFunc->GetMTime() )
    {
      needToUpdate = 1;
    }
  }

  // Has the gray transfer function changed in some way,
  // and we are using it?
  if ( colorChannels == 1 )
  {
    grayFunc = vol->GetProperty()->GetGrayTransferFunction(0);
    if ( this->SavedGrayFunction != grayFunc ||
         this->SavedColorOpacityMTime.GetMTime() < grayFunc->GetMTime() )
    {
      needToUpdate = 1;
    }
  }
  
  // Has the number of color channels changed?
  if ( this->SavedColorChannels2nd != colorChannels1 )
  {
    needToUpdate = 1;
  }

  // Has the color transfer function changed in some way,
  // and we are using it?
  if ( colorChannels1 == 3 )
  {
    rgbFunc1  = vol->GetProperty()->GetRGBTransferFunction(1);
    unsigned long mtime = this->SavedColorOpacityMTime2nd.GetMTime();
    if (rgbFunc1)
        mtime = rgbFunc1->GetMTime();
        
    if ( this->SavedRGBFunction2nd != rgbFunc1 || this->SavedColorOpacityMTime2nd.GetMTime() < mtime )
    {
      needToUpdate = 1;
    }
  }

  // Has the gray transfer function changed in some way,
  // and we are using it?
  if ( colorChannels1 == 1 )
  {
    grayFunc1 = vol->GetProperty()->GetGrayTransferFunction(1);
    unsigned long mtime = this->SavedColorOpacityMTime2nd.GetMTime();
    if (grayFunc1)
        mtime = grayFunc1->GetMTime();
        
    if ( this->SavedGrayFunction2nd != grayFunc1 || this->SavedColorOpacityMTime2nd.GetMTime() < mtime )
    {
      needToUpdate = 1;
    }
  }
    
  // Has the scalar opacity transfer function changed in some way?
  vtkPiecewiseFunction *scalarOpacityFunc = vol->GetProperty()->GetScalarOpacity(0);
  if ( this->SavedScalarOpacityFunction != scalarOpacityFunc ||
       this->SavedColorOpacityMTime.GetMTime() < scalarOpacityFunc->GetMTime() )
    {
      needToUpdate = 1;
    }
  
  vtkPiecewiseFunction *scalarOpacityFunc1 = vol->GetProperty()->GetScalarOpacity(1);
  {
    unsigned long mtime = this->SavedColorOpacityMTime2nd.GetMTime();
    if (scalarOpacityFunc1)
        mtime = scalarOpacityFunc1->GetMTime();
        
    if ( this->SavedScalarOpacityFunction2nd != scalarOpacityFunc1 || this->SavedColorOpacityMTime2nd.GetMTime() < mtime )
    {
      needToUpdate = 1;
    }
  }
    
  // Has the gradient opacity transfer function changed in some way?
  vtkPiecewiseFunction *gradientOpacityFunc = vol->GetProperty()->GetGradientOpacity(0);
  if ( this->SavedGradientOpacityFunction != gradientOpacityFunc ||
       this->SavedColorOpacityMTime.GetMTime() < gradientOpacityFunc->GetMTime() )
    {
    needToUpdate = 1;
    }

  vtkPiecewiseFunction *gradientOpacityFunc1 = vol->GetProperty()->GetGradientOpacity(1);
  {
    unsigned long mtime = this->SavedColorOpacityMTime2nd.GetMTime();
    if (scalarOpacityFunc1)
        mtime = gradientOpacityFunc1->GetMTime();
    if ( this->SavedGradientOpacityFunction2nd != gradientOpacityFunc1 || this->SavedColorOpacityMTime2nd.GetMTime() < mtime )
    {
      needToUpdate = 1;
    }
  }
    
  double scalarOpacityDistance = vol->GetProperty()->GetScalarOpacityUnitDistance(0);
  if ( this->SavedScalarOpacityDistance != scalarOpacityDistance )
    {
    needToUpdate = 1;
    }
  
  double scalarOpacityDistance1 = vol->GetProperty()->GetScalarOpacityUnitDistance(1);
  if ( this->SavedScalarOpacityDistance2nd != scalarOpacityDistance1 )
    {
    needToUpdate = 1;
    }
    
  // If we have not found any need to update, return now
  if ( !needToUpdate )
    {
    return 0;
    }
  
  this->SavedRGBFunction             = rgbFunc;
  this->SavedGrayFunction            = grayFunc;
  this->SavedScalarOpacityFunction   = scalarOpacityFunc;
  this->SavedGradientOpacityFunction = gradientOpacityFunc;
  this->SavedColorChannels           = colorChannels;
  this->SavedScalarOpacityDistance   = scalarOpacityDistance;
  
  this->SavedColorOpacityMTime.Modified();
  
  this->SavedRGBFunction2nd             = rgbFunc1;
  this->SavedGrayFunction2nd            = grayFunc1;
  this->SavedScalarOpacityFunction2nd   = scalarOpacityFunc1;
  this->SavedGradientOpacityFunction2nd = gradientOpacityFunc1;
  this->SavedColorChannels2nd           = colorChannels1;
  this->SavedScalarOpacityDistance2nd   = scalarOpacityDistance1;
  
  this->SavedColorOpacityMTime2nd.Modified();

  // Find the scalar range
  double scalarRange[2];
  input->GetPointData()->GetScalars()->GetRange(scalarRange, 0);
  
  int arraySizeNeeded = this->ColorTableSize;
  int arraySizeNeeded1 = this->ColorTableSize2nd;

  // Sample the transfer functions between the min and max.
  if ( colorChannels == 1 )
  {
    grayFunc->GetTable( scalarRange[0], scalarRange[1], arraySizeNeeded, this->TempArray1 );
  }
  else
  {
    rgbFunc->GetTable( scalarRange[0], scalarRange[1], arraySizeNeeded, this->TempArray1 );
  }
  
  scalarOpacityFunc->GetTable( scalarRange[0], scalarRange[1], arraySizeNeeded, this->TempArray2 );

  float goArray[256];
  gradientOpacityFunc->GetTable( 0, (scalarRange[1] - scalarRange[0])*0.25, 256, goArray );
                                 
// Find the scalar range
  double scalarRange1[2];
  if (input1)
    input1->GetPointData()->GetScalars()->GetRange(scalarRange1, 0);
  else
    input->GetPointData()->GetScalars()->GetRange(scalarRange1, 0);

  memset(this->TempArray11, 0, sizeof(float)*3*4096);
  memset(this->TempArray21, 0, sizeof(float)*4096);
  
  // Sample the transfer functions between the min and max.
  if ( colorChannels1 == 1 )
  {
    if (grayFunc1)
        grayFunc1->GetTable( scalarRange1[0], scalarRange1[1], arraySizeNeeded1, this->TempArray11 );      
  }
  else
  {
    if (rgbFunc1)
        rgbFunc1->GetTable( scalarRange1[0], scalarRange1[1], arraySizeNeeded1, this->TempArray11 );
  }
  
  if (scalarOpacityFunc1 && input1)
    scalarOpacityFunc1->GetTable( scalarRange1[0], scalarRange1[1], arraySizeNeeded1, this->TempArray21 );

  float goArray1[256];
  memset(goArray1, 0, sizeof(float)*256);
  if (gradientOpacityFunc1)
    gradientOpacityFunc1->GetTable( 0, (scalarRange1[1] - scalarRange1[0])*0.25, 256, goArray1 );

 /*
 <-  2048 color lookup table ->
 ^
 |
 256 gradient opacity lookup table
 |
 v
 */
  int goLoop;
  unsigned char *ptr;
  float *fptr1, *fptr2;
  
  unsigned char *ptr1;
  float *fptr11, *fptr21;
  
  int i;
  
  // Move the two temp float arrays into one RGBA unsigned char array
  ptr = this->ColorLookup;
  for ( goLoop = 0; goLoop < 256; goLoop++ )
  {
    fptr1 = this->TempArray1;
    fptr2 = this->TempArray2;
    if ( colorChannels == 1 )
    {
      for ( i = 0; i < arraySizeNeeded; i++ )
      {
        *(ptr++) = static_cast<unsigned char>(*(fptr1)*255.0 + 0.5);
        *(ptr++) = static_cast<unsigned char>(*(fptr1)*255.0 + 0.5);
        *(ptr++) = static_cast<unsigned char>(*(fptr1++)*255.0 + 0.5);
        *(ptr++) = static_cast<unsigned char>(*(fptr2++)*goArray[goLoop]*255.0 + 0.5);
      }
    }
    else
    {
      for ( i = 0; i < arraySizeNeeded; i++ )
      {
        *(ptr++) = static_cast<unsigned char>(*(fptr1++)*255.0 + 0.5);
        *(ptr++) = static_cast<unsigned char>(*(fptr1++)*255.0 + 0.5);
        *(ptr++) = static_cast<unsigned char>(*(fptr1++)*255.0 + 0.5);
        *(ptr++) = static_cast<unsigned char>(*(fptr2++)*goArray[goLoop]*255.0 + 0.5);
      }
    }

    for ( ; i < 256; i++ )
    {
      *(ptr++) = 0;
      *(ptr++) = 0;
      *(ptr++) = 0;
      *(ptr++) = 0;
    }
  }
  
  // Move the two temp float arrays into one RGBA unsigned char array
  ptr1 = this->ColorLookup2nd;
  for ( goLoop = 0; goLoop < 256; goLoop++ )
  {
    fptr11 = this->TempArray11;
    fptr21 = this->TempArray21;
    if ( colorChannels1 == 1 )
    {
      for ( i = 0; i < arraySizeNeeded; i++ )
      {
        *(ptr1++) = static_cast<unsigned char>(*(fptr11)*255.0 + 0.5);
        *(ptr1++) = static_cast<unsigned char>(*(fptr11)*255.0 + 0.5);
        *(ptr1++) = static_cast<unsigned char>(*(fptr11++)*255.0 + 0.5);
        *(ptr1++) = static_cast<unsigned char>(*(fptr21++)*goArray1[goLoop]*255.0 + 0.5);
      }
    }
    else
    {
      for ( i = 0; i < arraySizeNeeded; i++ )
      {
        *(ptr1++) = static_cast<unsigned char>(*(fptr11++)*255.0 + 0.5);
        *(ptr1++) = static_cast<unsigned char>(*(fptr11++)*255.0 + 0.5);
        *(ptr1++) = static_cast<unsigned char>(*(fptr11++)*255.0 + 0.5);
        *(ptr1++) = static_cast<unsigned char>(*(fptr21++)*goArray1[goLoop]*255.0 + 0.5);
      }
    }

    for ( ; i < 256; i++ )
    {
      *(ptr1++) = 0;
      *(ptr1++) = 0;
      *(ptr1++) = 0;
      *(ptr1++) = 0;
    }
  }
  return 1;
}


// Print the vtkSlicerGPUMultiVolumeMapper
void vtkSlicerGPUMultiVolumeMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "VolumeDimensions: " << this->VolumeDimensions[0] << " "
     << this->VolumeDimensions[1] << " " << this->VolumeDimensions[2] << endl;
  os << indent << "VolumeSpacing: " << this->VolumeSpacing[0] << " "
     << this->VolumeSpacing[1] << " " << this->VolumeSpacing[2] << endl;
}

void vtkSlicerGPUMultiVolumeMapper::SetNthInput( int index, vtkDataSet *genericInput)
{
  vtkImageData *input = vtkImageData::SafeDownCast( genericInput );
  
  if ( input )
    {
    this->SetNthInput( index, input);
    }
  else
    {
    vtkErrorMacro("The SetInput method of this mapper requires vtkImageData as input");
    }
}

void vtkSlicerGPUMultiVolumeMapper::SetNthInput( int index, vtkImageData *input )
{ 
  if (this->GetNumberOfInputPorts() < index + 1)
    this->SetNumberOfInputPorts(index + 1);
  
  if(input)
    this->SetInputConnection(index, input->GetProducerPort());
  else
    this->SetInputConnection(index, 0);
}

vtkImageData *vtkSlicerGPUMultiVolumeMapper::GetNthInput(int index)
{
  if (this->GetNumberOfInputPorts() < index + 1)
    return 0;
  
  if (this->GetNumberOfInputConnections(index) < 1)
    {
    return 0;
    }
    
  return vtkImageData::SafeDownCast(this->GetExecutive()->GetInputData(index, 0));
}

