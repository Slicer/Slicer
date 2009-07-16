#ifndef __CUDA_ZBUFFER_MATH_H__
#define __CUDA_ZBUFFER_MATH_H__

__device__ void CalculateZBuffer(const cudaRendererInformation renInfo, float *out, float clipPointMin, float clipPointMax, float currentPoint)
{
  if(renInfo.projectionMethod==0){
    float ratio = (currentPoint-clipPointMin)/(clipPointMax-clipPointMin);
    float zRange = ratio*(renInfo.ClippingRange.y-renInfo.ClippingRange.x)+renInfo.ClippingRange.x;
    
    *out =  renInfo.ZBufferA + renInfo.ZBufferB/zRange;
  }else{
    float ratio = (currentPoint-clipPointMin)/(clipPointMax-clipPointMin);
    *out = ratio;
  }
}

__device__ void CalculateReverseZBuffer(const cudaRendererInformation renInfo, float *out, float clipPointMin, float clipPointMax, float zBuffer)
{
  if(renInfo.projectionMethod==0){
    float zRange = renInfo.ZBufferB/(zBuffer-renInfo.ZBufferA);
    float ratio = (zRange - renInfo.ClippingRange.x)/(renInfo.ClippingRange.y-renInfo.ClippingRange.x);
    
    *out = ratio*(clipPointMax-clipPointMin)+clipPointMin;

  }else{
    *out = zBuffer*(clipPointMax-clipPointMin)+clipPointMin;
  }
}

#endif /* __CUDA_ZBUFFER_MATH_H__ */
