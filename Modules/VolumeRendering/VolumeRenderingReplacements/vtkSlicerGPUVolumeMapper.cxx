/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicerGPUVolumeTextureMapper3D.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSlicerGPUVolumeTextureMapper3D.h"
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
#include "vtkMultiThreader.h"

vtkCxxRevisionMacro(vtkSlicerGPUVolumeTextureMapper3D, "$Revision: 1.6 $");

//----------------------------------------------------------------------------
// Needed when we don't use the vtkStandardNewMacro.
vtkInstantiatorNewMacro(vtkSlicerGPUVolumeTextureMapper3D);
//----------------------------------------------------------------------------

// This method moves the scalars from the input volume into volume1 (and
// possibly volume2) which are the 3D texture maps used for rendering.
//
// In the case where our volume is a power of two, the copy is done
// directly. If we need to resample, then trilinear interpolation is used.
//
// A shift/scale is applied to the input scalar value to produce an 8 bit
// value for the texture volume.
//
/*06/30/2009 new scalar/normal storage schema (Yanling)
  1 component:
             R            G             B             A
  volume 1   scalar                                   normal.mag //one texture lookup for color
  volume 2   normal.x     normal.y      normal.z

  color = texture(scalar, normal.mag)

  2 components:
             R            G             B             A
  volume 1   scalar1      scalar2                     normal.mag
  volume 2   normal.x     normal.y      normal.z

  color = texture(scalar1, normal.mag)
  alpha = texture(scalar2, normal.mag)

  4 components:
             R            G             B             A
  volume 1   scalar1      scalar2       scalar3       scalar4
  volume 2   normal.x     normal.y      normal.z      normal.mag

  color = RGB(scalar1, scalar2, scalar3)
  alpha = texture(scalar4, normal.mag)

  1 component two volumes (A and B):
             R            G             B             A
  volume 1   scalar A     scalar B      normal.x.A    normal.y.A
  volume 2   normal.z.A   normal.x.B    normal.y.B    normal.z.B

*/
// 06/30/2009 original scalar/normal storage schema (VTK)
// When the input data is one component, the scalar value is placed in the
// second component of the two component volume1. The first component is
// filled in later with the gradient magnitude.
//
// When the input data is two component non-independent, the first component
// of the input data is placed in the first component of volume1, and the
// second component of the input data is placed in the third component of
// volume1. Volume1 has three components - the second is filled in later with
// the gradient magnitude.
//
// When the input data is four component non-independent, the first three
// components of the input data are placed in volume1 (which has three
// components), and the fourth component is placed in the second component
// of volume2. The first component of volume2 is later filled in with the
// gradient magnitude.

