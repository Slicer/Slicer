#include "qSlicerCamerasModuleLogic.h"

#include <vtkMRMLCameraNode.h>
#include <vtkMRMLViewNode.h>

//-----------------------------------------------------------------------------
qSlicerCamerasModuleLogic::qSlicerCamerasModuleLogic()
{
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleLogic::setCameraToView(vtkMRMLCameraNode* camera, vtkMRMLViewNode* view)
{
  if (camera == 0 || view == 0)
    {
    return;
    }
  camera->SetActiveTag(view->GetID());
}
