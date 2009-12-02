#include "qSlicerCamerasModule.h"
#include "ui_qSlicerCamerasModule.h"

#include "vtkMRMLViewNode.h"
#include "vtkMRMLCameraNode.h"

#include <vector>

//-----------------------------------------------------------------------------
struct qSlicerCamerasModulePrivate: public qCTKPrivate<qSlicerCamerasModule>,
                                    public Ui_qSlicerCamerasModule
{
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerCamerasModule, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerCamerasModule::setup()
{
  this->Superclass::setup();
  
  QCTK_D(qSlicerCamerasModule);
  d->setupUi(this);

  connect(d->ViewNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onCurrentViewNodeChanged(vtkMRMLNode*)));
  connect(d->CameraNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onCurrentCameraNodeChanged(vtkMRMLNode*)));
  connect(d->CameraNodeSelector, SIGNAL(nodeAdded(vtkMRMLNode*)),
          this, SLOT(onCameraNodeAdded(vtkMRMLNode*)));
  connect(d->CameraNodeSelector, SIGNAL(nodeAboutToBeRemoved(vtkMRMLNode*)),
          this, SLOT(onCameraNodeRemoved(vtkMRMLNode*)));
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}

//-----------------------------------------------------------------------------
QString qSlicerCamerasModule::helpText()const
{
  QString help =
    "Create new views and cameras.<br>"
    "The view pulldown menu below can be used to create new views and select "
    "the active view. Switch the layout to \"Tabbed 3D Layout\" from the "
    "layout icon in the toolbar to access multiple views. The view selected in "
    "\"Tabbed 3D Layout\" becomes the active view and replaces the 3D view in "
    "all other layouts. The camera pulldown menu below can be used to set the "
    "active camera for the selected view. <br>"
    "WARNING: this is rather experimental at the moment (fiducials, IO/data, "
    "closing the scene are probably broken for new views).";
  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerCamerasModule::acknowledgementText()const
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
    qctk_d()->ViewNodeSelector->currentNode());
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
  qctk_d()->CameraNodeSelector->setCurrentNode(found_camera_node);
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
    qctk_d()->ViewNodeSelector->currentNode());
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
