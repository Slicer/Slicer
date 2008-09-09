// includes, system
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// includes, project
#include <cutil.h>

// includes, kernels

#include "CUDA_renderSlice.h"
#include "CUDA_matrix_math.h"
#include "CUDA_zbuffer_math.h"

#include "vtkType.h"

//#define USE_TIMER
#define BLOCK_DIM2D 8 // this must be set to 4 or more
#define ACC(X,Y,Z) ( ( (Z)*(sizeX)*(sizeY) ) + ( (Y)*(sizeX) ) + (X) )
#define SQR(X) ((X) * (X) )

__constant__ cudaRendererInformation cRenInfo;
__constant__ cudaVolumeInformation cVolInfo;

__constant__ float colorTF[256*3];
__constant__ float alphaTF[256];

template <typename T>
__device__ T CUDAkernel_InterpolateNN(T * sourceData,
				      float posX, 
				      float posY, 
				      float posZ){
  return sourceData[(int)(__float2int_rn(posZ)*cVolInfo.VolumeSize.x*cVolInfo.VolumeSize.y+__float2int_rn(posY)*cVolInfo.VolumeSize.x+__float2int_rn(posX))];
}

template <typename T>
__device__ T CUDAkernel_InterpolateTrilinear(T * sourceData,
					     float posX, 
					     float posY, 
					     float posZ){
  
  
  float fracX=posX-(int)posX;
  float fracY=posY-(int)posY;
  float fracZ=posZ-(int)posZ;
  
  float revX=1-fracX;
  float revY=1-fracY;
  float revZ=1-fracZ;
  
  int base=(int)((int)(posZ)*cVolInfo.VolumeSize.x*cVolInfo.VolumeSize.y+(int)(posY)*cVolInfo.VolumeSize.x+(int)(posX));
  
  return ((T) (revX*(revY*(revZ* (sourceData)[(int)(base)]+
			   fracZ* (sourceData)[(int)(base+cVolInfo.VolumeSize.x*cVolInfo.VolumeSize.y)])+
		     fracY*(revZ* (sourceData)[(int)(base+cVolInfo.VolumeSize.x)]+
			    fracZ* (sourceData)[(int)(base+cVolInfo.VolumeSize.x*cVolInfo.VolumeSize.y+cVolInfo.VolumeSize.x)]))+
	       fracX*(revY*(revZ* (sourceData)[(int)(base+1)]+
			    fracZ* (sourceData)[(int)(base+cVolInfo.VolumeSize.x*cVolInfo.VolumeSize.y+1)])+
		      fracY*(revZ* (sourceData)[(int)(base+cVolInfo.VolumeSize.x+1)]+
			     fracZ* (sourceData)[(int)(base+cVolInfo.VolumeSize.x*cVolInfo.VolumeSize.y+cVolInfo.VolumeSize.x+1)])))
	  );
}

template <typename T>
__device__ T CUDAkernel_Interpolate(T * sourceData,
				    float posX, 
				    float posY, 
				    float posZ){
  if(cRenInfo.interpolationMethod == 0){
    return CUDAkernel_InterpolateNN(sourceData, posX, posY, posZ);
  }else if(cRenInfo.interpolationMethod == 1){
    return CUDAkernel_InterpolateTrilinear(sourceData, posX, posY, posZ);
  }
  return 0;
}

