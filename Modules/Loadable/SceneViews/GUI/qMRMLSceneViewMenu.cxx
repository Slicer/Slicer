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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QPushButton>

// CTK includes
#include <ctkMessageBox.h>

// qMRML includes
#include <qMRMLSceneViewMenu_p.h>
#include <qSlicerApplication.h>

// MRML includes
#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLSequenceBrowserNode.h>

//--------------------------------------------------------------------------
// qMRMLSceneViewMenuPrivate methods

//---------------------------------------------------------------------------
qMRMLSceneViewMenuPrivate::qMRMLSceneViewMenuPrivate(qMRMLSceneViewMenu& object)
  : Superclass(&object)
  , q_ptr(&object)
{
  connect(&this->RestoreActionMapper, SIGNAL(mapped(int)), SLOT(restoreSceneView(int)));
  connect(&this->DeleteActionMapper, SIGNAL(mapped(int)), SLOT(deleteSceneView(int)));

  this->SceneViewsLogic = vtkSlicerSceneViewsModuleLogic::SafeDownCast(qSlicerApplication::application()->applicationLogic()->GetModuleLogic("SceneViews"));
  qvtkConnect(this->SceneViewsLogic, vtkSlicerSceneViewsModuleLogic::SceneViewsModifiedEvent, this, SLOT(resetMenu()));
}

// --------------------------------------------------------------------------
void qMRMLSceneViewMenuPrivate::resetMenu()
{
  Q_Q(qMRMLSceneViewMenu);
  Q_ASSERT(this->MRMLScene);
  if (!this->SceneViewsLogic)
  {
    return;
  }

  // Clear menu
  q->clear();

  QAction* noSceneViewAction = q->addAction(this->NoSceneViewText);
  noSceneViewAction->setDisabled(true);

  // Loop over sceneView nodes and associated menu entry
  int sceneViewCount = this->SceneViewsLogic->GetNumberOfSceneViews();
  for (int n = 0; n < sceneViewCount; n++)
  {
    this->addMenuItem(n);
  }
}

// --------------------------------------------------------------------------
void qMRMLSceneViewMenuPrivate::onMRMLNodeAdded(vtkObject* mrmlScene, vtkObject* mrmlNode)
{
  Q_UNUSED(mrmlScene);

  if (!this->MRMLScene || !mrmlNode)
  {
    return;
  }

  vtkMRMLSequenceBrowserNode* sequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(mrmlNode);
  if (!sequenceBrowserNode)
  {
    return;
  }

  if (!this->SceneViewsLogic->IsSceneViewNode(sequenceBrowserNode))
  {
    return;
  }

  // Add observer to the sequence browser node
  qvtkConnect(sequenceBrowserNode, vtkCommand::ModifiedEvent, this, SLOT(resetMenu()));
  qvtkConnect(sequenceBrowserNode, vtkMRMLSequenceBrowserNode::SequenceNodeModifiedEvent, this, SLOT(resetMenu()));
  resetMenu();
}

// --------------------------------------------------------------------------
void qMRMLSceneViewMenuPrivate::addMenuItem(int index)
{
  Q_Q(qMRMLSceneViewMenu);
  if (this->hasNoSceneViewItem())
  {
    q->clear();
  }

  std::string sceneViewName = this->SceneViewsLogic->GetNthSceneViewName(index);
  QMenu* sceneViewMenu = q->addMenu(QString::fromUtf8(sceneViewName.c_str()));
  sceneViewMenu->setObjectName("sceneViewMenu");

  QAction* restoreAction = sceneViewMenu->addAction(QIcon(":/Icons/SnapshotRestore.png"), "Restore", &this->RestoreActionMapper, SLOT(map()));
  this->RestoreActionMapper.setMapping(restoreAction, index);

  QAction* deleteAction = sceneViewMenu->addAction(QIcon(":/Icons/SnapshotDelete.png"), "Delete", &this->DeleteActionMapper, SLOT(map()));
  this->DeleteActionMapper.setMapping(deleteAction, index);
}

