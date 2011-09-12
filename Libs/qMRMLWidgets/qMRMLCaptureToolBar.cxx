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
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QToolButton>
#include <QWidgetAction>
#include <QInputDialog>

// CTK includes
#include <ctkLogger.h>
#include <ctkSignalMapper.h>

// qMRML includes
#include "qMRMLCaptureToolBar.h"
#include "ui_qMRMLCaptureToolBar.h"
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
  : public Ui_qMRMLCaptureToolBar
{
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLCaptureToolBar);
protected:
  qMRMLCaptureToolBar* const q_ptr;
public:
  qMRMLCaptureToolBarPrivate(qMRMLCaptureToolBar& object);

  virtual void setupUi(QWidget* widget);
/*
  QToolButton* ScreenshotButton;
  QToolButton* SceneViewButton;
*/
  qMRMLSceneViewMenu*              SceneViewMenu;

  // TODO In LayoutManager, use GetActive/IsActive flag ...
  vtkWeakPointer<vtkMRMLViewNode>  ActiveMRMLThreeDViewNode;

  void setMRMLScene(vtkMRMLScene* newScene);
public slots:
  void onMRMLSceneAboutToBeClosedEvent();
  void onMRMLSceneImportedEvent();
  void onMRMLSceneClosedEvent();
  void updateWidgetFromMRML();
  void createSceneView();

public:

  vtkSmartPointer<vtkMRMLScene>            MRMLScene;
  
};
//--------------------------------------------------------------------------
// qMRMLCaptureToolBarPrivate methods

//---------------------------------------------------------------------------
qMRMLCaptureToolBarPrivate::qMRMLCaptureToolBarPrivate(qMRMLCaptureToolBar& object)
  : q_ptr(&object)
{
  this->ActiveMRMLThreeDViewNode = 0;
  
  this->SceneViewMenu = 0;
  /*
  this->SceneViewButton = 0;
  this->SelectSceneViewMenuButton = 0;
  this->ScreenshotButton = 0;
  */
}

// --------------------------------------------------------------------------
void qMRMLCaptureToolBarPrivate::updateWidgetFromMRML()
{
  if (!this->MRMLScene)
    {
    return;
    }
  // Enable buttons
  this->ScreenshotButton->setEnabled(this->ActiveMRMLThreeDViewNode != 0);
}

//---------------------------------------------------------------------------
void qMRMLCaptureToolBarPrivate::setupUi(QWidget* widget)
{
  // Q_Q(qMRMLCaptureToolBar);
  this->Ui_qMRMLCaptureToolBar::setupUi(widget);

  // start no ui
  /*
  // Screenshot button
  this->ScreenshotButton = new QToolButton();
  this->ScreenshotButton->setCheckable(true);
  this->ScreenshotButton->setToolTip(QObject::tr("Capture a screenshot of the full layout, 3D view or slice views."));
  this->ScreenshotButton->setIcon(QIcon(":/Icons/ViewCapture.png"));
  this->ScreenshotButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  this->ScreenshotButton->setText(QObject::tr("Screenshot"));
  
  // Scene View buttons
  this->SceneViewButton = new QToolButton();
  this->SceneViewButton->setToolTip(QObject::tr("Capture and name a scene view."));

  this->SelectSceneViewMenuButton = new QToolButton();
  this->SelectSceneViewMenuButton->setToolTip(QObject::tr("Restore or delete saved scene views."));
  this->SelectSceneViewMenuButton->setIcon(QIcon(":/Icons/ViewCameraSelect.png"));
  this->SelectSceneViewMenuButton->setPopupMode(QToolButton::InstantPopup);
  // end no ui
  */
  this->SceneViewMenu = new qMRMLSceneViewMenu(widget);
  this->SelectSceneViewMenuButton->setMenu(this->SceneViewMenu);
  
  
  QObject::connect(widget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
          this->SceneViewMenu, SLOT(setMRMLScene(vtkMRMLScene*)));
  

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
  q->setEnabled(this->MRMLScene != 0);
  if (this->MRMLScene)
    {
    this->updateWidgetFromMRML();
    }
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

  // re-enable in case it didn't get re-enabled for scene load
  q->setEnabled(true);
  
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
  // reenable it and update
  q->setEnabled(true);
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
  d->setupUi(this);
}

// --------------------------------------------------------------------------
qMRMLCaptureToolBar::qMRMLCaptureToolBar(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLCaptureToolBarPrivate(*this))
{
  Q_D(qMRMLCaptureToolBar);
  d->setupUi(this);

  QObject::connect(d->ScreenshotButton, SIGNAL(clicked()), SIGNAL(screenshotButtonClicked()));
  QObject::connect(d->SceneViewButton, SIGNAL(clicked()), SIGNAL(sceneViewButtonClicked()));
}

//---------------------------------------------------------------------------
qMRMLCaptureToolBar::~qMRMLCaptureToolBar()
{

}


// --------------------------------------------------------------------------
void qMRMLCaptureToolBar::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLCaptureToolBar);

//  this->Superclass::setMRMLScene(scene);
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


