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

// Qt includes
#include <QDebug>
#include <QMenu>
#include <QInputDialog>
#include <QTimer>
#include <QToolButton>

// CTK includes
#include <ctkMessageBox.h>

// qMRML includes
#include "qMRMLCaptureToolBar.h"
#include <qMRMLSceneViewMenu.h>
#include <qMRMLNodeFactory.h>
#include <qSlicerApplication.h>

#include <vtkSlicerApplicationLogic.h>

// Scene view logic
#include <vtkSlicerSceneViewsModuleLogic.h>

// MRML includes
#include <vtkMRMLViewNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

//-----------------------------------------------------------------------------
class qMRMLCaptureToolBarPrivate
{
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLCaptureToolBar);

protected:
  qMRMLCaptureToolBar* const q_ptr;
  bool timeOutFlag;

public:
  qMRMLCaptureToolBarPrivate(qMRMLCaptureToolBar& object);
  void init();
  void setMRMLScene(vtkMRMLScene* newScene);
  QAction* ScreenshotAction;
  QAction* SceneViewAction;
  qMRMLSceneViewMenu* SceneViewMenu;

  // TODO In LayoutManager, use GetActive/IsActive flag ...
  vtkWeakPointer<vtkMRMLViewNode> ActiveMRMLThreeDViewNode;
  vtkSmartPointer<vtkMRMLScene> MRMLScene;

public slots:
  void updateWidgetFromMRML();
  void createSceneView();
};

//--------------------------------------------------------------------------
// qMRMLCaptureToolBarPrivate methods

//---------------------------------------------------------------------------
qMRMLCaptureToolBarPrivate::qMRMLCaptureToolBarPrivate(qMRMLCaptureToolBar& object)
  : q_ptr(&object)
{
  this->ScreenshotAction = nullptr;
  this->SceneViewAction = nullptr;
  this->SceneViewMenu = nullptr;
  this->timeOutFlag = false;
}

// --------------------------------------------------------------------------
void qMRMLCaptureToolBarPrivate::updateWidgetFromMRML()
{
  Q_Q(qMRMLCaptureToolBar);
  // Enable buttons
  q->setEnabled(this->MRMLScene != nullptr);
  this->ScreenshotAction->setEnabled(this->ActiveMRMLThreeDViewNode != nullptr);
}

//---------------------------------------------------------------------------
void qMRMLCaptureToolBarPrivate::init()
{
  Q_Q(qMRMLCaptureToolBar);

  // Screenshot button
  this->ScreenshotAction = new QAction(q);
  this->ScreenshotAction->setIcon(QIcon(":/Icons/ViewCapture.png"));
  this->ScreenshotAction->setText(qMRMLCaptureToolBar::tr("Screenshot"));
  this->ScreenshotAction->setToolTip(qMRMLCaptureToolBar::tr("Capture a screenshot of the full layout, 3D view or slice views. Use File, Save to save the image."));
  QObject::connect(this->ScreenshotAction, SIGNAL(triggered()), q, SIGNAL(screenshotButtonClicked()));
  q->addAction(this->ScreenshotAction);

  // Scene View buttons, signal will be observed by qSlicerMainWindow and will cause
  // qSlicerSceneViewsModuleWidget to open a qSlicerSceneViewsModuleDialog.
  this->SceneViewAction = new QAction(q);
  this->SceneViewAction->setIcon(QIcon(":/Icons/ViewCamera.png"));
  this->SceneViewAction->setText(qMRMLCaptureToolBar::tr("Scene view"));
  this->SceneViewAction->setToolTip(qMRMLCaptureToolBar::tr("Capture and name a scene view."));
  QObject::connect(this->SceneViewAction, SIGNAL(triggered()), q, SIGNAL(sceneViewButtonClicked()));
  q->addAction(this->SceneViewAction);

  // Scene view menu
  QToolButton* sceneViewMenuButton = new QToolButton(q);
  sceneViewMenuButton->setText(qMRMLCaptureToolBar::tr("Restore view"));
  sceneViewMenuButton->setIcon(QIcon(":/Icons/ViewCameraSelect.png"));
  sceneViewMenuButton->setToolTip(qMRMLCaptureToolBar::tr("Restore or delete saved scene views."));
  this->SceneViewMenu = new qMRMLSceneViewMenu(sceneViewMenuButton);
  sceneViewMenuButton->setMenu(this->SceneViewMenu);
  sceneViewMenuButton->setPopupMode(QToolButton::InstantPopup);
  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), this->SceneViewMenu, SLOT(setMRMLScene(vtkMRMLScene*)));
  q->addWidget(sceneViewMenuButton);
  QObject::connect(q, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)), sceneViewMenuButton, SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));
}
// --------------------------------------------------------------------------
void qMRMLCaptureToolBarPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_Q(qMRMLCaptureToolBar);

  if (newScene == this->MRMLScene)
  {
    return;
  }

  qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::StartBatchProcessEvent, q, SLOT(OnMRMLSceneStartBatchProcessing()));
  qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::EndBatchProcessEvent, q, SLOT(OnMRMLSceneEndBatchProcessing()));

  this->MRMLScene = newScene;

  this->SceneViewMenu->setMRMLScene(newScene);

  // Update UI
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qMRMLCaptureToolBar::OnMRMLSceneStartBatchProcessing()
{
  Q_D(qMRMLCaptureToolBar);
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLCaptureToolBar::OnMRMLSceneEndBatchProcessing()
{
  Q_D(qMRMLCaptureToolBar);
  d->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qMRMLCaptureToolBarPrivate::createSceneView()
{
  Q_Q(qMRMLCaptureToolBar);

  // Ask user for a name
  bool ok = false;
  QString sceneViewName = QInputDialog::getText(q, qMRMLCaptureToolBar::tr("SceneView Name"), qMRMLCaptureToolBar::tr("SceneView Name:"), QLineEdit::Normal, "View", &ok);
  if (!ok || sceneViewName.isEmpty())
  {
    return;
  }

  vtkSlicerSceneViewsModuleLogic* sceneViewsLogic =
    vtkSlicerSceneViewsModuleLogic::SafeDownCast(qSlicerApplication::application()->applicationLogic()->GetModuleLogic("SliceViews"));
  sceneViewsLogic->CreateSceneView(sceneViewName.toStdString());
}

// --------------------------------------------------------------------------
// qMRMLCaptureToolBar methods

// --------------------------------------------------------------------------
qMRMLCaptureToolBar::qMRMLCaptureToolBar(const QString& title, QWidget* parentWidget)
  : Superclass(title, parentWidget)
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
qMRMLCaptureToolBar::~qMRMLCaptureToolBar() = default;

// --------------------------------------------------------------------------
void qMRMLCaptureToolBar::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLCaptureToolBar);
  d->setMRMLScene(scene);
}

// --------------------------------------------------------------------------
void qMRMLCaptureToolBar::setActiveMRMLThreeDViewNode(vtkMRMLViewNode* newActiveMRMLThreeDViewNode)
{
  Q_D(qMRMLCaptureToolBar);
  d->ActiveMRMLThreeDViewNode = newActiveMRMLThreeDViewNode;
  d->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
bool qMRMLCaptureToolBar::popupsTimeOut() const
{
  Q_D(const qMRMLCaptureToolBar);

  return d->timeOutFlag;
}

// --------------------------------------------------------------------------
void qMRMLCaptureToolBar::setPopupsTimeOut(bool flag)
{
  Q_D(qMRMLCaptureToolBar);

  d->timeOutFlag = flag;
  emit popupsTimeOutChanged();
}