__device__ void CUDAkernel_renderSlice_SetRayMapVolumeRendering(long int base, float3* rayMap, float3* clippingPoints, long int index, float* lensMap, int xIndex, int yIndex){
  
  float3 start;
  float3 end;

  start.x=cRenInfo.CameraRayStart.x+
    (float)xIndex/(cRenInfo.ActualResolution.x-1)*cRenInfo.CameraRayStartX.x+
    (float)yIndex/(cRenInfo.ActualResolution.y-1)*cRenInfo.CameraRayStartY.x;
  start.y=cRenInfo.CameraRayStart.y+
    (float)xIndex/(cRenInfo.ActualResolution.x-1)*cRenInfo.CameraRayStartX.y+
    (float)yIndex/(cRenInfo.ActualResolution.y-1)*cRenInfo.CameraRayStartY.y;
  start.z=cRenInfo.CameraRayStart.z+
    (float)xIndex/(cRenInfo.ActualResolution.x-1)*cRenInfo.CameraRayStartX.z+
    (float)yIndex/(cRenInfo.ActualResolution.y-1)*cRenInfo.CameraRayStartY.z;

  end.x=cRenInfo.CameraRayEnd.x+
    (float)xIndex/(cRenInfo.ActualResolution.x-1)*cRenInfo.CameraRayEndX.x+
    (float)yIndex/(cRenInfo.ActualResolution.y-1)*cRenInfo.CameraRayEndY.x;
  end.y=cRenInfo.CameraRayEnd.y+
    (float)xIndex/(cRenInfo.ActualResolution.x-1)*cRenInfo.CameraRayEndX.y+
    (float)yIndex/(cRenInfo.ActualResolution.y-1)*cRenInfo.CameraRayEndY.y;
  end.z=cRenInfo.CameraRayEnd.z+
    (float)xIndex/(cRenInfo.ActualResolution.x-1)*cRenInfo.CameraRayEndX.z+
    (float)yIndex/(cRenInfo.ActualResolution.y-1)*cRenInfo.CameraRayEndY.z;

  rayMap[base*2].x=start.x;
  rayMap[base*2].y=start.y;
  rayMap[base*2].z=start.z;

  rayMap[base*2+1].x=end.x-start.x;
  rayMap[base*2+1].y=end.y-start.y;
  rayMap[base*2+1].z=end.z-start.z;

  rayMap[base*2]=MatMul(cVolInfo.Transform, rayMap[base*2]);
  rayMap[base*2+1]=MatMul(cVolInfo.Transform, rayMap[base*2+1], 0.0f);

  clippingPoints[base*2].x=rayMap[base*2].x;
  clippingPoints[base*2].y=rayMap[base*2].y;
  clippingPoints[base*2].z=rayMap[base*2].z;

  clippingPoints[base*2+1].x=rayMap[base*2].x+rayMap[base*2+1].x;
  clippingPoints[base*2+1].y=rayMap[base*2].y+rayMap[base*2+1].y;
  clippingPoints[base*2+1].z=rayMap[base*2].z+rayMap[base*2+1].z;

  float getmax = fabs(rayMap[base*2+1].x);
  if(fabs(rayMap[base*2+1].y)>getmax) getmax = fabs(rayMap[base*2+1].y);
  if(fabs(rayMap[base*2+1].z)>getmax) getmax = fabs(rayMap[base*2+1].z);
  
  if(getmax!=0){
    float temp= 1.0f/getmax;
    rayMap[base*2+1].x*=temp;
    rayMap[base*2+1].y*=temp;
    rayMap[base*2+1].z*=temp;
  }
}

__device__ void CUDAkernel_renderSlice_CalculateMinmax(long int tempacc, float3* rayMap, float2* minmax, int xindex, int yindex){

  float3 zVec;
  float3 normalVec;
  float3 newOrigin;

  zVec.x=0;
  zVec.y=0;
  zVec.z=1;

  normalVec=MatMul(cVolInfo.SliceMatrix, zVec, 0.0f);
  newOrigin=MatMul(cVolInfo.SliceMatrix, make_float3(0,0,0), 1.0f);

  float3 transformedOrigin;
  float3 transformedVector;

  transformedVector=MatMul(cVolInfo.OrientationMatrix, rayMap[2*tempacc+1], 0.0f);
  transformedOrigin=MatMul(cVolInfo.OrientationMatrix, rayMap[2*tempacc], 1.0f);
  
  float length=sqrt(normalVec.x*normalVec.x+
		    normalVec.y*normalVec.y+
		    normalVec.z*normalVec.z);
  
  normalVec.x/=length;
  normalVec.y/=length;
  normalVec.z/=length;
  
  float3 relPos;
  
  relPos.x=newOrigin.x-transformedOrigin.x;
  relPos.y=newOrigin.y-transformedOrigin.y;
  relPos.z=newOrigin.z-transformedOrigin.z;

  float unit=(relPos.x*normalVec.x+
	      relPos.y*normalVec.y+
	      relPos.z*normalVec.z)/(
				     transformedVector.x*normalVec.x+
				     transformedVector.y*normalVec.y+
				     transformedVector.z*normalVec.z);

  
  minmax[tempacc].x=unit;
  minmax[tempacc].y=unit;
}

