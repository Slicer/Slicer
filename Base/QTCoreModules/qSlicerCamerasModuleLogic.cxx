#include "qSlicerCamerasModuleLogic.h"

#include <vtkMRMLCameraNode.h>
#include <vtkMRMLViewNode.h>

//-----------------------------------------------------------------------------
struct qSlicerCamerasModuleLogicPrivate: public qCTKPrivate<qSlicerCamerasModuleLogic>
{
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_NO_ARG_CXX(qSlicerCamerasModuleLogic);

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleLogic::setup()
{
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleLogic::synchronizeCameraWithView( void * currentView )
{
  if( currentView == NULL )
    {
    return;
    }
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleLogic::setCameraToView(vtkMRMLCameraNode* camera, vtkMRMLViewNode* view)
{
  if (camera && view)
    {
    camera->SetActiveTag(view->GetID());
    }
}
