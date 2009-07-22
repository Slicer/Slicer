/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicerGPUVolumeMapper.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSlicerGPUVolumeMapper.h"
#include "vtkSlicerVolumeRenderingFactory.h"

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

vtkCxxRevisionMacro(vtkSlicerGPUVolumeMapper, "$Revision: 1.6 $");

//----------------------------------------------------------------------------
// Needed when we don't use the vtkStandardNewMacro.
vtkInstantiatorNewMacro(vtkSlicerGPUVolumeMapper);
//----------------------------------------------------------------------------

//
//  1 component three input volumes (A, B, and LabelMap):
//             R            G             B             A
//  volume 1   scalar.A     scalar.B      Label                
//  
//  normal calculated at run time

template <class T>
void vtkSlicerGPUVolumeMapperComputeScalars( T *dataPtr,
                                                T *dataPtr1,
                                                T *dataPtr2;
                                               vtkSlicerGPUVolumeMapper *me,
                                               float offset, float scale,
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
  me->GetInput()->GetDimensions( inputDimensions );
  me->GetInput()->GetSpacing( inputSpacing );

  int   outputDimensions[3];
  float outputSpacing[3];
  me->GetVolumeDimensions( outputDimensions );
  me->GetVolumeSpacing( outputSpacing );

  int components = me->GetInput()->GetNumberOfScalarComponents();

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
    inPtr = dataPtr1;
    if ( components == 1 )
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
            idx = static_cast<unsigned char>((*(inPtr1++) + offset) * scale);
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

          inPtr = dataPtr + components * ( z*inputDimensions[0]*inputDimensions[1] + y*inputDimensions[0] + x );
          
          if ( components == 1 )
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

                idx = static_cast<unsigned char>((val + offset)*scale);
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

vtkSlicerGPUVolumeMapper::vtkSlicerGPUVolumeMapper()
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
  this->VolumeSize                    = 0;
  
  this->Framerate                    = 5.0f;
}

vtkSlicerGPUVolumeMapper::~vtkSlicerGPUVolumeMapper()
{
    if (this->Volume1)
        delete [] this->Volume1;
}


vtkSlicerGPUVolumeMapper *vtkSlicerGPUVolumeMapper::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkSlicerVolumeRenderingFactory::CreateInstance("vtkSlicerGPUVolumeMapper");
  return (vtkSlicerGPUVolumeMapper*)ret;
}

int vtkSlicerGPUVolumeMapper::UpdateVolumes(vtkVolume *vtkNotUsed(vol))
{
  int needToUpdate = 0;

  // Get the image data
  vtkImageData *input = this->GetNthInput(0);
  input->Update();
  
  vtkImageData *input1 = this->GetNthInput(1);
  input1->Update();
  
  // Has the volume changed in some way?
  if ( this->SavedTextureInput != input || this->SavedTextureMTime.GetMTime() < input->GetMTime() ||
       this->SavedTextureInput2nd != input1 || this->SavedTextureMTime2nd.GetMTime() < input1->GetMTime() )
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
    
    this->Volume1 = new unsigned char [4*neededSize];
    
    this->VolumeSize       = neededSize;
  }
 
  // Find the scalar range
  double scalarRange[2];
  double scalarRange1[2], scalarRange2[2];
  
  input->GetPointData()->GetScalars()->GetRange(scalarRange1, components-1);
  input1->GetPointData()->GetScalars()->GetRange(scalarRange2, components-1);
  
  //find scalar range for two inputs
  scalarRange[0] = scalarRange1[0] < scalarRange2[0] ? scalarRange1[0] : scalarRange2[0];
  scalarRange[1] = scalarRange1[1] > scalarRange2[1] ? scalarRange1[1] : scalarRange2[1];
 
  // Is the difference between max and min less than 4096? If so, and if
  // the data is not of float or double type, use a simple offset mapping.
  // If the difference between max and min is 4096 or greater, or the data
  // is of type float or double, we must use an offset / scaling mapping.
  // In this case, the array size will be 4096 - we need to figure out the 
  // offset and scale factor.
  float offset;
  float scale;
 
  int arraySizeNeeded;
 
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
 
  this->ScalarOffset = offset;
  this->ScalarScale = scale;
  
  this->ColorTableSize   = arraySizeNeeded;
  
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
  void *dataPtr1 = input1->GetScalarPointer();
    
  switch ( scalarType )
    {
    vtkTemplateMacro(
      vtkSlicerGPUVolumeMapperComputeScalars(
        (VTK_TT *)(dataPtr), (VTK_TT*)(dataPtr1), 
        this, offset, scale,
        this->Volume1));
    }

  return 1;
}