template <class T>
void vtkSlicerGPUVolumeTextureMapper3DComputeScalars( T *dataPtr,
                                               vtkSlicerGPUVolumeTextureMapper3D *me,
                                               float offset, float scale,
                                               unsigned char *volume1)
{
  T              *inPtr;
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
    if ( components == 1 )
      {
      outPtr = volume1;
        {
        for ( i = 0; i < size; i++ )
          {
          idx = static_cast<unsigned char>((*(inPtr++) + offset) * scale);
          *(outPtr++) = idx;
          *(outPtr++) = 0;
          *(outPtr++) = 0;
          *(outPtr++) = 0;
          }
        }
      }
    else if ( components == 2 )
      {
      outPtr = volume1;
        {
        for ( i = 0; i < size; i++ )
          {
          idx = static_cast<unsigned char>((*(inPtr++) + offset) * scale);
          *(outPtr++) = idx;
          idx = static_cast<unsigned char>((*(inPtr++) + offset) * scale);
          *(outPtr++) = idx;

          *(outPtr++) = 0;
          *(outPtr++) = 0;
          }
        }
      }
    else if ( components == 4 )
      {
      outPtr = volume1;
        {
        for ( i = 0; i < size; i++ )
          {
          idx = static_cast<unsigned char>((*(inPtr++) + offset) * scale);
          *(outPtr++) = idx;
          idx = static_cast<unsigned char>((*(inPtr++) + offset) * scale);
          *(outPtr++) = idx;
          idx = static_cast<unsigned char>((*(inPtr++) + offset) * scale);
          *(outPtr++) = idx;
          idx = static_cast<unsigned char>((*(inPtr++) + offset) * scale);
          *(outPtr++) = idx;
          }
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

          inPtr =
            dataPtr + components * ( z*inputDimensions[0]*inputDimensions[1] +
                                     y*inputDimensions[0] +
                                     x );

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
            *(outPtr++) = 0;
            *(outPtr++) = 0;
            *(outPtr++) = 0;
            }
          else if ( components == 2 )
            {
            float A1, B1, C1, D1, E1, F1, G1, H1;
            float A2, B2, C2, D2, E2, F2, G2, H2;
            A1 = static_cast<float>(*(inPtr));
            A2 = static_cast<float>(*(inPtr+1));
            B1 = static_cast<float>(*(inPtr+2));
            B2 = static_cast<float>(*(inPtr+3));
            C1 = static_cast<float>(*(inPtr+2*inputDimensions[0]));
            C2 = static_cast<float>(*(inPtr+2*inputDimensions[0]+1));
            D1 = static_cast<float>(*(inPtr+2*inputDimensions[0]+2));
            D2 = static_cast<float>(*(inPtr+2*inputDimensions[0]+3));
            E1 = static_cast<float>(*(inPtr+2*inputDimensions[0]*inputDimensions[1]));
            E2 = static_cast<float>(*(inPtr+2*inputDimensions[0]*inputDimensions[1]+1));
            F1 = static_cast<float>(*(inPtr+2*inputDimensions[0]*inputDimensions[1]+2));
            F2 = static_cast<float>(*(inPtr+2*inputDimensions[0]*inputDimensions[1]+3));
            G1 = static_cast<float>(*(inPtr+2*inputDimensions[0]*inputDimensions[1]+2*inputDimensions[0]));
            G2 = static_cast<float>(*(inPtr+2*inputDimensions[0]*inputDimensions[1]+2*inputDimensions[0]+1));
            H1 = static_cast<float>(*(inPtr+2*inputDimensions[0]*inputDimensions[1]+2*inputDimensions[0]+2));
            H2 = static_cast<float>(*(inPtr+2*inputDimensions[0]*inputDimensions[1]+2*inputDimensions[0]+3));

            float val1 =
              (1.0-wx)*(1.0-wy)*(1.0-wz)*A1 +
              (    wx)*(1.0-wy)*(1.0-wz)*B1 +
              (1.0-wx)*(    wy)*(1.0-wz)*C1 +
              (    wx)*(    wy)*(1.0-wz)*D1 +
              (1.0-wx)*(1.0-wy)*(    wz)*E1 +
              (    wx)*(1.0-wy)*(    wz)*F1 +
              (1.0-wx)*(    wy)*(    wz)*G1 +
              (    wx)*(    wy)*(    wz)*H1;


            float val2 =
              (1.0-wx)*(1.0-wy)*(1.0-wz)*A2 +
              (    wx)*(1.0-wy)*(1.0-wz)*B2 +
              (1.0-wx)*(    wy)*(1.0-wz)*C2 +
              (    wx)*(    wy)*(1.0-wz)*D2 +
              (1.0-wx)*(1.0-wy)*(    wz)*E2 +
              (    wx)*(1.0-wy)*(    wz)*F2 +
              (1.0-wx)*(    wy)*(    wz)*G2 +
              (    wx)*(    wy)*(    wz)*H2;

            idx = static_cast<unsigned char>((val1 + offset) * scale);
            *(outPtr++) = idx;

            idx = static_cast<unsigned char>((val2 + offset) * scale);
            *(outPtr++) = idx;

            *(outPtr++) = 0;
            *(outPtr++) = 0;
            }
          else
            {
            float Ar, Br, Cr, Dr, Er, Fr, Gr, Hr;
            float Ag, Bg, Cg, Dg, Eg, Fg, Gg, Hg;
            float Ab, Bb, Cb, Db, Eb, Fb, Gb, Hb;
            float Aa, Ba, Ca, Da, Ea, Fa, Ga, Ha;
            Ar = static_cast<float>(*(inPtr));
            Ag = static_cast<float>(*(inPtr+1));
            Ab = static_cast<float>(*(inPtr+2));
            Aa = static_cast<float>(*(inPtr+3));
            Br = static_cast<float>(*(inPtr+4));
            Bg = static_cast<float>(*(inPtr+5));
            Bb = static_cast<float>(*(inPtr+6));
            Ba = static_cast<float>(*(inPtr+7));
            Cr = static_cast<float>(*(inPtr+4*inputDimensions[0]));
            Cg = static_cast<float>(*(inPtr+4*inputDimensions[0]+1));
            Cb = static_cast<float>(*(inPtr+4*inputDimensions[0]+2));
            Ca = static_cast<float>(*(inPtr+4*inputDimensions[0]+3));
            Dr = static_cast<float>(*(inPtr+4*inputDimensions[0]+4));
            Dg = static_cast<float>(*(inPtr+4*inputDimensions[0]+5));
            Db = static_cast<float>(*(inPtr+4*inputDimensions[0]+6));
            Da = static_cast<float>(*(inPtr+4*inputDimensions[0]+7));
            Er = static_cast<float>(*(inPtr+4*inputDimensions[0]*inputDimensions[1]));
            Eg = static_cast<float>(*(inPtr+4*inputDimensions[0]*inputDimensions[1]+1));
            Eb = static_cast<float>(*(inPtr+4*inputDimensions[0]*inputDimensions[1]+2));
            Ea = static_cast<float>(*(inPtr+4*inputDimensions[0]*inputDimensions[1]+3));
            Fr = static_cast<float>(*(inPtr+4*inputDimensions[0]*inputDimensions[1]+4));
            Fg = static_cast<float>(*(inPtr+4*inputDimensions[0]*inputDimensions[1]+5));
            Fb = static_cast<float>(*(inPtr+4*inputDimensions[0]*inputDimensions[1]+6));
            Fa = static_cast<float>(*(inPtr+4*inputDimensions[0]*inputDimensions[1]+7));
            Gr = static_cast<float>(*(inPtr+4*inputDimensions[0]*inputDimensions[1]+4*inputDimensions[0]));
            Gg = static_cast<float>(*(inPtr+4*inputDimensions[0]*inputDimensions[1]+4*inputDimensions[0]+1));
            Gb = static_cast<float>(*(inPtr+4*inputDimensions[0]*inputDimensions[1]+4*inputDimensions[0]+2));
            Ga = static_cast<float>(*(inPtr+4*inputDimensions[0]*inputDimensions[1]+4*inputDimensions[0]+3));
            Hr = static_cast<float>(*(inPtr+4*inputDimensions[0]*inputDimensions[1]+4*inputDimensions[0]+4));
            Hg = static_cast<float>(*(inPtr+4*inputDimensions[0]*inputDimensions[1]+4*inputDimensions[0]+5));
            Hb = static_cast<float>(*(inPtr+4*inputDimensions[0]*inputDimensions[1]+4*inputDimensions[0]+6));
            Ha = static_cast<float>(*(inPtr+4*inputDimensions[0]*inputDimensions[1]+4*inputDimensions[0]+7));

            float valr =
              (1.0-wx)*(1.0-wy)*(1.0-wz)*Ar +
              (    wx)*(1.0-wy)*(1.0-wz)*Br +
              (1.0-wx)*(    wy)*(1.0-wz)*Cr +
              (    wx)*(    wy)*(1.0-wz)*Dr +
              (1.0-wx)*(1.0-wy)*(    wz)*Er +
              (    wx)*(1.0-wy)*(    wz)*Fr +
              (1.0-wx)*(    wy)*(    wz)*Gr +
              (    wx)*(    wy)*(    wz)*Hr;

            float valg =
              (1.0-wx)*(1.0-wy)*(1.0-wz)*Ag +
              (    wx)*(1.0-wy)*(1.0-wz)*Bg +
              (1.0-wx)*(    wy)*(1.0-wz)*Cg +
              (    wx)*(    wy)*(1.0-wz)*Dg +
              (1.0-wx)*(1.0-wy)*(    wz)*Eg +
              (    wx)*(1.0-wy)*(    wz)*Fg +
              (1.0-wx)*(    wy)*(    wz)*Gg +
              (    wx)*(    wy)*(    wz)*Hg;

            float valb =
              (1.0-wx)*(1.0-wy)*(1.0-wz)*Ab +
              (    wx)*(1.0-wy)*(1.0-wz)*Bb +
              (1.0-wx)*(    wy)*(1.0-wz)*Cb +
              (    wx)*(    wy)*(1.0-wz)*Db +
              (1.0-wx)*(1.0-wy)*(    wz)*Eb +
              (    wx)*(1.0-wy)*(    wz)*Fb +
              (1.0-wx)*(    wy)*(    wz)*Gb +
              (    wx)*(    wy)*(    wz)*Hb;

            float vala =
              (1.0-wx)*(1.0-wy)*(1.0-wz)*Aa +
              (    wx)*(1.0-wy)*(1.0-wz)*Ba +
              (1.0-wx)*(    wy)*(1.0-wz)*Ca +
              (    wx)*(    wy)*(1.0-wz)*Da +
              (1.0-wx)*(1.0-wy)*(    wz)*Ea +
              (    wx)*(1.0-wy)*(    wz)*Fa +
              (1.0-wx)*(    wy)*(    wz)*Ga +
              (    wx)*(    wy)*(    wz)*Ha;

            idx = static_cast<unsigned char>((valr + offset) * scale);
            *(outPtr++) = idx;
            idx = static_cast<unsigned char>((valg + offset) * scale);
            *(outPtr++) = idx;
            idx = static_cast<unsigned char>((valb + offset) * scale);
            *(outPtr++) = idx;
            idx = static_cast<unsigned char>((vala + offset) * scale);
            *(outPtr++) = idx;
            }
          }
        }
      }
    }
}


