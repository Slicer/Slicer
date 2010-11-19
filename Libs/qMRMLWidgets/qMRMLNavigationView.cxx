/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QFileInfo>

// CTK includes
#include <ctkLogger.h>

// qMRML includes
#include "qMRMLNavigationView.h"

// MRML includes
#include <vtkMRMLViewNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkCamera.h>
#include <vtkFollower.h>
#include <vtkInteractorStyle.h>
#include <vtkMath.h>
#include <vtkOutlineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLNavigationView");
//--------------------------------------------------------------------------

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
  this->MRMLScene = 0;
}

//---------------------------------------------------------------------------
qMRMLNavigationViewPrivate::~qMRMLNavigationViewPrivate()
{
}

// --------------------------------------------------------------------------
// qMRMLNavigationView methods

// --------------------------------------------------------------------------
qMRMLNavigationView::qMRMLNavigationView(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLNavigationViewPrivate(*this))
{
  // Set default background color
  this->setBackgroundColor(QColor::fromRgbF(0.701960784314, 0.701960784314, 0.905882352941));
}

// --------------------------------------------------------------------------
qMRMLNavigationView::~qMRMLNavigationView()
{
}

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

  this->qvtkReconnect(d->MRMLScene, newScene, vtkMRMLScene::SceneClosedEvent,
                      this, SLOT(updateFromMRMLScene()));
  this->qvtkReconnect(d->MRMLScene, newScene, vtkMRMLScene::SceneImportedEvent,
                      this, SLOT(updateFromMRMLScene()));
  d->MRMLScene = newScene;
  if (!d->MRMLViewNode || newScene != d->MRMLViewNode->GetScene())
    {
    this->setMRMLViewNode(0);
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

  this->qvtkReconnect(d->MRMLViewNode, newViewNode, vtkMRMLViewNode::BackgroundColorEvent,
                      this, SLOT(updateFromMRMLViewNode()));
  d->MRMLViewNode = newViewNode;
  this->updateFromMRMLViewNode();

  // Enable/disable widget
  this->setEnabled(d->MRMLViewNode != 0);
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
  if (!d->MRMLScene || d->MRMLScene->GetIsUpdating())
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
