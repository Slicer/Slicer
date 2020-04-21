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

// qMRML includes
#include "qMRMLColors.h"
#include "qMRMLNavigationView.h"

// MRML includes
#include <vtkMRMLViewNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkWeakPointer.h>

//-----------------------------------------------------------------------------
class qMRMLNavigationViewPrivate
{
  Q_DECLARE_PUBLIC(qMRMLNavigationView);
protected:
  qMRMLNavigationView* const q_ptr;
public:
  qMRMLNavigationViewPrivate(qMRMLNavigationView& object);
  ~qMRMLNavigationViewPrivate();

  vtkMRMLScene*                      MRMLScene;
  vtkWeakPointer<vtkMRMLViewNode>    MRMLViewNode;
};

//--------------------------------------------------------------------------
// qMRMLNavigationViewPrivate methods

//---------------------------------------------------------------------------
qMRMLNavigationViewPrivate::qMRMLNavigationViewPrivate(qMRMLNavigationView& object)
  : q_ptr(&object)
{
  this->MRMLScene = nullptr;
}

//---------------------------------------------------------------------------
qMRMLNavigationViewPrivate::~qMRMLNavigationViewPrivate() = default;

// --------------------------------------------------------------------------
// qMRMLNavigationView methods

// --------------------------------------------------------------------------
qMRMLNavigationView::qMRMLNavigationView(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLNavigationViewPrivate(*this))
{
  // Set default background color
  this->setBackgroundColor(QColor::fromRgbF(
    vtkMRMLViewNode::defaultBackgroundColor()[0],
    vtkMRMLViewNode::defaultBackgroundColor()[1],
    vtkMRMLViewNode::defaultBackgroundColor()[2]));
}

// --------------------------------------------------------------------------
qMRMLNavigationView::~qMRMLNavigationView() = default;

//------------------------------------------------------------------------------
void qMRMLNavigationView::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLNavigationView);
  if (d->MRMLScene == newScene)
    {
    return;
    }
  this->qvtkReconnect(d->MRMLScene, newScene, vtkMRMLScene::NodeAddedEvent,
                      this, SLOT(updateFromMRMLScene()));
  this->qvtkReconnect(d->MRMLScene, newScene, vtkMRMLScene::NodeRemovedEvent,
                      this, SLOT(updateFromMRMLScene()));

  this->qvtkReconnect(d->MRMLScene, newScene, vtkMRMLScene::EndBatchProcessEvent,
                      this, SLOT(updateFromMRMLScene()));
  d->MRMLScene = newScene;
  if (!d->MRMLViewNode || newScene != d->MRMLViewNode->GetScene())
    {
    this->setMRMLViewNode(nullptr);
    }
  this->updateFromMRMLScene();
}

//---------------------------------------------------------------------------
void qMRMLNavigationView::setMRMLViewNode(vtkMRMLViewNode* newViewNode)
{
  Q_D(qMRMLNavigationView);
  if (d->MRMLViewNode == newViewNode)
    {
    return;
    }

  this->qvtkReconnect(d->MRMLViewNode, newViewNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateFromMRMLViewNode()));
  d->MRMLViewNode = newViewNode;
  this->updateFromMRMLViewNode();

  // Enable/disable widget
  this->setEnabled(d->MRMLViewNode != nullptr);
}

//---------------------------------------------------------------------------
vtkMRMLViewNode* qMRMLNavigationView::mrmlViewNode()const
{
  Q_D(const qMRMLNavigationView);
  return d->MRMLViewNode;
}

//---------------------------------------------------------------------------
void qMRMLNavigationView::updateFromMRMLScene()
{
  Q_D(qMRMLNavigationView);
  if (!d->MRMLScene || d->MRMLScene->IsBatchProcessing())
    {
    return;
    }
  this->updateBounds();
}

//---------------------------------------------------------------------------
void qMRMLNavigationView::updateFromMRMLViewNode()
{
  Q_D(qMRMLNavigationView);
  if (!d->MRMLViewNode)
    {
    return;
    }
  double backgroundColor[3];
  d->MRMLViewNode->GetBackgroundColor(backgroundColor);
  this->setBackgroundColor(
    QColor::fromRgbF(backgroundColor[0], backgroundColor[1], backgroundColor[2]));
}