// --------------------------------------------------------------------------
void qMRMLSceneViewMenuPrivate::onMRMLNodeRemoved(vtkObject* mrmlScene, vtkObject* mrmlNode)
{
  if (!this->MRMLScene || !mrmlNode)
  {
    return;
  }

  vtkMRMLSequenceBrowserNode* sequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(mrmlNode);
  if (!sequenceBrowserNode)
  {
    return;
  }

  if (this->SceneViewsLogic->IsSceneViewNode(sequenceBrowserNode))
  {
    return;
  }

  // Remove observer from the sequence browser node
  qvtkDisconnect(sequenceBrowserNode, vtkCommand::ModifiedEvent, this, SLOT(resetMenu()));
  qvtkDisconnect(sequenceBrowserNode, vtkMRMLSequenceBrowserNode::SequenceNodeModifiedEvent, this, SLOT(resetMenu()));
  resetMenu();
}

// --------------------------------------------------------------------------
void qMRMLSceneViewMenuPrivate::removeMenuItem(int index)
{
  Q_Q(qMRMLSceneViewMenu);
  if (index < 0 || index >= q->actions().count())
  {
    return;
  }

  QAction* action = q->actions().at(index);
  q->removeAction(action);

  if (q->actions().isEmpty())
  {
    QAction* noSceneViewAction = q->addAction(this->NoSceneViewText);
    noSceneViewAction->setDisabled(true);
  }
}

// --------------------------------------------------------------------------
bool qMRMLSceneViewMenuPrivate::hasNoSceneViewItem() const
{
  Q_Q(const qMRMLSceneViewMenu);
  QList<QAction*> actions = q->actions();
  Q_ASSERT(actions.count() > 0); // At least one item is expected
  return (actions.at(0)->text().compare(this->NoSceneViewText) == 0);
}

// --------------------------------------------------------------------------
void qMRMLSceneViewMenuPrivate::restoreSceneView(int index)
{
  this->MRMLScene->SaveStateForUndo();
  this->SceneViewsLogic->RestoreSceneView(index);
}

// --------------------------------------------------------------------------
void qMRMLSceneViewMenuPrivate::deleteSceneView(int index)
{
  this->MRMLScene->SaveStateForUndo();
  this->SceneViewsLogic->RemoveSceneView(index);
}

// --------------------------------------------------------------------------
// qMRMLSceneViewMenu methods

// --------------------------------------------------------------------------
qMRMLSceneViewMenu::qMRMLSceneViewMenu(QWidget* newParent)
  : Superclass(newParent)
  , d_ptr(new qMRMLSceneViewMenuPrivate(*this))
{
  Q_D(qMRMLSceneViewMenu);
  d->NoSceneViewText = tr("No scene views");
}

// --------------------------------------------------------------------------
qMRMLSceneViewMenu::~qMRMLSceneViewMenu() = default;

//-----------------------------------------------------------------------------
void qMRMLSceneViewMenu::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLSceneViewMenu);
  if (scene == d->MRMLScene)
  {
    return;
  }

  qvtkReconnect(d->MRMLScene, scene, vtkMRMLScene::NodeAddedEvent, d, SLOT(onMRMLNodeAdded(vtkObject*, vtkObject*)));

  qvtkReconnect(d->MRMLScene, scene, vtkMRMLScene::NodeRemovedEvent, d, SLOT(onMRMLNodeRemoved(vtkObject*, vtkObject*)));

  d->MRMLScene = scene;
  emit mrmlSceneChanged(scene);

  if (d->MRMLScene)
  {
    d->resetMenu();
  }
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qMRMLSceneViewMenu::mrmlScene() const
{
  Q_D(const qMRMLSceneViewMenu);
  return d->MRMLScene;
}

//-----------------------------------------------------------------------------
QString qMRMLSceneViewMenu::noSceneViewText() const
{
  Q_D(const qMRMLSceneViewMenu);
  return d->NoSceneViewText;
}

//-----------------------------------------------------------------------------
void qMRMLSceneViewMenu::setNoSceneViewText(const QString& newText)
{
  Q_D(qMRMLSceneViewMenu);
  d->NoSceneViewText = newText;
  emit this->NoSceneViewTextChanged(newText);
}