template <typename T>
__global__ void CUDAkernel_renderSlice_calculateShadeField()
{
  int xIndex = (blockDim.x*blockIdx.x + threadIdx.x) % (int)cVolInfo.VolumeSize.x;
  int yIndex = (blockDim.x*blockIdx.x + threadIdx.x) / (int)cVolInfo.VolumeSize.x;
  int zIndex = blockDim.y*blockIdx.y+ threadIdx.y;

  long int index = (xIndex+yIndex*cVolInfo.VolumeSize.x+zIndex*cVolInfo.VolumeSize.x*cVolInfo.VolumeSize.y);

  float3 tempShade;

  if(xIndex>0 && xIndex < cVolInfo.VolumeSize.x-1 && yIndex>0 && yIndex < cVolInfo.VolumeSize.y-1 && zIndex>0 && zIndex < cVolInfo.VolumeSize.z-1){
    tempShade.x = (float)((T*)cVolInfo.SourceData)[(int)(__float2int_rn(zIndex)*cVolInfo.VolumeSize.x*cVolInfo.VolumeSize.y+__float2int_rn(yIndex)*cVolInfo.VolumeSize.x+__float2int_rn(xIndex+1))];
    tempShade.y = (float)((T*)cVolInfo.SourceData)[(int)(__float2int_rn(zIndex)*cVolInfo.VolumeSize.x*cVolInfo.VolumeSize.y+__float2int_rn(yIndex+1)*cVolInfo.VolumeSize.x+__float2int_rn(xIndex))];
    tempShade.z = (float)((T*)cVolInfo.SourceData)[(int)(__float2int_rn(zIndex+1)*cVolInfo.VolumeSize.x*cVolInfo.VolumeSize.y+__float2int_rn(yIndex)*cVolInfo.VolumeSize.x+__float2int_rn(xIndex))];

    tempShade.x-=(float)((T*)cVolInfo.SourceData)[(int)(__float2int_rn(zIndex)*cVolInfo.VolumeSize.x*cVolInfo.VolumeSize.y+__float2int_rn(yIndex)*cVolInfo.VolumeSize.x+__float2int_rn(xIndex-1))];
    tempShade.y-=(float)((T*)cVolInfo.SourceData)[(int)(__float2int_rn(zIndex)*cVolInfo.VolumeSize.x*cVolInfo.VolumeSize.y+__float2int_rn(yIndex-1)*cVolInfo.VolumeSize.x+__float2int_rn(xIndex))];
    tempShade.z-=(float)((T*)cVolInfo.SourceData)[(int)(__float2int_rn(zIndex-1)*cVolInfo.VolumeSize.x*cVolInfo.VolumeSize.y+__float2int_rn(yIndex)*cVolInfo.VolumeSize.x+__float2int_rn(xIndex))];
  }else if((xIndex==0 || xIndex == cVolInfo.VolumeSize.x-1) && (yIndex==0 || yIndex == cVolInfo.VolumeSize.y-1) && (zIndex==0 || zIndex == cVolInfo.VolumeSize.z-1)){
    
    tempShade.x=0;
    tempShade.y=0;
    tempShade.z=0;
  }else{
    index=-1;
  }
  
  if(index!=-1){
    cVolInfo.shadeField[index].x=tempShade.x;
    cVolInfo.shadeField[index].y=tempShade.y;
    cVolInfo.shadeField[index].z=tempShade.z;
  }
}

