#ifndef NTKCUDA_RAYCASTCOMPOSITESHADED_H
#define NTKCUDA_RAYCASTCOMPOSITESHADED_H

#include "CUDA_matrix_math.h"
#include "CUDA_zbuffer_math.h"

#define SQR(X) ((X) * (X) )

template <typename T>
__device__ void CUDAkernel_RayCastCompositeShaded(cudaRendererInformation& renInfo,
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
  float3 shadeField;
  float specular;
  float val;
  float tempf;
  
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
    
    if(zBuffer >= renInfo.ZBuffer[(renInfo.ActualResolution.x-1-xIndex)+yIndex*renInfo.ActualResolution.x]){
      break;
    }

    if(tempx >= volInfo.minROI.x && tempx <= volInfo.maxROI.x && tempy >= volInfo.minROI.y && tempy <= volInfo.maxROI.y && tempz >= volInfo.minROI.z && tempz <= volInfo.maxROI.z){
      
      temp=CUDAkernel_Interpolate<T>((T*)volInfo.SourceData, tempx, tempy, tempz);        
      
      if(tempx >= 0 && tempx <= volInfo.VolumeSize.x-2 && tempy >= 1 && tempy <= volInfo.VolumeSize.y-2 && tempz>=1 && tempz <= volInfo.VolumeSize.z-2){
        shadeField.x = ((float)((T*)volInfo.SourceData)[(int)(__float2int_rn(tempz)*volInfo.VolumeSize.x*volInfo.VolumeSize.y+__float2int_rn(tempy)*volInfo.VolumeSize.x+__float2int_rn(tempx+1))]-(float)((T*)volInfo.SourceData)[(int)(__float2int_rn(tempz)*volInfo.VolumeSize.x*volInfo.VolumeSize.y+__float2int_rn(tempy)*volInfo.VolumeSize.x+__float2int_rn(tempx-1))]);
        shadeField.y = ((float)((T*)volInfo.SourceData)[(int)(__float2int_rn(tempz)*volInfo.VolumeSize.x*volInfo.VolumeSize.y+__float2int_rn(tempy+1)*volInfo.VolumeSize.x+__float2int_rn(tempx))]-(float)((T*)volInfo.SourceData)[(int)(__float2int_rn(tempz)*volInfo.VolumeSize.x*volInfo.VolumeSize.y+__float2int_rn(tempy-1)*volInfo.VolumeSize.x+__float2int_rn(tempx))]);
        shadeField.z = ((float)((T*)volInfo.SourceData)[(int)(__float2int_rn(tempz+1)*volInfo.VolumeSize.x*volInfo.VolumeSize.y+__float2int_rn(tempy)*volInfo.VolumeSize.x+__float2int_rn(tempx))]-(float)((T*)volInfo.SourceData)[(int)(__float2int_rn(tempz-1)*volInfo.VolumeSize.x*volInfo.VolumeSize.y+__float2int_rn(tempy)*volInfo.VolumeSize.x+__float2int_rn(tempx))]);
      }else{
        shadeField.x=0;
        shadeField.y=0;
        shadeField.z=0;
      }
      
      int index = (int)(__float2int_rn(tempz)*volInfo.VolumeSize.x*volInfo.VolumeSize.y+__float2int_rn(tempy)*volInfo.VolumeSize.x+__float2int_rn(tempx));
      
      tempf = sqrt(SQR(shadeField.x) + SQR(shadeField.y) + SQR(shadeField.z));
      
      specular=tempf/((float)(volInfo.TypeRange[1]-(float)volInfo.TypeRange[0])*sqrtf(3));
      
      specular=log10f((specular*9)+1);
      
      if(tempf!=0){ 
        float tempf2=1.0/tempf;
        shadeField.x *= tempf2;
        shadeField.y *= tempf2;
        shadeField.z *= tempf2;
      }else{
        shadeField.x = 0;
        shadeField.y = 0;
        shadeField.z = 0;
      }
      
      
      float3 lightVec;
      lightVec=MatMul(volInfo.Transform, renInfo.CameraDirection, 0.0f);
      
      val = (shadeField.x*lightVec.x+shadeField.y*lightVec.y+shadeField.z*lightVec.z);
      
      if(val<0)val=-val;
      
      if(temp>maxVal)maxVal=temp;
      
      if(temp >=volInfo.MinThreshold && temp <= volInfo.MaxThreshold){
        
        temp=(temp-volInfo.TypeRange[0])*(volInfo.FunctionSize-1)/(volInfo.TypeRange[1]-volInfo.TypeRange[0]);
        
        alpha=alphaTF[(int)temp];
        r=colorTF[(int)temp*3];
        g=colorTF[(int)temp*3+1];
        b=colorTF[(int)temp*3+2];
        
        if(remainingOpacity>0.02){
          if(!zBufferIsSet){
            newZBuffer = zBuffer;
            zBufferIsSet=true;
          }
          outputVal[0]+=remainingOpacity*alpha*((1.0-specular)*r+specular*val);
          outputVal[1]+=remainingOpacity*alpha*((1.0-specular)*g+specular*val);
          outputVal[2]+=remainingOpacity*alpha*((1.0-specular)*b+specular*val);
          remainingOpacity*=(1.0-alpha);
          
        }else{
          pos = s_minmaxTrace[tempacc].y-s_minmaxTrace[tempacc].x;
        }
        
        
      }
      
    }
    pos+=1.0;
  }
  
  renInfo.ZBuffer[renInfo.ActualResolution.x-1-xIndex+yIndex*renInfo.ActualResolution.x]=newZBuffer;
  
  renInfo.OutputImage[(int)(xIndex+yIndex*renInfo.ActualResolution.x)]=make_uchar4((outputVal[0]*255.0), 
                                                                                   (outputVal[1]*255.0), 
                                                                                   (outputVal[2]*255.0), 
                                                                                   (1-remainingOpacity)*255.0);
  
  
}

#endif