VTK_THREAD_RETURN_TYPE vtkSlicerGPUVolumeTextureMapper3DComputeGradients( void *arg)
{
  GradientsArgsType *pArgs = (GradientsArgsType *)(((vtkMultiThreader::ThreadInfo *)arg)->UserData);

  float *dataPtr = pArgs->dataPtr;
  vtkSlicerGPUVolumeTextureMapper3D *me = pArgs->me;
  double scalarRange[2];
  scalarRange[0] = pArgs->scalarRange[0];
  scalarRange[1] = pArgs->scalarRange[1];
  unsigned char *volume1 = pArgs->volume1;
  unsigned char *volume2 = pArgs->volume2;

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
  unsigned char       *outPtr1, *outPtr2;
  unsigned char       *normals, *gradmags;
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

  if (thread_id == 0)
    me->InvokeEvent( vtkCommand::VolumeMapperComputeGradientsStartEvent, NULL );

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

  normals = volume2;

  if ( components == 1 || components == 2 )
    {
    gradmags = volume1;
    }
  else
    {
    gradmags = volume2;
    }

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
      outPtr1 = gradmags + 4*outputOffset;
      outPtr2 = normals + 4*outputOffset;

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

        *(outPtr1+3) = static_cast<unsigned char>(gvalue + 0.5);
//        *(outPtr1+3) *= 256;//scale up to fit unsigned char 16bit

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

        *(outPtr2  ) = nx;
        *(outPtr2+1) = ny;
        *(outPtr2+2) = nz;
        *(outPtr2+3) = 0;

        outPtr1 += 4;
        outPtr2 += 4;
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


vtkSlicerGPUVolumeTextureMapper3D::vtkSlicerGPUVolumeTextureMapper3D()
{
  // The input used when creating the textures
  this->SavedTextureInput             = NULL;

  // The input used when creating the color tables
  this->SavedParametersInput           = NULL;

  this->SavedRGBFunction              = NULL;
  this->SavedGrayFunction             = NULL;
  this->SavedScalarOpacityFunction    = NULL;
  this->SavedColorChannels            = 0;
  this->SavedScalarOpacityDistance    = 0;

  this->Volume1                       = NULL;
  this->Volume2                       = NULL;
  this->VolumeSize                    = 0;
  this->VolumeComponents              = 0;

  this->Framerate                    = 5.0f;

  this->GradientsArgs                 = NULL;

  this->Threader               = vtkMultiThreader::New();
}

vtkSlicerGPUVolumeTextureMapper3D::~vtkSlicerGPUVolumeTextureMapper3D()
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


vtkSlicerGPUVolumeTextureMapper3D *vtkSlicerGPUVolumeTextureMapper3D::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret =
    vtkSlicerVolumeRenderingFactory::CreateInstance("vtkSlicerGPUVolumeTextureMapper3D");
  return (vtkSlicerGPUVolumeTextureMapper3D*)ret;
}