template <typename T>
__global__ void CUDAkernel_renderSlice_doRendering()
{
  
  int xIndex = blockDim.x *blockIdx.x + threadIdx.x;
  int yIndex = blockDim.y *blockIdx.y + threadIdx.y;
  
  __shared__ float2 s_minmaxTrace[BLOCK_DIM2D*BLOCK_DIM2D];
  __shared__ float3 s_rayMap[BLOCK_DIM2D*BLOCK_DIM2D*2];
  __shared__ float3 s_clippingPoints[BLOCK_DIM2D*BLOCK_DIM2D*2];
  
  int tempacc=threadIdx.x+threadIdx.y*BLOCK_DIM2D;
  
  __syncthreads();
  
  long int index = (xIndex+yIndex*cRenInfo.ActualResolution.x)*4;
  
  if(xIndex<cRenInfo.ActualResolution.x && yIndex <cRenInfo.ActualResolution.y){  
    CUDAkernel_renderSlice_SetRayMapVolumeRendering(tempacc, s_rayMap, s_clippingPoints, index, cRenInfo.LensMap, xIndex, yIndex);
        
    CUDAkernel_renderSlice_CalculateMinmax(tempacc, s_rayMap, s_minmaxTrace, xIndex, yIndex);
    
    float tempx, tempy, tempz;
    float temp;
    float r=0;
    float g=0;
    float b=0;
    float alpha=0;

    float zBuffer;
    float newZBuffer = cRenInfo.ZBuffer[cRenInfo.ActualResolution.x-1-xIndex+yIndex*cRenInfo.ActualResolution.x];
    
    tempx = ( s_rayMap[tempacc*2].x+((int)s_minmaxTrace[tempacc].x)*s_rayMap[tempacc*2+1].x);
    tempy = ( s_rayMap[tempacc*2].y+((int)s_minmaxTrace[tempacc].x)*s_rayMap[tempacc*2+1].y);
    tempz = ( s_rayMap[tempacc*2].z+((int)s_minmaxTrace[tempacc].x)*s_rayMap[tempacc*2+1].z);
    
    if(tempx >= cVolInfo.minROI.x+1 && tempx < cVolInfo.maxROI.x-1 && tempy >= cVolInfo.minROI.y+1 && tempy < cVolInfo.maxROI.y-1 && tempz >= cVolInfo.minROI.z+1 && tempz < cVolInfo.maxROI.z-1){
      
      CalculateZBuffer(cRenInfo, &zBuffer, s_clippingPoints[tempacc*2].x, s_clippingPoints[tempacc*2+1].x, tempx);

      temp=CUDAkernel_Interpolate((T*)cVolInfo.SourceData, tempx, tempy, tempz);

      if(temp >=cVolInfo.MinThreshold && temp <= cVolInfo.MaxThreshold){
	
	if(zBuffer < cRenInfo.ZBuffer[cRenInfo.ActualResolution.x-1-xIndex+yIndex*cRenInfo.ActualResolution.x]){
	  temp=(float)(temp-cVolInfo.TypeRange[0])/(float)(cVolInfo.TypeRange[1]-cVolInfo.TypeRange[0])*(cVolInfo.FunctionSize-1);
	  
	  r=colorTF[(int)temp*3]* 255.0;
	  g=colorTF[(int)temp*3+1]* 255.0;
	  b=colorTF[(int)temp*3+2]* 255.0;
	  alpha=255.0;
	  
	  newZBuffer = zBuffer;
	}
      }  
    }
    
    cRenInfo.ZBuffer[cRenInfo.ActualResolution.x-1-xIndex+yIndex*cRenInfo.ActualResolution.x]=newZBuffer;
    cRenInfo.OutputImage[(int)(xIndex+yIndex*cRenInfo.ActualResolution.x)]=make_uchar4(r, g, b, alpha);
  }
  
}

