#include "qSlicerCamerasModule.h"
#include "ui_qSlicerCamerasModule.h"

#include "vtkMRMLViewNode.h"
#include "vtkMRMLCameraNode.h"

#include <vector>

//-----------------------------------------------------------------------------
struct qSlicerCamerasModule::qInternal : public Ui::qSlicerCamerasModule
{
};

//-----------------------------------------------------------------------------
qSlicerCxxInternalConstructor1Macro(qSlicerCamerasModule, QWidget*);
qSlicerCxxDestructorMacro(qSlicerCamerasModule);

//-----------------------------------------------------------------------------
void qSlicerCamerasModule::initializer()
{
  this->Superclass::initializer();
  Q_ASSERT(this->Internal != 0);

  this->Internal->setupUi(this);

  connect(this->Internal->ViewNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onCurrentViewNodeChanged(vtkMRMLNode*)));
  connect(this->Internal->CameraNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onCurrentCameraNodeChanged(vtkMRMLNode*)));
  connect(this->Internal->CameraNodeSelector, SIGNAL(nodeAdded(vtkMRMLNode*)),
          this, SLOT(onCameraNodeAdded(vtkMRMLNode*)));
  connect(this->Internal->CameraNodeSelector, SIGNAL(nodeAboutToBeRemoved(vtkMRMLNode*)),
          this, SLOT(onCameraNodeRemoved(vtkMRMLNode*)));
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}

//-----------------------------------------------------------------------------
QString qSlicerCamerasModule::helpText()
{
  // TODO Format text properly .. see transform module for example
  //return "**Cameras Module:** Create new views and cameras. The view pulldown menu below can be used to create new views and select the active view. Switch the layout to \"Tabbed 3D Layout\" from the layout icon in the toolbar to access multiple views. The view selected in \"Tabbed 3D Layout\" becomes the active view and replaces the 3D view in all other layouts. The camera pulldown menu below can be used to set the active camera for the selected view. WARNING: this is rather experimental at the moment (fiducials, IO/data, closing the scene are probably broken for new views). ";
  QString help =
    "To be updated %1";

  return help.arg(this->slicerWikiUrl());
}

//-----------------------------------------------------------------------------
QString qSlicerCamerasModule::acknowledgementText()
{
  QString about =
    "To be updated %1";

  return about.arg(this->slicerWikiUrl());
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModule::onCurrentViewNodeChanged(vtkMRMLNode* mrmlNode)
{
  vtkMRMLViewNode* currentViewNode = vtkMRMLViewNode::SafeDownCast(mrmlNode);
  this->synchronizeCameraWithView(currentViewNode);
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModule::synchronizeCameraWithView(void *)
{
  vtkMRMLViewNode* currentViewNode = vtkMRMLViewNode::SafeDownCast(
    this->Internal->ViewNodeSelector->currentNode());
  this->synchronizeCameraWithView(currentViewNode);
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModule::synchronizeCameraWithView(vtkMRMLViewNode* currentViewNode)
{
  if (!currentViewNode)
    {
    return;
    }
  vtkMRMLCameraNode *found_camera_node = NULL;
  std::vector<vtkMRMLNode*> cameraNodes;
  int nnodes = this->mrmlScene()->GetNodesByClass("vtkMRMLCameraNode", cameraNodes);
  for (int n = 0; n < nnodes; n++)
    {
    vtkMRMLCameraNode *cameraNode = vtkMRMLCameraNode::SafeDownCast(cameraNodes[n]);
    if (cameraNode &&
        cameraNode->GetActiveTag() &&
        !strcmp(cameraNode->GetActiveTag(), currentViewNode->GetID()))
      {
      found_camera_node = cameraNode;
      break;
      }
    }
  this->Internal->CameraNodeSelector->setCurrentNode(found_camera_node);
}


//-----------------------------------------------------------------------------
void qSlicerCamerasModule::onCurrentCameraNodeChanged(vtkMRMLNode* mrmlNode)
{
  vtkMRMLCameraNode *currentCameraNode =
        vtkMRMLCameraNode::SafeDownCast(mrmlNode);
  if (!currentCameraNode)
    {// if the camera list is empty, there is no current camera
    return;
    }
  vtkMRMLViewNode *currentViewNode = vtkMRMLViewNode::SafeDownCast(
    this->Internal->ViewNodeSelector->currentNode());
  if (currentViewNode)
    {
    currentCameraNode->SetActiveTag(currentViewNode->GetID());
    }
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModule::onCameraNodeAdded(vtkMRMLNode* mrmlNode)
{
  vtkMRMLCameraNode *cameraNode =
    vtkMRMLCameraNode::SafeDownCast(mrmlNode);
  if (!cameraNode)
    {
    Q_ASSERT(cameraNode);
    return;
    }
  this->qvtkConnect(cameraNode, vtkMRMLCameraNode::ActiveTagModifiedEvent,
                    this, SLOT(synchronizeCameraWithView(void*)));
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModule::onCameraNodeRemoved(vtkMRMLNode* mrmlNode)
{
  vtkMRMLCameraNode *cameraNode =
    vtkMRMLCameraNode::SafeDownCast(mrmlNode);
  if (!cameraNode)
    {
    Q_ASSERT(cameraNode);
    return;
    }
  this->qvtkDisconnect(cameraNode,
    vtkMRMLCameraNode::ActiveTagModifiedEvent,
    this, SLOT(synchronizeCameraWithView(void*)));
}

//-----------------------------------------------------------------------------
void  qSlicerCamerasModule::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
  // When the view and camera selectors populate their items, the view might populate
  // its items before the camera, and the synchronizeCameraWithView() might do nothing.
  // Let's resync here.
  this->synchronizeCameraWithView();
}
