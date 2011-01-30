#include "vtkCudaVolumeInformationHandler.h"
#include "vtkObjectFactory.h"

//Volume and Property
#include "vtkVolumeProperty.h"
#include "vtkVolume.h"
#include "vtkColorTransferFunction.h"
#include "vtkPiecewiseFunction.h"
#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkMatrix4x4.h"

#include "cuda_runtime_api.h"
#include <cutil.h>
#include "vector_functions.h"

extern "C" {
#include "CUDA_renderBase.h"
}

vtkCxxRevisionMacro(vtkCudaVolumeInformationHandler, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkCudaVolumeInformationHandler);

vtkCudaVolumeInformationHandler::vtkCudaVolumeInformationHandler()
{
    this->SetSampleDistance(1.0);
    this->VolumeInfo.FunctionSize = 0;
    this->ResizeTransferFunction(256);

    this->Volume = NULL;
    this->InputData = NULL;

    this->SetThreshold(0, 255);    

    this->TransformationMatrix=vtkMatrix4x4::New();
    this->TransformationMatrix->Identity();
    this->OrientationMatrix=vtkMatrix4x4::New();
    this->OrientationMatrix->Identity();
 
    this->UpdateMatrix();

    this->VolumeInfo.InterpolationType=0;

}

vtkCudaVolumeInformationHandler::~vtkCudaVolumeInformationHandler()
{
    this->SetVolume(NULL);
    this->SetInputData(NULL);

    this->TransformationMatrix->Delete();
    this->OrientationMatrix->Delete();
}

void vtkCudaVolumeInformationHandler::ResizeTransferFunction(unsigned int size)
{
    if (size != this->VolumeInfo.FunctionSize)
    {
        this->VolumeInfo.FunctionSize = size;

        this->LocalAlphaTransferFunction.Allocate<float>(this->VolumeInfo.FunctionSize);
        this->LocalColorTransferFunction.Allocate<float3>(this->VolumeInfo.FunctionSize);
        this->LocalGradientTransferFunction.Allocate<float>(this->VolumeInfo.FunctionSize);

        this->Modified();
    }
}

void vtkCudaVolumeInformationHandler::SetVolume(vtkVolume* volume)
{
  if(volume!=this->Volume){
    this->Volume = volume;
    if (this->Volume != NULL)
      this->UpdateVolume();
    this->Modified();
  }
}

void vtkCudaVolumeInformationHandler::SetInputData(vtkImageData* inputData)
{
  if (inputData == NULL)
    {
      this->CudaInputBuffer.Free();
      this->GradientBuffer.Free();
      this->InputData = NULL;
    }
  else if (inputData != this->InputData)
    {
      this->InputData = inputData;
      
      this->UpdateImageData();
      this->Modified();
    }
}

/**
* @brief sets the threshold to min and max.
*/
void vtkCudaVolumeInformationHandler::SetThreshold(float min, float max)
{
  this->VolumeInfo.MinThreshold = min;
  this->VolumeInfo.MaxThreshold = max;
}

void vtkCudaVolumeInformationHandler::SetSampleDistance(float sampleDistance)
{ 
    if (sampleDistance <= 0.0f)
        sampleDistance = .1f;
    else
        this->VolumeInfo.SampleDistance = sampleDistance; 
}

float vtkCudaVolumeInformationHandler::GetSampleDistance(){
  return this->VolumeInfo.SampleDistance;
}

void vtkCudaVolumeInformationHandler::SetOrientationMatrix(vtkMatrix4x4* matrix){
  int i,j;
  for(i=0;i<4;i++){
    for(j=0;j<4;j++){
      this->OrientationMatrix->SetElement(i,j,matrix->GetElement(i,j));
    }
  }
  this->UpdateMatrix();
}

void vtkCudaVolumeInformationHandler::SetTransformationMatrix(vtkMatrix4x4* matrix){
  int i,j;
  for(i=0;i<4;i++){
    for(j=0;j<4;j++){
      this->TransformationMatrix->SetElement(i,j,matrix->GetElement(i,j));
    }
  }
  this->UpdateMatrix();
}

