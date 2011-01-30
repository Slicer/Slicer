#ifndef __CUDAVOLUMEINFORMATION_H__
#define __CUDAVOLUMEINFORMATION_H__

#include <cutil.h>
#include "vector_types.h"

//! A datastructure located on the cudacard that holds the information of the volume.
extern "C"
typedef struct __align__(16) {
   void* SourceData;
  int InputDataType;
  float TypeRange[2];
  
  int3 VolumeSize;
  float Transform[4][4];
  float OrientationMatrix[4][4];
  float* ColorTransferFunction;
  float* AlphaTransferFunction;
  float* GradientTransferFunction;
  unsigned int FunctionSize;
  float FunctionRange[2];
  
  int InterpolationType; //0:NN, 1:trilinear

  float Ambient;
  float Diffuse;
  float Specular;
  float SpecularPower;

  float3 MinROI;
  float3 MaxROI;
  float MinThreshold;
  float MaxThreshold;

  float SampleDistance;

  float3* GradientData;
  int IsGradientReady;
} cudaVolumeInformation;
#endif /* __CUDAVOLUMEINFORMATION_H__ */
