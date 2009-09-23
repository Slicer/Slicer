#include "vtkVolumeRenderingLogic.h"
#include "vtkObjectFactory.h"
#include "vtkObject.h"
#include "vtkVolumeProperty.h"
#include "vtkImageData.h"
#include "vtkPointData.h"

#include "vtkMRMLVolumeRenderingParametersNode.h"

#include "vtkSlicerVolumeTextureMapper3D.h"
#include "vtkSlicerFixedPointVolumeRayCastMapper.h"
#include "vtkSlicerGPURayCastVolumeTextureMapper3D.h"
#include "vtkCudaVolumeMapper.h"

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
      vtkMRMLVolumeRenderingParametersNode *vrpNode = vtkMRMLVolumeRenderingParametersNode::New();
      this->MRMLScene->RegisterNodeClass(vrpNode);
      vrpNode->Delete();

      this->First = false;
    }
}

vtkMRMLVolumeRenderingParametersNode* vtkVolumeRenderingLogic::GetParametersNode()
{
  vtkMRMLVolumeRenderingParametersNode *node = NULL;
  if (this->MRMLScene)
    {
    node = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(0, "vtkMRMLVolumeRenderingParametersNode"));
    if (node == NULL)
      {
      node = vtkMRMLVolumeRenderingParametersNode::New();
      vtkMRMLVolumeRenderingParametersNode *snode = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(this->MRMLScene->AddNode(node));
      if (snode == node)
        {
        node->Delete();
        }
      node = snode;
      }
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkVolumeRenderingLogic::ProcessMRMLEvents(vtkObject *caller,
                                            unsigned long event,
                                            void *callData)
{
}