/**
* @brief Updates the transfer functions on local and global memory.
* @param property: The property that holds the transfer function information.
*/
void vtkCudaVolumeInformationHandler::UpdateVolumeProperties(vtkVolumeProperty *property)
{
  /*added here*/

  double range[2];
 
  this->InputData->GetPointData()->GetScalars()->GetRange(range);
  //range[0]=this->InputData->GetScalarTypeMin();
  //range[1]=this->InputData->GetScalarTypeMax();
  this->VolumeInfo.FunctionRange[0] = range[0];
  this->VolumeInfo.FunctionRange[1] = range[1];
  this->VolumeInfo.TypeRange[0] = range[0];
  this->VolumeInfo.TypeRange[1] = range[1];
  
  this->VolumeInfo.MinThreshold = range[0];
  this->VolumeInfo.MaxThreshold = range[1];
  
  property->GetRGBTransferFunction()->GetTable(this->VolumeInfo.FunctionRange[0], this->VolumeInfo.FunctionRange[1],
                                               this->VolumeInfo.FunctionSize, this->LocalColorTransferFunction.GetMemPointerAs<float>());
  
  property->GetScalarOpacity()->GetTable(this->VolumeInfo.FunctionRange[0], this->VolumeInfo.FunctionRange[1], 
                                         this->VolumeInfo.FunctionSize, this->LocalAlphaTransferFunction.GetMemPointerAs<float>());
  
  property->GetGradientOpacity()->GetTable(this->VolumeInfo.FunctionRange[0], this->VolumeInfo.FunctionRange[1], 
                                           this->VolumeInfo.FunctionSize, this->LocalGradientTransferFunction.GetMemPointerAs<float>());
  
  this->VolumeInfo.ColorTransferFunction = this->LocalColorTransferFunction.GetMemPointerAs<float>();
  this->VolumeInfo.AlphaTransferFunction = this->LocalAlphaTransferFunction.GetMemPointerAs<float>();
  this->VolumeInfo.GradientTransferFunction = this->LocalGradientTransferFunction.GetMemPointerAs<float>();
  
  this->VolumeInfo.Ambient = property->GetAmbient();
  this->VolumeInfo.Diffuse = property->GetDiffuse();
  this->VolumeInfo.Specular = property->GetSpecular();
  this->VolumeInfo.SpecularPower = property->GetSpecularPower();
  
  this->VolumeInfo.InterpolationType = property->GetInterpolationType();  
}

#include "vtkMatrix4x4.h"
void vtkCudaVolumeInformationHandler::UpdateVolume()
{
  if (this->Volume->GetProperty()->GetMTime() > this->GetMTime())
    this->UpdateVolumeProperties(this->Volume->GetProperty());

}

void vtkCudaVolumeInformationHandler::UpdateImageData()
{
    int* dims = this->InputData->GetDimensions();
    int* extent = this->InputData->GetExtent();

    this->VolumeInfo.MinROI=make_float3((float)extent[0], (float)extent[2], (float)extent[4]);
    this->VolumeInfo.MaxROI=make_float3((float)extent[1], (float)extent[3], (float)extent[5]);

    this->VolumeInfo.VolumeSize.x = dims[0];
    this->VolumeInfo.VolumeSize.y = dims[1];
    this->VolumeInfo.VolumeSize.z = dims[2];

    // needs precalculated data from above
    unsigned long size = this->InputData->GetScalarSize() * 
        this->VolumeInfo.VolumeSize.x *
        this->VolumeInfo.VolumeSize.y *
        this->VolumeInfo.VolumeSize.z *
        this->InputData->GetNumberOfScalarComponents();

    if (size != this->CudaInputBuffer.GetSize())
        this->CudaInputBuffer.AllocateBytes(size);

    this->CudaInputBuffer.CopyFrom(this->InputData->GetScalarPointer(),
        size);

    this->VolumeInfo.SourceData = this->CudaInputBuffer.GetMemPointer();
    this->VolumeInfo.InputDataType = this->InputData->GetScalarType();

    this->UpdateGradient();
}

void vtkCudaVolumeInformationHandler::UpdateMatrix(){
  vtkMatrix4x4 *invTrans=vtkMatrix4x4::New();
  vtkMatrix4x4 *invOri=vtkMatrix4x4::New();
  vtkMatrix4x4 *finalMat=vtkMatrix4x4::New();
  vtkMatrix4x4::Invert(this->TransformationMatrix, invTrans);
  vtkMatrix4x4::Invert(this->OrientationMatrix, invOri);
  vtkMatrix4x4::Multiply4x4(invOri, invTrans, finalMat);
  
  for (unsigned int i = 0; i < 4 ; i++){
    for (unsigned int j = 0; j < 4; j++){
      this->VolumeInfo.Transform[i][j]=finalMat->GetElement(i,j);
    }
  }
  
  for (unsigned int i = 0; i < 4 ; i++){
    for (unsigned int j = 0; j < 4; j++){
      this->VolumeInfo.OrientationMatrix[i][j]=this->OrientationMatrix->GetElement(i,j);
    }
  }

  invOri->Delete();
  invTrans->Delete();
  finalMat->Delete();
}

/**
* @brief Updates the volume information that is being sent to the Cuda Card.
*/
void vtkCudaVolumeInformationHandler::Update()
{
    if (this->Volume != NULL && this->InputData != NULL)
    {
      if (this->Volume->GetMTime() > this->GetMTime())
        this->UpdateVolume();
      
      if (this->InputData->GetMTime() > this->GetMTime())
        this->UpdateImageData();
      
      this->Modified();
    }
}

void vtkCudaVolumeInformationHandler::UpdateGradient(){
  unsigned long gradientSize = sizeof(float3) * 
    this->VolumeInfo.VolumeSize.x *
    this->VolumeInfo.VolumeSize.y *
    this->VolumeInfo.VolumeSize.z;
  
  if (gradientSize != this->GradientBuffer.GetSize())
    this->GradientBuffer.AllocateBytes(gradientSize);
  
  this->VolumeInfo.GradientData = this->GradientBuffer.GetMemPointerAs<float3>();
  
  CUDArenderBase_calculateGradient(this->VolumeInfo);
}

void vtkCudaVolumeInformationHandler::PrintSelf(std::ostream& os, vtkIndent indent)
{

}
