/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// SlicerQt includes
#include "qSlicerCamerasModuleWidget.h"
#include "ui_qSlicerCamerasModule.h"

// MRML includes
#include "vtkMRMLViewNode.h"
#include "vtkMRMLCameraNode.h"

// STD includes
#include <vector>

//-----------------------------------------------------------------------------
class qSlicerCamerasModuleWidgetPrivate: public ctkPrivate<qSlicerCamerasModuleWidget>,
                                         public Ui_qSlicerCamerasModule
{
public:
  CTK_DECLARE_PUBLIC(qSlicerCamerasModuleWidget);
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerCamerasModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleWidget::setup()
{
  CTK_D(qSlicerCamerasModuleWidget);
  d->setupUi(this);

  connect(d->ViewNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onCurrentViewNodeChanged(vtkMRMLNode*)));
  connect(d->CameraNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(setCameraToCurrentView(vtkMRMLNode*)));
  connect(d->CameraNodeSelector, SIGNAL(nodeAdded(vtkMRMLNode*)),
          this, SLOT(onCameraNodeAdded(vtkMRMLNode*)));
  connect(d->CameraNodeSelector, SIGNAL(nodeAboutToBeRemoved(vtkMRMLNode*)),
          this, SLOT(onCameraNodeRemoved(vtkMRMLNode*)));
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleWidget::onCurrentViewNodeChanged(vtkMRMLNode* mrmlNode)
{
  vtkMRMLViewNode* currentViewNode = vtkMRMLViewNode::SafeDownCast(mrmlNode);
  this->synchronizeCameraWithView(currentViewNode);
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleWidget::synchronizeCameraWithView()
{
  vtkMRMLViewNode* currentViewNode = vtkMRMLViewNode::SafeDownCast(
    ctk_d()->ViewNodeSelector->currentNode());
  this->synchronizeCameraWithView(currentViewNode);
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleWidget::synchronizeCameraWithView(vtkMRMLViewNode* currentViewNode)
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
  ctk_d()->CameraNodeSelector->setCurrentNode(found_camera_node);
}


//-----------------------------------------------------------------------------
void qSlicerCamerasModuleWidget::setCameraToCurrentView(vtkMRMLNode* mrmlNode)
{
  CTK_D(qSlicerCamerasModuleWidget);
  vtkMRMLCameraNode *currentCameraNode =
        vtkMRMLCameraNode::SafeDownCast(mrmlNode);
  if (!currentCameraNode)
    {// if the camera list is empty, there is no current camera
    return;
    }
  vtkMRMLViewNode *currentViewNode = vtkMRMLViewNode::SafeDownCast(
    d->ViewNodeSelector->currentNode());
  if (currentViewNode == 0)
    {
    return;
    }
  currentCameraNode->SetActiveTag(currentViewNode->GetID());
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleWidget::onCameraNodeAdded(vtkMRMLNode* mrmlNode)
{
  vtkMRMLCameraNode *cameraNode = vtkMRMLCameraNode::SafeDownCast(mrmlNode);
  if (!cameraNode)
    {
    //Q_ASSERT(cameraNode);
    return;
    }
  this->qvtkConnect(cameraNode, vtkMRMLCameraNode::ActiveTagModifiedEvent,
                    this, SLOT(synchronizeCameraWithView()));
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleWidget::onCameraNodeRemoved(vtkMRMLNode* mrmlNode)
{
  vtkMRMLCameraNode *cameraNode = vtkMRMLCameraNode::SafeDownCast(mrmlNode);
  if (!cameraNode)
    {
    //Q_ASSERT(cameraNode);
    return;
    }
  this->qvtkDisconnect(cameraNode, vtkMRMLCameraNode::ActiveTagModifiedEvent,
                       this, SLOT(synchronizeCameraWithView()));
}

//-----------------------------------------------------------------------------
void  qSlicerCamerasModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);

  // When the view and camera selectors populate their items, the view might populate
  // its items before the camera, and the synchronizeCameraWithView() might do nothing.
  // Let's resync here.
  this->synchronizeCameraWithView();
}