int vtkSlicerGPUVolumeMapper::UpdateColorLookup( vtkVolume *vol )
{
  int needToUpdate = 0;

  // Get the image data
  vtkImageData *input = this->GetNthInput(0);
  input->Update();
  
  vtkImageData *input1 = this->GetNthInput(1);
  input1->Update();
  
  // Has the volume changed in some way?
  if ( this->SavedTextureInput != input || this->SavedColorOpacityMTime.GetMTime() < input->GetMTime() ||
       this->SavedTextureInput2nd != input1 || this->SavedColorOpacityMTime2nd.GetMTime() < input1->GetMTime() )
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
         this->SavedParametersMTime.GetMTime() < rgbFunc->GetMTime() )
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
         this->SavedParametersMTime.GetMTime() < grayFunc->GetMTime() )
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
    if ( this->SavedRGBFunction2nd != rgbFunc1 ||
         this->SavedParametersMTime2nd.GetMTime() < rgbFunc1->GetMTime() )
    {
      needToUpdate = 1;
    }
  }

  // Has the gray transfer function changed in some way,
  // and we are using it?
  if ( colorChannels1 == 1 )
  {
    grayFunc1 = vol->GetProperty()->GetGrayTransferFunction(1);
    if ( this->SavedGrayFunction2nd != grayFunc1 ||
         this->SavedParametersMTime2nd.GetMTime() < grayFunc1->GetMTime() )
    {
      needToUpdate = 1;
    }
  }
    
  // Has the scalar opacity transfer function changed in some way?
  vtkPiecewiseFunction *scalarOpacityFunc = vol->GetProperty()->GetScalarOpacity(0);
  if ( this->SavedScalarOpacityFunction != scalarOpacityFunc ||
       this->SavedParametersMTime.GetMTime() < scalarOpacityFunc->GetMTime() )
    {
    needToUpdate = 1;
    }

  vtkPiecewiseFunction *scalarOpacityFunc1 = vol->GetProperty()->GetScalarOpacity(1);
  if ( this->SavedScalarOpacityFunction2nd != scalarOpacityFunc1 ||
       this->SavedParametersMTime2nd.GetMTime() < scalarOpacityFunc1->GetMTime() )
    {
    needToUpdate = 1;
    }
    
  // Has the gradient opacity transfer function changed in some way?
  vtkPiecewiseFunction *gradientOpacityFunc = vol->GetProperty()->GetGradientOpacity(0);
  if ( this->SavedGradientOpacityFunction != gradientOpacityFunc ||
       this->SavedParametersMTime.GetMTime() < gradientOpacityFunc->GetMTime() )
    {
    needToUpdate = 1;
    }

  vtkPiecewiseFunction *gradientOpacityFunc1 = vol->GetProperty()->GetGradientOpacity(1);
  if ( this->SavedGradientOpacityFunction2nd != gradientOpacityFunc1 ||
       this->SavedParametersMTime2nd.GetMTime() < gradientOpacityFunc1->GetMTime() )
    {
    needToUpdate = 1;
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
  input->GetPointData()->GetScalars()->GetRange(scalarRange, components-1);
  
  int arraySizeNeeded = this->ColorTableSize;

  if ( components < 3 )
    {
    // Sample the transfer functions between the min and max.
    if ( colorChannels == 1 )
      {
      grayFunc->GetTable( scalarRange[0], scalarRange[1], 
                          arraySizeNeeded, this->TempArray1 );
      }
    else
      {
      rgbFunc->GetTable( scalarRange[0], scalarRange[1], 
                         arraySizeNeeded, this->TempArray1 );
      }
    }
  
  scalarOpacityFunc->GetTable( scalarRange[0], scalarRange[1], 
                               arraySizeNeeded, this->TempArray2 );

  float goArray[256];
  gradientOpacityFunc->GetTable( 0, (scalarRange[1] - scalarRange[0])*0.25, 
                                 256, goArray );
                                 
// Find the scalar range
  double scalarRange1[2];
  input1->GetPointData()->GetScalars()->GetRange(scalarRange1, components1-1);

  // Sample the transfer functions between the min and max.
  if ( colorChannels1 == 1 )
  {
    grayFunc1->GetTable( scalarRange1[0], scalarRange1[1], arraySizeNeeded, this->TempArray11 );
  }
  else
  {
    rgbFunc1->GetTable( scalarRange1[0], scalarRange1[1], arraySizeNeeded, this->TempArray11 );
  }
  
  scalarOpacityFunc1->GetTable( scalarRange1[0], scalarRange1[1], arraySizeNeeded, this->TempArray21 );

  float goArray1[256];
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


// Print the vtkSlicerGPUVolumeMapper
void vtkSlicerGPUVolumeMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "VolumeDimensions: " << this->VolumeDimensions[0] << " "
     << this->VolumeDimensions[1] << " " << this->VolumeDimensions[2] << endl;
  os << indent << "VolumeSpacing: " << this->VolumeSpacing[0] << " "
     << this->VolumeSpacing[1] << " " << this->VolumeSpacing[2] << endl;
}

void vtkSlicerGPUVolumeMapper::SetNthInput( vtkDataSet *genericInput , int index)
{
  vtkImageData *input = vtkImageData::SafeDownCast( genericInput );
  
  if ( input )
    {
    this->SetNthInput( input , index);
    }
  else
    {
    vtkErrorMacro("The SetInput method of this mapper requires vtkImageData as input");
    }
}

void vtkSlicerGPUVolumeMapper::SetNthInput( vtkImageData *input , int index )
{
  if(input)
    {
    this->SetInputConnection(index, input->GetProducerPort());
    }
  else
    {
    // Setting a NULL input removes the connection.
    this->SetInputConnection(index, 0);
    }
}

vtkImageData *vtkSlicerGPUVolumeMapper::GetNthInput(int index)
{
  if (this->GetNumberOfInputConnections(index) < index+1)
    {
    return 0;
    }
  return vtkImageData::SafeDownCast(this->GetExecutive()->GetInputData(index, 0));
}

