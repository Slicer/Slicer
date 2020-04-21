/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Slicer includes
#include "qSlicerCamerasModuleWidget.h"
#include "ui_qSlicerCamerasModuleWidget.h"
#include "vtkSlicerCamerasModuleLogic.h"

// MRML includes
#include "vtkMRMLViewNode.h"
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLScene.h"

// STD includes

//-----------------------------------------------------------------------------
class qSlicerCamerasModuleWidgetPrivate: public Ui_qSlicerCamerasModuleWidget
{
public:
};

//-----------------------------------------------------------------------------
qSlicerCamerasModuleWidget::qSlicerCamerasModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerCamerasModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerCamerasModuleWidget::~qSlicerCamerasModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleWidget::setup()
{
  Q_D(qSlicerCamerasModuleWidget);
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
  Q_D(qSlicerCamerasModuleWidget);
  vtkMRMLViewNode* currentViewNode = vtkMRMLViewNode::SafeDownCast(
    d->ViewNodeSelector->currentNode());
  this->synchronizeCameraWithView(currentViewNode);
}

//-----------------------------------------------------------------------------
void qSlicerCamerasModuleWidget::synchronizeCameraWithView(vtkMRMLViewNode* currentViewNode)
{
  Q_D(qSlicerCamerasModuleWidget);
  if (!currentViewNode)
    {
    return;
    }
  vtkSlicerCamerasModuleLogic* camerasLogic =
    vtkSlicerCamerasModuleLogic::SafeDownCast(this->logic());
  vtkMRMLCameraNode *found_camera_node =
    camerasLogic->GetViewActiveCameraNode(currentViewNode);
  d->CameraNodeSelector->setCurrentNode(found_camera_node);
}


//-----------------------------------------------------------------------------
void qSlicerCamerasModuleWidget::setCameraToCurrentView(vtkMRMLNode* mrmlNode)
{
  Q_D(qSlicerCamerasModuleWidget);
  vtkMRMLCameraNode *currentCameraNode =
        vtkMRMLCameraNode::SafeDownCast(mrmlNode);
  if (!currentCameraNode)
    {// if the camera list is empty, there is no current camera
    return;
    }
  vtkMRMLViewNode *currentViewNode = vtkMRMLViewNode::SafeDownCast(
    d->ViewNodeSelector->currentNode());
  if (currentViewNode == nullptr)
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

//-----------------------------------------------------------
bool qSlicerCamerasModuleWidget::setEditedNode(vtkMRMLNode* node,
                                               QString role /* = QString()*/,
                                               QString context /* = QString()*/)
{
  Q_D(qSlicerCamerasModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);

  if (vtkMRMLViewNode::SafeDownCast(node))
    {
    d->ViewNodeSelector->setCurrentNode(node);
    return true;
    }

  if (vtkMRMLCameraNode::SafeDownCast(node))
    {
    vtkMRMLCameraNode* cameraNode = vtkMRMLCameraNode::SafeDownCast(node);
    vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(this->mrmlScene()->GetNodeByID(cameraNode->GetActiveTag()));
    if (!viewNode)
      {
      return false;
      }
    d->ViewNodeSelector->setCurrentNode(viewNode);
    return true;
    }

  return false;
}