int vtkSlicerGPUVolumeTextureMapper3D::UpdateVolumes(vtkVolume *vtkNotUsed(vol))
{
  int needToUpdate = 0;

  // Get the image data
  vtkImageData *input = this->GetInput();
  input->Update();

  // Has the volume changed in some way?
  if ( this->SavedTextureInput != input ||
       this->SavedTextureMTime.GetMTime() < input->GetMTime() )
  {
    needToUpdate = 1;
  }

  if ( !needToUpdate )
  {
    return 0;
  }

  this->SavedTextureInput = input;
  this->SavedTextureMTime.Modified();

  // How big does the Volume need to be?
  int dim[3];
  input->GetDimensions(dim);

  int powerOfTwoDim[3];
  input->GetDimensions(powerOfTwoDim);

/*  for ( int i = 0; i < 3; i++ )
    {
    powerOfTwoDim[i] = 32;
    while ( powerOfTwoDim[i] < dim[i] )
      {
      powerOfTwoDim[i] *= 2;
      }
    }
*/
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

  int components = input->GetNumberOfScalarComponents();

  // What is the spacing?
  double spacing[3];
  input->GetSpacing(spacing);

  // allocate memory
  {
    if (this->Volume1)
        delete [] this->Volume1;
    if (this->Volume2)
        delete [] this->Volume2;

    this->Volume1 = new unsigned char [4*neededSize];
    this->Volume2 = new unsigned char [4*neededSize];

    this->VolumeSize       = neededSize;
    this->VolumeComponents = components;
  }

  // Find the scalar range
  double scalarRange[2];
  input->GetPointData()->GetScalars()->GetRange(scalarRange, components-1);

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
  void*  dataPtr = input->GetScalarPointer();

  switch ( scalarType )
    {
    vtkTemplateMacro(
      vtkSlicerGPUVolumeTextureMapper3DComputeScalars(
        (VTK_TT *)(dataPtr), this,
        offset, scale,
        this->Volume1));
    }

  int dataPtrSize = dim[0]*dim[1]*dim[2]*components;
  float* floatDataPtr = new float[dataPtrSize];

  //copy out scalar as float array for multithreading
  //or maybe later multithreading with template...
  CopyToFloatBuffer(input, floatDataPtr, dataPtrSize);

  if (this->GradientsArgs)
        delete this->GradientsArgs;

  this->GradientsArgs = new GradientsArgsType;

  this->GradientsArgs->dataPtr = floatDataPtr;
  this->GradientsArgs->me = this;
  this->GradientsArgs->scalarRange[0] = scalarRange[0];
  this->GradientsArgs->scalarRange[1] = scalarRange[1];
  this->GradientsArgs->volume1 = this->Volume1;
  this->GradientsArgs->volume2 = this->Volume2;

  this->Threader->SetSingleMethod( vtkSlicerGPUVolumeTextureMapper3DComputeGradients, (void *)(this->GradientsArgs) );

  this->Threader->SingleMethodExecute();

  delete [] floatDataPtr;

  return 1;
}