void CUDArenderSlice_doRender(cudaRendererInformation& renInfo, cudaVolumeInformation& volInfo)
{
  // setup execution parameters

  int blockX=(((int)renInfo.ActualResolution.x-1)/ BLOCK_DIM2D) + 1;
  int blockY=(((int)renInfo.ActualResolution.y-1)/ BLOCK_DIM2D) + 1;
  
  // setup execution parameters
  
  dim3 grid(blockX, blockY, 1);
  dim3 threads(BLOCK_DIM2D, BLOCK_DIM2D, 1);

  blockX=((int)(volInfo.VolumeSize.x*volInfo.VolumeSize.y-1)/ BLOCK_DIM2D) + 1;
  blockY=((int)(volInfo.VolumeSize.z-1)/ BLOCK_DIM2D) + 1;

  dim3 grid2(blockX, blockY, 1);
  dim3 threads2(BLOCK_DIM2D, BLOCK_DIM2D, 1);

  // copy host memory to device

  prepareShadeFieldSlice(renInfo, volInfo);

  CUDA_SAFE_CALL( cudaMemcpyToSymbol(cRenInfo, &renInfo, sizeof(cudaRendererInformation)));
  CUDA_SAFE_CALL( cudaMemcpyToSymbol(cVolInfo, &volInfo, sizeof(cudaVolumeInformation)));

  CUDA_SAFE_CALL( cudaMemcpyToSymbol(colorTF, volInfo.ColorTransferFunction, sizeof(float)*256*3));
  CUDA_SAFE_CALL( cudaMemcpyToSymbol(alphaTF, volInfo.AlphaTransferFunction, sizeof(float)*256));

  //execute the kernel
  if(renInfo.rayCastingMethod==3){ //if CompositeShaded
#define CALCULATE_SHADE_FIELD_SLICE(ID, TYPE)					\
    (ID==volInfo.InputDataType)						\
      CUDAkernel_renderSlice_calculateShadeField<TYPE><<<grid2, threads2>>>()

    if CALCULATE_SHADE_FIELD_SLICE(VTK_CHAR, char);
    else if CALCULATE_SHADE_FIELD_SLICE(VTK_CHAR, char);
    else if CALCULATE_SHADE_FIELD_SLICE(VTK_UNSIGNED_CHAR, unsigned char);
    else if CALCULATE_SHADE_FIELD_SLICE(VTK_SHORT, short);
    else if CALCULATE_SHADE_FIELD_SLICE(VTK_UNSIGNED_SHORT, unsigned short);
    else if CALCULATE_SHADE_FIELD_SLICE(VTK_INT, int);
    else if CALCULATE_SHADE_FIELD_SLICE(VTK_FLOAT, float);
    
    //CUDAkernel_renderSlice_calculateShadeField<unsigned char><<<grid2, threads2>>>();
  }

#define CALL_KERNEL_DO_RENDER(ID, TYPE)					\
  (ID==volInfo.InputDataType)						\
    CUDAkernel_renderSlice_doRendering<TYPE><<<grid, threads>>>()
  
  if CALL_KERNEL_DO_RENDER(VTK_CHAR, char);
  else if CALL_KERNEL_DO_RENDER(VTK_CHAR, char);
  else if CALL_KERNEL_DO_RENDER(VTK_UNSIGNED_CHAR, unsigned char);
  else if CALL_KERNEL_DO_RENDER(VTK_SHORT, short);
  else if CALL_KERNEL_DO_RENDER(VTK_UNSIGNED_SHORT, unsigned short);
  else if CALL_KERNEL_DO_RENDER(VTK_INT, int);
  else if CALL_KERNEL_DO_RENDER(VTK_FLOAT, float);

  deleteShadeFieldSlice(renInfo, volInfo);

  CUT_CHECK_ERROR("Kernel execution failed");

  return;
}

void prepareShadeFieldSlice(cudaRendererInformation& renInfo, cudaVolumeInformation& volInfo){
  CUDA_SAFE_CALL(cudaMalloc((void**)&volInfo.shadeField, (int)(volInfo.VolumeSize.x*volInfo.VolumeSize.y*volInfo.VolumeSize.z*sizeof(float3))));
}

void deleteShadeFieldSlice(cudaRendererInformation& renInfo, cudaVolumeInformation& volInfo){
  CUDA_SAFE_CALL( cudaFree(volInfo.shadeField));
}


