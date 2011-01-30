// Type
#include "vtkCudaVolumeMapper.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkCudaVolumeMapper, "$Revision: 1.8 $");
vtkStandardNewMacro(vtkCudaVolumeMapper);

vtkCudaVolumeMapper::vtkCudaVolumeMapper()
{
  this->CUDAEnabled = false;
}  

vtkCudaVolumeMapper::~vtkCudaVolumeMapper()
{

}

void vtkCudaVolumeMapper::SetInput(vtkImageData * input)
{

}

void vtkCudaVolumeMapper::SetRenderMode(int mode)
{

}

void vtkCudaVolumeMapper::SetRayCastingMethod(int mode)
{

}

void vtkCudaVolumeMapper::SetInterpolationMethod(int mode)
{

}

void vtkCudaVolumeMapper::SetOrientationMatrix(vtkMatrix4x4* matrix){

}

void vtkCudaVolumeMapper::SetTransformationMatrix(vtkMatrix4x4* matrix){

}

void vtkCudaVolumeMapper::SetImageData(vtkImageData* data){

}

int vtkCudaVolumeMapper::GetCurrentRenderMode() const
{
  return 42; // and that really compiled without the return? really? :)
}

void vtkCudaVolumeMapper::SetThreshold(float min, float max)
{

}

void vtkCudaVolumeMapper::SetSampleDistance(float sampleDistance)
{

}

void vtkCudaVolumeMapper::SetRenderOutputScaleFactor(float scaleFactor)
{

}

void vtkCudaVolumeMapper::ClippingOn(){

}

void vtkCudaVolumeMapper::ClippingOff(){

}

void vtkCudaVolumeMapper::ShadingOn(){

}

void vtkCudaVolumeMapper::ShadingOff(){

}

void vtkCudaVolumeMapper::Render(vtkRenderer *renderer, vtkVolume *volume)
{

}

void vtkCudaVolumeMapper::AdjustSampleDistance(float time){

}

void vtkCudaVolumeMapper::SetIntendedFrameRate(float frameRate){

}

void vtkCudaVolumeMapper::PrintSelf(ostream& os, vtkIndent indent)
{

}
