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
  float SliceMatrix[4][4];
  float OrientationMatrix[4][4];
  float* ColorTransferFunction;
  float* AlphaTransferFunction;
  unsigned int FunctionSize;
  float FunctionRange[2];
  int VolumeRenderDirection; //0:Plus/Minus 1:Plus 2:Minus
  
  float Ambient;
  float Diffuse;
  float Specular;
  float SpecularPower;

  float MinMaxValue[6];
  float3 minROI;
  float3 maxROI;
  float MinThreshold;
  float MaxThreshold;

  float SampleDistance;

  float3* shadeField;
  float3 disp;

  float3 colorHigh;
  float3 colorLow;
} cudaVolumeInformation;
#endif /* __CUDAVOLUMEINFORMATION_H__ */
