#ifndef NTKCUDA_RAYCASTCOMPOSITE_H
#define NTKCUDA_RAYCASTCOMPOSITE_H

#include "CUDA_zbuffer_math.h"

template <typename T>
__device__ void CUDAkernel_RayCastComposite(cudaRendererInformation& renInfo,
                                            cudaVolumeInformation& volInfo,
                                            float* colorTF,
                                            float* alphaTF,
                                            float3* s_rayMap,
                                            float2* s_minmaxTrace,
                                            float3* s_clippingPoints,
                                            int tempacc,
                                            int xIndex,
                                            int yIndex){
  
  float tempx,tempy,tempz;
  float pos=0;
  
  float temp;
  float alpha;
  float r,g,b;
  float maxVal=0;
  float remainingOpacity=1.0;

  float outputVal[3];
  
  outputVal[0]=0;
  outputVal[1]=0;
  outputVal[2]=0;
  
  float newZBuffer;
  
  bool zBufferIsSet=false;
  float zBuffer;
  
  while((s_minmaxTrace[tempacc].y-s_minmaxTrace[tempacc].x)>=pos){
    
    tempx = ( s_rayMap[tempacc*2].x+((int)s_minmaxTrace[tempacc].x+pos)*s_rayMap[tempacc*2+1].x);
    tempy = ( s_rayMap[tempacc*2].y+((int)s_minmaxTrace[tempacc].x+pos)*s_rayMap[tempacc*2+1].y);
    tempz = ( s_rayMap[tempacc*2].z+((int)s_minmaxTrace[tempacc].x+pos)*s_rayMap[tempacc*2+1].z);
    
    CalculateZBuffer(renInfo, &zBuffer, s_clippingPoints[tempacc*2].x, s_clippingPoints[tempacc*2+1].x, tempx);
    
    if(zBuffer >= renInfo.ZBuffer[renInfo.ActualResolution.x-1-xIndex+yIndex*renInfo.ActualResolution.x]){
      break;
    }
    
    if(tempx >= volInfo.minROI.x && tempx <= volInfo.maxROI.x && tempy >= volInfo.minROI.y && tempy <= volInfo.maxROI.y && tempz >= volInfo.minROI.z && tempz <= volInfo.maxROI.z){
      
      temp=CUDAkernel_Interpolate<T>((T*)volInfo.SourceData, tempx, tempy, tempz);
 
      if(temp>maxVal)maxVal=temp;
      
      if(temp >=volInfo.MinThreshold && temp <= volInfo.MaxThreshold){
 
        temp=(float)(temp-volInfo.TypeRange[0])/(float)(volInfo.TypeRange[1]-volInfo.TypeRange[0])*(volInfo.FunctionSize-1);
 
        alpha=alphaTF[(int)temp];
        r=colorTF[(int)temp*3];
        g=colorTF[(int)temp*3+1];
        b=colorTF[(int)temp*3+2];
 
        if(remainingOpacity>0.02){
          if(!zBufferIsSet){
            newZBuffer = zBuffer;
            zBufferIsSet=true;
          }
   
          outputVal[0]+=remainingOpacity*alpha*r;
          outputVal[1]+=remainingOpacity*alpha*g;
          outputVal[2]+=remainingOpacity*alpha*b;
          remainingOpacity*=(1.0-alpha);
   
        }else{
          pos = s_minmaxTrace[tempacc].y-s_minmaxTrace[tempacc].x;
        }
       }
     }
    pos+=1.0;
  }
  
  renInfo.ZBuffer[renInfo.ActualResolution.x-1-xIndex+yIndex*renInfo.ActualResolution.x]=newZBuffer;
  
  renInfo.OutputImage[(int)(xIndex+yIndex*renInfo.ActualResolution.x)]=make_uchar4((outputVal[0]*255.0), (outputVal[1]*255.0), (outputVal[2]*255.0), (1-remainingOpacity)*255.0);
}

#endif
