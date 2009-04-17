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
                                                  float* gradientTF,
                                                  float3* s_rayMap,
                                                  float2* s_minmaxTrace,
                                                  float3* s_clippingPoints,
                                                  float* s_rayLength,
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
  float val;
  
  outputVal[0]=0;
  outputVal[1]=0;
  outputVal[2]=0;
  
  float newZBuffer;
  
  bool zBufferIsSet=false;

  float ZReverseX;
  float newZReverseX;
  float ZReverseY;
  float newZReverseY;
  float ZReverseZ;
  float newZReverseZ;

  CalculateReverseZBuffer(renInfo, &ZReverseX, s_clippingPoints[tempacc*2].x, s_clippingPoints[tempacc*2+1].x, renInfo.ZBuffer[(renInfo.ActualResolution.x-1-xIndex)+yIndex*renInfo.ActualResolution.x]);
  
  CalculateReverseZBuffer(renInfo, &ZReverseY, s_clippingPoints[tempacc*2].y, s_clippingPoints[tempacc*2+1].y, renInfo.ZBuffer[(renInfo.ActualResolution.x-1-xIndex)+yIndex*renInfo.ActualResolution.x]);
  
  CalculateReverseZBuffer(renInfo, &ZReverseZ, s_clippingPoints[tempacc*2].z, s_clippingPoints[tempacc*2+1].z, renInfo.ZBuffer[(renInfo.ActualResolution.x-1-xIndex)+yIndex*renInfo.ActualResolution.x]);

  while((s_minmaxTrace[tempacc].y-s_minmaxTrace[tempacc].x)>=pos){
    
    tempx = ( s_rayMap[tempacc*2].x+(s_minmaxTrace[tempacc].x+pos)*s_rayMap[tempacc*2+1].x);
    tempy = ( s_rayMap[tempacc*2].y+(s_minmaxTrace[tempacc].x+pos)*s_rayMap[tempacc*2+1].y);
    tempz = ( s_rayMap[tempacc*2].z+(s_minmaxTrace[tempacc].x+pos)*s_rayMap[tempacc*2+1].z);

    if(s_rayMap[tempacc*2+1].x>0){
      if(tempx >= ZReverseX){
        break;
      }
    }else if(s_rayMap[tempacc*2+1].x<0){
      if(tempx <= ZReverseX){
        break;
      }
    }else{
      if(s_rayMap[tempacc*2+1].y>0){
        if(tempy >= ZReverseY){
          break;
        }
      }else if(s_rayMap[tempacc*2+1].y<0){
        if(tempy <= ZReverseY){
          break;
        }
      }else{
        if(s_rayMap[tempacc*2+1].z>0){
          if(tempz >= ZReverseZ){
            break;
          }
          }else if(s_rayMap[tempacc*2+1].z<0){
          if(tempz <= ZReverseZ){
            break;
          }
        }
      }
    }
        
    if(tempx >= volInfo.MinROI.x && tempx <= volInfo.MaxROI.x && tempy >= volInfo.MinROI.y && tempy <= volInfo.MaxROI.y && tempz >= volInfo.MinROI.z && tempz <= volInfo.MaxROI.z){
      
      temp=CUDAkernel_Interpolate<T>((T*)volInfo.SourceData, tempx, tempy, tempz);        
      if(temp>maxVal)maxVal=temp;
      
      if(temp >=volInfo.MinThreshold && temp <= volInfo.MaxThreshold){
        
        temp=(temp-volInfo.TypeRange[0])*(volInfo.FunctionSize-1)/(volInfo.TypeRange[1]-volInfo.TypeRange[0]);
        
        alpha=alphaTF[(int)temp]*s_rayLength[tempacc]*volInfo.SampleDistance;
        if(alpha>1)alpha=1;
        
        if(remainingOpacity>0.02){
          if(!zBufferIsSet){
            newZReverseX = tempx;
            newZReverseY = tempy;
            newZReverseZ = tempz;
            zBufferIsSet=true;
          }
          
          if(alpha>0){
            shadeField = CUDAkernel_Interpolate2(volInfo.GradientData, tempx, tempy, tempz);        
            
            float3 lightVec;
            lightVec=MatMul(volInfo.Transform, renInfo.CameraDirection, 0.0f);
            
            val = (shadeField.x*lightVec.x+shadeField.y*lightVec.y+shadeField.z*lightVec.z);
            
            if(val<0)val=-val;
            if(val>1)val=1;

            r=colorTF[(int)temp*3];
            g=colorTF[(int)temp*3+1];
            b=colorTF[(int)temp*3+2];
            
            float valpow = pow(val, volInfo.SpecularPower);
            //float grad = gradientTF[(int)valpow*(volInfo.FunctionSize-1)];

            float factor = 1.0/(volInfo.Specular+volInfo.Ambient+volInfo.Diffuse);
            
            outputVal[0]+=remainingOpacity*factor*alpha*((volInfo.Ambient+volInfo.Diffuse*val)*r+volInfo.Specular*valpow);
            outputVal[1]+=remainingOpacity*factor*alpha*((volInfo.Ambient+volInfo.Diffuse*val)*g+volInfo.Specular*valpow);
            outputVal[2]+=remainingOpacity*factor*alpha*((volInfo.Ambient+volInfo.Diffuse*val)*b+volInfo.Specular*valpow);
            remainingOpacity*=(1.0-alpha);
          }
        }else{
          pos = s_minmaxTrace[tempacc].y-s_minmaxTrace[tempacc].x;
        }
      }
      
    }
    pos+=volInfo.SampleDistance;
  }
  
  if(s_rayMap[tempacc*2+1].x!=0)
    CalculateZBuffer(renInfo, &newZBuffer, s_clippingPoints[tempacc*2].x, s_clippingPoints[tempacc*2+1].x, newZReverseX);
  else if(s_rayMap[tempacc*2+1].y!=0){
    CalculateZBuffer(renInfo, &newZBuffer, s_clippingPoints[tempacc*2].y, s_clippingPoints[tempacc*2+1].y, newZReverseY);
  }else{
    CalculateZBuffer(renInfo, &newZBuffer, s_clippingPoints[tempacc*2].z, s_clippingPoints[tempacc*2+1].z, newZReverseZ);
  }

  renInfo.ZBuffer[renInfo.ActualResolution.x-1-xIndex+yIndex*renInfo.ActualResolution.x]=newZBuffer;

  if(outputVal[0]>1.0)outputVal[0]=1.0;
  if(outputVal[1]>1.0)outputVal[1]=1.0;
  if(outputVal[2]>1.0)outputVal[2]=1.0;
  
  renInfo.OutputImage[(int)(xIndex+yIndex*renInfo.ActualResolution.x)]=make_uchar4((outputVal[0]*255.0), 
                                                                                   (outputVal[1]*255.0), 
                                                                                   (outputVal[2]*255.0), 
                                                                                   (1-remainingOpacity)*255.0);
  
  
}

#endif
