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
#include "qMRMLSceneViewMenu.h"
#include "qMRMLNodeFactory.h"

// MRML includes
#include <vtkMRMLViewNode.h>
#include <vtkMRMLSceneViewNode.h>
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
  void OnMRMLSceneStartBatchProcessing();
  void OnMRMLSceneEndBatchProcessing();
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
    "Capture a screenshot of the full layout, 3D view or slice views. Use File, Save to save the image. Edit in the Annotations module."));
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
  QObject::connect(q, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                  sceneViewMenuButton,
                  SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));
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
  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::StartBatchProcessEvent,
                      this, SLOT(OnMRMLSceneStartBatchProcessing()));

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::EndBatchProcessEvent,
                      this, SLOT(OnMRMLSceneEndBatchProcessing()));

*/
  // observe for storable nodes added after scene views are present in the scene
  this->qvtkReconnect(this->MRMLScene, newScene,
                      vtkMRMLScene::NodeAddedEvent,
                      q, SLOT(OnMRMLSceneNodeAddedEvent(vtkObject*,vtkObject*)));

  this->MRMLScene = newScene;

  this->SceneViewMenu->setMRMLScene(newScene);

  // Update UI
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qMRMLCaptureToolBarPrivate::OnMRMLSceneStartBatchProcessing()
{
  Q_Q(qMRMLCaptureToolBar);
  q->setEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLCaptureToolBarPrivate::OnMRMLSceneEndBatchProcessing()
{
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

// --------------------------------------------------------------------------
void qMRMLCaptureToolBar::OnMRMLSceneNodeAddedEvent(vtkObject *vtkNotUsed(caller), vtkObject* callData)
{
  Q_D(qMRMLCaptureToolBar);

  if (!d->MRMLScene ||
      d->MRMLScene->IsClosing() ||
      d->MRMLScene->IsBatchProcessing() ||
      !callData)
    {
    return;
    }

  vtkMRMLNode *mrmlNode = vtkMRMLNode::SafeDownCast(callData);
  if (!mrmlNode)
    {
    return;
    }

  if (!mrmlNode->IsA("vtkMRMLSceneViewNode") &&
      mrmlNode->IsA("vtkMRMLStorableNode") &&
      d->MRMLScene->GetNumberOfNodesByClass("vtkMRMLSceneViewNode") > 0)
    {
    qDebug() << "Warning! Adding the node" << mrmlNode->GetName() << "to the scene does not add it\nto the scene views already present!\n\tRestoring a previously saved scene view will remove this node!";
    ctkMessageBox msgBox;
    msgBox.setWindowTitle("New data added after scene view created.");
    QString msgText = QString("Data that can be saved to disk was added to the scene, but doesn't appear in the currently defined scene views!\n")
      + QString("If you restore one of those scene views you will lose\n")
      + QString(mrmlNode->GetName())
      + QString("\nYou can save ")
      + QString(mrmlNode->GetName())
      + QString(" to disk, restore the scene view, reload the data, then resave the scene view.");
    msgBox.setText(msgText);
    QPushButton *ackButton = msgBox.addButton("Acknowledged",
                                              QMessageBox::AcceptRole);
    msgBox.setDefaultButton(ackButton);
    msgBox.setDontShowAgainVisible(true);
    msgBox.setDontShowAgainSettingsKey("SceneViews/NeverShowLostDataWarning");
    // have it time out after giving the user some time to read it
    QTimer::singleShot(15000, &msgBox, SLOT(close()));
    msgBox.exec();
    }
}
