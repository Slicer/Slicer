#ifndef NTKCUDAKERNEL_RAYCASTISOSURFACE
#define NTKCUDAKERNEL_RAYCASTISOSURFACE

#include "CUDA_matrix_math.h"
#include "CUDA_zbuffer_math.h"

#define SQR(X) ((X) * (X) )

template <typename T>
__device__ void CUDAkernel_RayCastIsosurface(cudaRendererInformation& renInfo,
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
  
  float temp;
  float tempf;
  
  float4 shadeField;
  
  float newZBuffer = renInfo.ZBuffer[(renInfo.ActualResolution.x-1-xIndex)+yIndex*renInfo.ActualResolution.x];
  
  bool zBufferIsSet=false;
  float zBuffer;
  
  while((s_minmaxTrace[tempacc].y-s_minmaxTrace[tempacc].x)>=temppos){
    
    tempx = ( s_rayMap[tempacc*2].x+((int)s_minmaxTrace[tempacc].x+temppos)*s_rayMap[tempacc*2+1].x);
    tempy = ( s_rayMap[tempacc*2].y+((int)s_minmaxTrace[tempacc].x+temppos)*s_rayMap[tempacc*2+1].y);
    tempz = ( s_rayMap[tempacc*2].z+((int)s_minmaxTrace[tempacc].x+temppos)*s_rayMap[tempacc*2+1].z);
    
    if(tempx >= volInfo.minROI.x && tempx <= volInfo.maxROI.x && tempy >= volInfo.minROI.y && tempy <= volInfo.maxROI.y && tempz >= volInfo.minROI.z && tempz <= volInfo.maxROI.z){
      
      temp=CUDAkernel_Interpolate<T>((T*)volInfo.SourceData, tempx, tempy, tempz);
      
      if(pos==-1 && temp >=volInfo.MinThreshold && temp <= volInfo.MaxThreshold){
        pos=temppos;
        
        if(!zBufferIsSet){
          
          CalculateZBuffer(renInfo, &zBuffer, s_clippingPoints[tempacc*2].x, s_clippingPoints[tempacc*2+1].x, tempx);
          
          if(zBuffer < renInfo.ZBuffer[(renInfo.ActualResolution.x-1-xIndex)+yIndex*renInfo.ActualResolution.x]){
            newZBuffer = zBuffer;
            zBufferIsSet=true;
          }else{
            pos=-1;
            break;
          }
        }
        break;
      }
      
    }
    temppos++;
    
  }
  
  renInfo.ZBuffer[renInfo.ActualResolution.x-1-xIndex+yIndex*renInfo.ActualResolution.x]=newZBuffer;
  
  if(pos!=-1){
    
    tempx = ( s_rayMap[tempacc*2].x+((int)s_minmaxTrace[tempacc].x+pos)*s_rayMap[tempacc*2+1].x);
    tempy = ( s_rayMap[tempacc*2].y+((int)s_minmaxTrace[tempacc].x+pos)*s_rayMap[tempacc*2+1].y);
    tempz = ( s_rayMap[tempacc*2].z+((int)s_minmaxTrace[tempacc].x+pos)*s_rayMap[tempacc*2+1].z);
    
    if(tempx >= 0 && tempx <= volInfo.VolumeSize.x-2 && tempy >= 1 && tempy <= volInfo.VolumeSize.y-2 && tempz>=1 && tempz <= volInfo.VolumeSize.z-2){
      shadeField.x = ((float)((T*)volInfo.SourceData)[(int)(__float2int_rn(tempz)*volInfo.VolumeSize.x*volInfo.VolumeSize.y+__float2int_rn(tempy)*volInfo.VolumeSize.x+__float2int_rn(tempx+1))]-(float)((T*)volInfo.SourceData)[(int)(__float2int_rn(tempz)*volInfo.VolumeSize.x*volInfo.VolumeSize.y+__float2int_rn(tempy)*volInfo.VolumeSize.x+__float2int_rn(tempx-1))]);
      shadeField.y = ((float)((T*)volInfo.SourceData)[(int)(__float2int_rn(tempz)*volInfo.VolumeSize.x*volInfo.VolumeSize.y+__float2int_rn(tempy+1)*volInfo.VolumeSize.x+__float2int_rn(tempx))]-(float)((T*)volInfo.SourceData)[(int)(__float2int_rn(tempz)*volInfo.VolumeSize.x*volInfo.VolumeSize.y+__float2int_rn(tempy-1)*volInfo.VolumeSize.x+__float2int_rn(tempx))]);
      shadeField.z = ((float)((T*)volInfo.SourceData)[(int)(__float2int_rn(tempz+1)*volInfo.VolumeSize.x*volInfo.VolumeSize.y+__float2int_rn(tempy)*volInfo.VolumeSize.x+__float2int_rn(tempx))]-(float)((T*)volInfo.SourceData)[(int)(__float2int_rn(tempz-1)*volInfo.VolumeSize.x*volInfo.VolumeSize.y+__float2int_rn(tempy)*volInfo.VolumeSize.x+__float2int_rn(tempx))]);
    }else{
      shadeField.x=0;
      shadeField.y=0;
      shadeField.z=0;
    }
    
    tempf = sqrt(SQR(shadeField.x) + SQR(shadeField.y) + SQR(shadeField.z));
    if(tempf!=0.0)tempf=1.0/tempf;
    
    shadeField.x = tempf * shadeField.x;
    shadeField.y = tempf * shadeField.y;
    shadeField.z = tempf * shadeField.z;
    
    float3 lightVec;
    
    lightVec=MatMul(volInfo.Transform, renInfo.CameraDirection, 0.0f);
    
    if(val==0.0){
      val = (shadeField.x*lightVec.x+shadeField.y*lightVec.y+shadeField.z*lightVec.z);
    }
    
    if(val<0)val=1;
    
    if(val<=1.0){
      renInfo.OutputImage[(int)(xIndex+yIndex*renInfo.ActualResolution.x)]=make_uchar4((unsigned char)( volInfo.colorLow.x+(volInfo.colorHigh.x-volInfo.colorLow.x)*val),
                                                                                       (unsigned char)( volInfo.colorLow.y+(volInfo.colorHigh.y-volInfo.colorLow.y)*val), 
                                                                                       (unsigned char)( volInfo.colorLow.z+(volInfo.colorHigh.z-volInfo.colorLow.z)*val), 
                                                                                       255);
    }else{
      renInfo.OutputImage[(int)(xIndex+yIndex*renInfo.ActualResolution.x)]=make_uchar4((unsigned char)val, (unsigned char)val, (unsigned char)val, 255 ); 
    }
    
  }
  else{
    renInfo.OutputImage[(int)(xIndex+yIndex*renInfo.ActualResolution.x)]=make_uchar4(0,0,0,0); 
  }
}

#endif
