#include "vtkVolumeRenderingCudaLogic.h"
#include "vtkObjectFactory.h"
#include "vtkObject.h"

vtkVolumeRenderingCudaLogic::vtkVolumeRenderingCudaLogic(void)
{
}

vtkVolumeRenderingCudaLogic::~vtkVolumeRenderingCudaLogic(void)
{
}
vtkVolumeRenderingCudaLogic* vtkVolumeRenderingCudaLogic::New()
{
 // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkVolumeRenderingCudaLogic");
  if(ret)
    {
      return (vtkVolumeRenderingCudaLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkVolumeRenderingCudaLogic;
}
void vtkVolumeRenderingCudaLogic::PrintSelf(std::ostream &os, vtkIndent indent)
{
    os<<indent<<"Print logic"<<endl;
}
