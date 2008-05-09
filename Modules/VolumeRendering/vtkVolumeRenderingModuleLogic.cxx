#include "vtkVolumeRenderingModuleLogic.h"
#include "vtkObjectFactory.h"
#include "vtkObject.h"

#include "vtkMRMLVolumeRenderingNode.h"

bool vtkVolumeRenderingModuleLogic::First = true;

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


void vtkVolumeRenderingModuleLogic::SetMRMLScene(vtkMRMLScene *scene)
{
  vtkSlicerModuleLogic::SetMRMLScene(scene);

  // TODO I think this is needed for loadable module support
  //   tgl@rideside.net
//   if (this->First) {
//     // Guard this so it is only registered once.
//     vtkMRMLVolumeRenderingNode *vrNode=vtkMRMLVolumeRenderingNode::New();
//     scene->RegisterNodeClass(vrNode);
//     vrNode->Delete();
//     this->First = false;
//  }
}