void vtkSlicerGPUVolumeTextureMapper3D::CopyToFloatBuffer(vtkImageData* input, float* floatDataPtr, int dataPtrSize)
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

int vtkSlicerGPUVolumeTextureMapper3D::UpdateColorLookup( vtkVolume *vol )
{
  int needToUpdate = 0;

  // Get the image data
  vtkImageData *input = this->GetInput();
  input->Update();

  // Has the volume changed in some way?
  if ( this->SavedParametersInput != input ||
       this->SavedParametersMTime.GetMTime() < input->GetMTime() )
    {
    needToUpdate = 1;
    }

  // How many components?
  int components = input->GetNumberOfScalarComponents();

  vtkColorTransferFunction *rgbFunc  = NULL;
  vtkPiecewiseFunction     *grayFunc = NULL;

  // How many color channels for this component?
  int colorChannels = vol->GetProperty()->GetColorChannels(0);

  if ( components < 3 )
    {
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
    }

  // Has the scalar opacity transfer function changed in some way?
  vtkPiecewiseFunction *scalarOpacityFunc =
    vol->GetProperty()->GetScalarOpacity(0);
  if ( this->SavedScalarOpacityFunction != scalarOpacityFunc ||
       this->SavedParametersMTime.GetMTime() <
       scalarOpacityFunc->GetMTime() )
    {
    needToUpdate = 1;
    }

  // Has the gradient opacity transfer function changed in some way?
  vtkPiecewiseFunction *gradientOpacityFunc =
    vol->GetProperty()->GetGradientOpacity(0);
  if ( this->SavedGradientOpacityFunction != gradientOpacityFunc ||
       this->SavedParametersMTime.GetMTime() <
       gradientOpacityFunc->GetMTime() )
    {
    needToUpdate = 1;
    }


  double scalarOpacityDistance =
    vol->GetProperty()->GetScalarOpacityUnitDistance(0);
  if ( this->SavedScalarOpacityDistance != scalarOpacityDistance )
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
  this->SavedParametersInput         = input;

  this->SavedParametersMTime.Modified();

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

 /*
 <-  2048 color lookup table ->
 ^
 |
 256 gradient opacity lookup table
 |
 v
 */
  int goLoop;
  unsigned char *ptr, *rgbptr, *aptr;
  float *fptr1, *fptr2;

  int i;
  switch (components)
    {
    case 1:
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
      break;

    case 2:
      // Move the two temp float arrays into one RGB unsigned char array and
      // one alpha array.
      rgbptr = this->ColorLookup;

      if ( colorChannels == 1 )
        {
        for ( i = 0; i < arraySizeNeeded; i++ )
          {
          fptr1 = this->TempArray1;
          fptr2 = this->TempArray2;
          for ( goLoop = 0; goLoop < 256; goLoop++ )
            {
            *(rgbptr++) = static_cast<unsigned char>(*(fptr1)*255.0 + 0.5);
            *(rgbptr++) = static_cast<unsigned char>(*(fptr1)*255.0 + 0.5);
            *(rgbptr++) = static_cast<unsigned char>(*(fptr1++)*255.0 + 0.5);
            *(rgbptr++) = static_cast<unsigned char>(*(fptr2++)*goArray[goLoop]*255.0 + 0.5);
            }
          }
        }
      else
        {
        fptr1 = this->TempArray1;
        fptr2 = this->TempArray2;
        for ( i = 0; i < arraySizeNeeded; i++ )
          {
          for ( goLoop = 0; goLoop < 256; goLoop++ )
            {
            *(rgbptr++) = static_cast<unsigned char>(*(fptr1)*255.0 + 0.5);
            *(rgbptr++) = static_cast<unsigned char>(*(fptr1+1)*255.0 + 0.5);
            *(rgbptr++) = static_cast<unsigned char>(*(fptr1+2)*255.0 + 0.5);
            *(rgbptr++) = static_cast<unsigned char>(*(fptr2)*goArray[goLoop]*255.0 + 0.5);
            }
          fptr1+=3;
          fptr2++;
          }
        }

      for ( ; i < 256; i++ )
        {
        for ( goLoop = 0; goLoop < 256; goLoop++ )
          {
          *(rgbptr++) = 0;
          *(rgbptr++) = 0;
          *(rgbptr++) = 0;
          *(rgbptr++) = 0;
          }
        }
      break;

    case 3:
    case 4:
      // Move the two temp float arrays into one alpha array
      aptr = this->ColorLookup;

      for ( goLoop = 0; goLoop < 256; goLoop++ )
        {
        fptr2 = this->TempArray2;
        for ( i = 0; i < arraySizeNeeded; i++ )
          {
          *(aptr++)   = static_cast<unsigned char>(*(fptr2++)*goArray[goLoop]*255.0 + 0.5);
          *(aptr++)   = static_cast<unsigned char>(*(fptr2++)*goArray[goLoop]*255.0 + 0.5);
          *(aptr++)   = static_cast<unsigned char>(*(fptr2++)*goArray[goLoop]*255.0 + 0.5);
          *(aptr++)   = static_cast<unsigned char>(*(fptr2++)*goArray[goLoop]*255.0 + 0.5);
          }
        for ( ; i < 256; i++ )
          {
          *(aptr++)   = 0;
          *(aptr++)   = 0;
          *(aptr++)   = 0;
          *(aptr++)   = 0;
          }
        }

      break;
    }
  return 1;
}


// Print the vtkSlicerGPUVolumeTextureMapper3D
void vtkSlicerGPUVolumeTextureMapper3D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "VolumeDimensions: " << this->VolumeDimensions[0] << " "
     << this->VolumeDimensions[1] << " " << this->VolumeDimensions[2] << endl;
  os << indent << "VolumeSpacing: " << this->VolumeSpacing[0] << " "
     << this->VolumeSpacing[1] << " " << this->VolumeSpacing[2] << endl;
}



