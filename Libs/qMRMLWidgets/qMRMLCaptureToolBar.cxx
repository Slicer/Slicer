/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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

// Qt includes
#include <QMenu>
#include <QInputDialog>
#include <QToolButton>

// CTK includes
#include <ctkLogger.h>

// qMRML includes
#include "qMRMLCaptureToolBar.h"
#include "qMRMLSceneViewMenu.h"
#include "qMRMLNodeFactory.h"

// MRML includes
#include <vtkMRMLViewNode.h>
#include <vtkMRMLSceneViewNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLCaptureToolBar");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class qMRMLCaptureToolBarPrivate
{
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLCaptureToolBar);
protected:
  qMRMLCaptureToolBar* const q_ptr;
public:
  qMRMLCaptureToolBarPrivate(qMRMLCaptureToolBar& object);
  void init();
  void setMRMLScene(vtkMRMLScene* newScene);
  QAction*                         ScreenshotAction;
  QAction*                         SceneViewAction;
  qMRMLSceneViewMenu*              SceneViewMenu;

  // TODO In LayoutManager, use GetActive/IsActive flag ...
  vtkWeakPointer<vtkMRMLViewNode>  ActiveMRMLThreeDViewNode;
  vtkSmartPointer<vtkMRMLScene>    MRMLScene;

public slots:
  void onMRMLSceneAboutToBeClosedEvent();
  void onMRMLSceneImportedEvent();
  void onMRMLSceneClosedEvent();
  void updateWidgetFromMRML();
  void createSceneView();
};

//--------------------------------------------------------------------------
// qMRMLCaptureToolBarPrivate methods

//---------------------------------------------------------------------------
qMRMLCaptureToolBarPrivate::qMRMLCaptureToolBarPrivate(qMRMLCaptureToolBar& object)
  : q_ptr(&object)
{
  this->ScreenshotAction = 0;
  this->SceneViewAction = 0;
  this->SceneViewMenu = 0;
}

// --------------------------------------------------------------------------
void qMRMLCaptureToolBarPrivate::updateWidgetFromMRML()
{
  Q_Q(qMRMLCaptureToolBar);
  // Enable buttons
  q->setEnabled(this->MRMLScene != 0);
  this->ScreenshotAction->setEnabled(this->ActiveMRMLThreeDViewNode != 0);
}

//---------------------------------------------------------------------------
void qMRMLCaptureToolBarPrivate::init()
{
  Q_Q(qMRMLCaptureToolBar);

  // Screenshot button
  this->ScreenshotAction = new QAction(q);
  this->ScreenshotAction->setIcon(QIcon(":/Icons/ViewCapture.png"));
  this->ScreenshotAction->setText(q->tr("Screenshot"));
  this->ScreenshotAction->setToolTip(q->tr(
    "Capture a screenshot of the full layout, 3D view or slice views."));
  QObject::connect(this->ScreenshotAction, SIGNAL(triggered()),
                   q, SIGNAL(screenshotButtonClicked()));
  q->addAction(this->ScreenshotAction);

  // Scene View buttons
  this->SceneViewAction = new QAction(q);
  this->SceneViewAction->setIcon(QIcon(":/Icons/ViewCamera.png"));
  this->SceneViewAction->setText(q->tr("Scene view"));
  this->SceneViewAction->setToolTip(q->tr("Capture and name a scene view."));
  QObject::connect(this->SceneViewAction, SIGNAL(triggered()),
                   q, SIGNAL(sceneViewButtonClicked()));
  q->addAction(this->SceneViewAction);

  // Scene view menu
  QToolButton* sceneViewMenuButton = new QToolButton(q);
  sceneViewMenuButton->setText(q->tr("Restore view"));
  sceneViewMenuButton->setIcon(QIcon(":/Icons/ViewCameraSelect.png"));
  sceneViewMenuButton->setToolTip(QObject::tr("Restore or delete saved scene views."));
  this->SceneViewMenu = new qMRMLSceneViewMenu(sceneViewMenuButton);
  sceneViewMenuButton->setMenu(this->SceneViewMenu);
  sceneViewMenuButton->setPopupMode(QToolButton::InstantPopup);
  //QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
  //                 this->SceneViewMenu, SLOT(setMRMLScene(vtkMRMLScene*)));
  q->addWidget(sceneViewMenuButton);
}
// --------------------------------------------------------------------------
void qMRMLCaptureToolBarPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_Q(qMRMLCaptureToolBar);

  if (newScene == this->MRMLScene)
    {
    return;
    }
