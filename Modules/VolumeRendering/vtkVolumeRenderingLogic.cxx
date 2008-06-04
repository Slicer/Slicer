#include "vtkVolumeRenderingLogic.h"
#include "vtkObjectFactory.h"
#include "vtkObject.h"

#include "vtkMRMLVolumeRenderingNode.h"

bool vtkVolumeRenderingLogic::First = true;

vtkVolumeRenderingLogic::vtkVolumeRenderingLogic(void)
{
}

vtkVolumeRenderingLogic::~vtkVolumeRenderingLogic(void)
{
}
vtkVolumeRenderingLogic* vtkVolumeRenderingLogic::New()
{
 // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkVolumeRenderingLogic");
  if(ret)
    {
      return (vtkVolumeRenderingLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkVolumeRenderingLogic;
}
void vtkVolumeRenderingLogic::PrintSelf(std::ostream &os, vtkIndent indent)
{
    os<<indent<<"Print logic"<<endl;
}


void vtkVolumeRenderingLogic::SetMRMLScene(vtkMRMLScene *scene)
{
  vtkSlicerModuleLogic::SetMRMLScene(scene);
  this->RegisterNodes();
}
void vtkVolumeRenderingLogic::RegisterNodes()
{
  if (this->MRMLScene && this->First)
    {
      // :NOTE: 20050513 tgl: Guard this so it is only registered once.
      vtkMRMLVolumeRenderingNode *vrNode=vtkMRMLVolumeRenderingNode::New();
      this->MRMLScene->RegisterNodeClass(vrNode);
      vrNode->Delete();
      this->First = false;
    }
}
