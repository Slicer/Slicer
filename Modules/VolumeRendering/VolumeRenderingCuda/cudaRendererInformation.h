#ifndef __CUDARENDERERINFORMATION_H__
#define __CUDARENDERERINFORMATION_H__

#include <cutil.h>
#include "vector_types.h"

//! A Datastucture located on the cuda hardware that holds all the information about the renderer.
extern "C"
typedef struct __align__(16)
{
  uint2 Resolution;
  uint2 ActualResolution;
  
  float3 CameraRayStart;
  float3 CameraRayStartX;
  float3 CameraRayStartY;
  float3 CameraRayEnd;
  float3 CameraRayEndX;
  float3 CameraRayEndY;

  unsigned int ColorDepth;
  unsigned int LightCount;
  float3* LightVectors;
  
  float3 CameraPos;
  float3 FocalPoint;
  float3 CameraDirection;
  float3 ViewUp;
  float3 HorizontalVec;
  float3 VerticalVec;
  float3 Vec;

  uchar4* OutputImage;
  float* ZBuffer;
  float ZBufferA;
  float ZBufferB;
  float2 ClippingRange;
  float3 ClippingPlaneNormal;
  float* LensMap;

  float ScaleFactor;

  unsigned int ClippingOn; //0:Off, 1:On
  unsigned int CroppingPlaneNumber;
  float3 CroppingNormal[10];
  float3 CroppingOrigin[10];

  unsigned int ShadingOn; //0:Off, 1:On
  
  int rayCastingMethod; // 0:Isosurface 1:MIP 2:Composite 3:CompositeShaded
  int interpolationMethod; // 0:Nearest Neighbor 1:Trilinear
  int projectionMethod; //0:Perspective 1:Orthogonal
} cudaRendererInformation;

#endif /* __CUDARENDERERINFORMATION_H__ */