/*
  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::SceneAboutToBeClosedEvent,
                      this, SLOT(onMRMLSceneAboutToBeClosedEvent()));

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::SceneImportedEvent,
                      this, SLOT(onMRMLSceneImportedEvent()));

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::SceneClosedEvent,
                      this, SLOT(onMRMLSceneClosedEvent()));
*/
  this->MRMLScene = newScene;

  this->SceneViewMenu->setMRMLScene(newScene);

  // Update UI
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qMRMLCaptureToolBarPrivate::onMRMLSceneAboutToBeClosedEvent()
{
  Q_Q(qMRMLCaptureToolBar);
  q->setEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLCaptureToolBarPrivate::onMRMLSceneImportedEvent()
{
  Q_Q(qMRMLCaptureToolBar);
  // update the state from mrml
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qMRMLCaptureToolBarPrivate::onMRMLSceneClosedEvent()
{
  Q_Q(qMRMLCaptureToolBar);
  Q_ASSERT(this->MRMLScene);
  if (!this->MRMLScene || this->MRMLScene->GetIsUpdating())
    {
    return;
    }
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qMRMLCaptureToolBarPrivate::createSceneView()
{
  Q_Q(qMRMLCaptureToolBar);

  // Ask user for a name
  bool ok = false;
  QString sceneViewName = QInputDialog::getText(q, QObject::tr("SceneView Name"),
                                                QObject::tr("SceneView Name:"), QLineEdit::Normal,
                                                "View", &ok);
  if (!ok || sceneViewName.isEmpty())
    {
    return;
    }

  // Create scene view
  qMRMLNodeFactory nodeFactory;
  nodeFactory.setMRMLScene(this->MRMLScene);
  nodeFactory.setBaseName("vtkMRMLSceneViewNode", sceneViewName);
  vtkMRMLNode * newNode = nodeFactory.createNode("vtkMRMLSceneViewNode");
  vtkMRMLSceneViewNode * newSceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(newNode);
  newSceneViewNode->StoreScene();
}

// --------------------------------------------------------------------------
// qMRMLCaptureToolBar methods

// --------------------------------------------------------------------------
qMRMLCaptureToolBar::qMRMLCaptureToolBar(const QString& title, QWidget* parentWidget)
  :Superclass(title, parentWidget)
   , d_ptr(new qMRMLCaptureToolBarPrivate(*this))
{
  Q_D(qMRMLCaptureToolBar);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLCaptureToolBar::qMRMLCaptureToolBar(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qMRMLCaptureToolBarPrivate(*this))
{
  Q_D(qMRMLCaptureToolBar);
  d->init();
}

//---------------------------------------------------------------------------
qMRMLCaptureToolBar::~qMRMLCaptureToolBar()
{
}

// --------------------------------------------------------------------------
void qMRMLCaptureToolBar::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLCaptureToolBar);
  d->setMRMLScene(scene);
}

// --------------------------------------------------------------------------
void qMRMLCaptureToolBar::setActiveMRMLThreeDViewNode(
  vtkMRMLViewNode * newActiveMRMLThreeDViewNode)
{
  Q_D(qMRMLCaptureToolBar);
  d->ActiveMRMLThreeDViewNode = newActiveMRMLThreeDViewNode;
  d->updateWidgetFromMRML();
}
