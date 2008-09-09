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

#endif /* __CUDA_ZBUFFER_MATH_H__ */
