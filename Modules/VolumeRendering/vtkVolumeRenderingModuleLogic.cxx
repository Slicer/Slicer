#include "vtkVolumeRenderingModuleLogic.h"
#include "vtkObjectFactory.h"
#include "vtkObject.h"

vtkVolumeRenderingModuleLogic::vtkVolumeRenderingModuleLogic(void)
{
}

vtkVolumeRenderingModuleLogic::~vtkVolumeRenderingModuleLogic(void)
{
}
vtkVolumeRenderingModuleLogic* vtkVolumeRenderingModuleLogic::New()
{
 // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkVolumeRenderingModuleLogic");
  if(ret)
    {
      return (vtkVolumeRenderingModuleLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkVolumeRenderingModuleLogic;
}
void vtkVolumeRenderingModuleLogic::PrintSelf(std::ostream &os, vtkIndent indent)
{
    os<<indent<<"Print logic"<<endl;
}
