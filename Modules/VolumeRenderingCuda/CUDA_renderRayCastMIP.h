#ifndef NTKCUDAKERNEL_RAYCASTMIP
#define NTKCUDAKERNEL_RAYCASTMIP

#include "CUDA_zbuffer_math.h"

template <typename T>
__device__ void CUDAkernel_RayCastMIP(cudaRendererInformation& renInfo,
                                      cudaVolumeInformation& volInfo,
                                      float* colorTF,
                                      float* alphaTF,
                                      float3* s_rayMap,
                                      float2* s_minmaxTrace,
                                      float3* s_clippingPoints,
                                      int tempacc,
                                      int xIndex,
                                      int yIndex){
  
  float pos=-1;
  
  float val=0;
  float tempx,tempy,tempz;
  float temppos=0;
  
  float maxVal=0;
  
  float temp;
  
  float newZBuffer;
  
  bool zBufferIsSet=false;
  float zBuffer;
  
  while((s_minmaxTrace[tempacc].y-s_minmaxTrace[tempacc].x)>=temppos){
    
    tempx = ( s_rayMap[tempacc*2].x+((int)s_minmaxTrace[tempacc].x+temppos)*s_rayMap[tempacc*2+1].x);
    tempy = ( s_rayMap[tempacc*2].y+((int)s_minmaxTrace[tempacc].x+temppos)*s_rayMap[tempacc*2+1].y);
    tempz = ( s_rayMap[tempacc*2].z+((int)s_minmaxTrace[tempacc].x+temppos)*s_rayMap[tempacc*2+1].z);
    
    if(tempx >= volInfo.minROI.x && tempx <= volInfo.maxROI.x && tempy >= volInfo.minROI.y && tempy <= volInfo.maxROI.y && tempz >= volInfo.minROI.z && tempz <= volInfo.maxROI.z){
      
      temp=CUDAkernel_Interpolate<T>((T*)volInfo.SourceData, tempx, tempy, tempz);
      
      if(temp>maxVal)maxVal=temp;
      
      if(pos==-1 && temp >=volInfo.MinThreshold && temp <= volInfo.MaxThreshold){
        pos=temppos;
        
      }
      
    }
    temppos++;
    
  }
  
  
  tempx = ( s_rayMap[tempacc*2].x+((int)s_minmaxTrace[tempacc].x+pos)*s_rayMap[tempacc*2+1].x);
  tempy = ( s_rayMap[tempacc*2].y+((int)s_minmaxTrace[tempacc].x+pos)*s_rayMap[tempacc*2+1].y);
  tempz = ( s_rayMap[tempacc*2].z+((int)s_minmaxTrace[tempacc].x+pos)*s_rayMap[tempacc*2+1].z);
  
  CalculateZBuffer(renInfo, &zBuffer, s_clippingPoints[tempacc*2].x, s_clippingPoints[tempacc*2+1].x, tempx);
  
  if(zBuffer < renInfo.ZBuffer[renInfo.ActualResolution.x-1-xIndex+yIndex*renInfo.ActualResolution.x]){
    if(!zBufferIsSet){
      newZBuffer = zBuffer;
      renInfo.ZBuffer[renInfo.ActualResolution.x-1-xIndex+yIndex*renInfo.ActualResolution.x]=newZBuffer;
      zBufferIsSet=true;
    }
  }else{
    pos=-1;
  }
  
  if(pos!=-1 ){
    
    if(val==0.0){
      
    }
    
    if(val<0)val=1;
    
    if(val<=1.0){
      val=(maxVal-volInfo.TypeRange[0])/(volInfo.TypeRange[1]-volInfo.TypeRange[0]);
      renInfo.OutputImage[(int)((xIndex)+yIndex*renInfo.ActualResolution.x)]=make_uchar4((unsigned char)( volInfo.colorLow.x+(volInfo.colorHigh.x-volInfo.colorLow.x)*val),
                                                                                         (unsigned char)( volInfo.colorLow.y+(volInfo.colorHigh.y-volInfo.colorLow.y)*val), 
                                                                                         (unsigned char)( volInfo.colorLow.z+(volInfo.colorHigh.z-volInfo.colorLow.z)*val), 
                                                                                         255);
    }else{
      val=(val-volInfo.TypeRange[0])/(volInfo.TypeRange[1]-volInfo.TypeRange[0])*255.0;
      renInfo.OutputImage[(int)(xIndex+yIndex*renInfo.ActualResolution.x)]=make_uchar4((unsigned char)val, (unsigned char)val, (unsigned char)val, 255 ); 
    }
    
    
  }
  else{
    renInfo.OutputImage[(int)(xIndex+yIndex*renInfo.ActualResolution.x)]=make_uchar4(0,0,0,0);
  }
}

#endif
