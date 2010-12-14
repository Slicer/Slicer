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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>

// qMRML includes
#include "qMRMLSceneSnapshotMenu.h"
#include "qMRMLSceneSnapshotMenu_p.h"

// MRML includes
#include <vtkMRMLSceneViewNode.h>


//--------------------------------------------------------------------------
// qMRMLSceneSnapshotMenuPrivate methods

//---------------------------------------------------------------------------
qMRMLSceneSnapshotMenuPrivate::qMRMLSceneSnapshotMenuPrivate(qMRMLSceneSnapshotMenu& object)
  : Superclass(&object), q_ptr(&object)
{
  connect(&this->RestoreActionMapper, SIGNAL(mapped(QString)), SLOT(restoreSnapshot(QString)));
  connect(&this->DeleteActionMapper, SIGNAL(mapped(QString)), SLOT(deleteSnapshot(QString)));
}

// --------------------------------------------------------------------------
void qMRMLSceneSnapshotMenuPrivate::resetMenu()
{
  Q_Q(qMRMLSceneSnapshotMenu);
  Q_ASSERT(this->MRMLScene);

  // Clear menu
  q->clear();

  QAction * noSnapshotAction = q->addAction(this->NoSnapshotText);
  noSnapshotAction->setDisabled(true);

  // Loop over snapshot nodes and associated menu entry
  const char* className = "vtkMRMLSceneViewNode";
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass(className);
  for (int n = 0; n < nnodes; n++)
    {
    this->addMenuItem(this->MRMLScene->GetNthNodeByClass(n, className));
    }
}

// --------------------------------------------------------------------------
void qMRMLSceneSnapshotMenuPrivate::onMRMLNodeAdded(vtkObject* mrmlScene, vtkObject * mrmlNode)
{
  Q_UNUSED(mrmlScene);
  vtkMRMLSceneViewNode * snapshotNode = vtkMRMLSceneViewNode::SafeDownCast(mrmlNode);
  if (!snapshotNode)
    {
    return;
    }

  this->addMenuItem(snapshotNode);
}

// --------------------------------------------------------------------------
void qMRMLSceneSnapshotMenuPrivate::addMenuItem(vtkMRMLNode * snapshotNode)
{
  Q_Q(qMRMLSceneSnapshotMenu);
  vtkMRMLSceneViewNode * node = vtkMRMLSceneViewNode::SafeDownCast(snapshotNode);
  if (!node)
    {
    return;
    }

  // Reload the menu each time a snapshot node is modified, if there
  // are performance issues, the relation between menu item and snapshot should
  // be tracked either using a QModel or a QHash
  this->qvtkReconnect(snapshotNode, vtkCommand::ModifiedEvent,
                      this, SLOT(onMRMLSceneSnaphodeNodeModified(vtkObject*)));

  if (this->hasNoSnapshotItem())
    {
    q->clear();
    }

  QMenu* snapshotMenu = q->addMenu(QLatin1String(node->GetName()));

  QAction* restoreAction = snapshotMenu->addAction(QIcon(":/Icons/SnapshotRestore.png"), "Restore",
                                                   &this->RestoreActionMapper, SLOT(map()));
  this->RestoreActionMapper.setMapping(restoreAction, QLatin1String(node->GetID()));

  QAction* deleteAction = snapshotMenu->addAction(QIcon(":/Icons/SnapshotDelete.png"), "Delete",
                                                  &this->DeleteActionMapper, SLOT(map()));
  this->DeleteActionMapper.setMapping(deleteAction, QLatin1String(node->GetID()));
}

// --------------------------------------------------------------------------
void qMRMLSceneSnapshotMenuPrivate::onMRMLNodeRemoved(vtkObject* mrmlScene, vtkObject * mrmlNode)
{
  Q_UNUSED(mrmlScene);
  vtkMRMLSceneViewNode * snapshotNode = vtkMRMLSceneViewNode::SafeDownCast(mrmlNode);
  if (!snapshotNode)
    {
    return;
    }

  this->removeMenuItem(snapshotNode);
}

// --------------------------------------------------------------------------
void qMRMLSceneSnapshotMenuPrivate::removeMenuItem(vtkMRMLNode * snapshotNode)
{
  Q_Q(qMRMLSceneSnapshotMenu);
  vtkMRMLSceneViewNode * node = vtkMRMLSceneViewNode::SafeDownCast(snapshotNode);
  if (!node)
    {
    return;
    }

  // Do not listen for ModifiedEvent anymore
  this->qvtkDisconnect(snapshotNode, vtkCommand::ModifiedEvent,
                       this, SLOT(onMRMLSceneSnaphodeNodeModified(vtkObject*)));

  QList<QAction*> actions = q->actions();
  foreach(QAction * action, actions)
    {
    if (action->text().compare(QLatin1String(node->GetName())) == 0)
      {
      q->removeAction(action);
      break;
      }
    }

  if (q->actions().isEmpty())
    {
    QAction * noSnapshotAction = q->addAction(this->NoSnapshotText);
    noSnapshotAction->setDisabled(true);
    }
}

// --------------------------------------------------------------------------
void qMRMLSceneSnapshotMenuPrivate::onMRMLSceneSnaphodeNodeModified(vtkObject * snapshotNode)
{
  vtkMRMLSceneViewNode * node = vtkMRMLSceneViewNode::SafeDownCast(snapshotNode);
  if (!node)
    {
    return;
    }

  this->resetMenu();
}

// --------------------------------------------------------------------------
bool qMRMLSceneSnapshotMenuPrivate::hasNoSnapshotItem()const
{
  Q_Q(const qMRMLSceneSnapshotMenu);
  QList<QAction*> actions = q->actions();
  Q_ASSERT(actions.count() > 0); // At least one item is expected
  return (actions.at(0)->text().compare(this->NoSnapshotText) == 0);
}

// --------------------------------------------------------------------------
void qMRMLSceneSnapshotMenuPrivate::restoreSnapshot(const QString& snapshotNodeId)
{
  vtkMRMLSceneViewNode * snapshotNode = vtkMRMLSceneViewNode::SafeDownCast(
      this->MRMLScene->GetNodeByID(snapshotNodeId.toLatin1()));
  Q_ASSERT(snapshotNode);
  this->MRMLScene->SaveStateForUndo();
  snapshotNode->RestoreScene();
}

// --------------------------------------------------------------------------
void qMRMLSceneSnapshotMenuPrivate::deleteSnapshot(const QString& snapshotNodeId)
{
  vtkMRMLSceneViewNode * snapshotNode = vtkMRMLSceneViewNode::SafeDownCast(
      this->MRMLScene->GetNodeByID(snapshotNodeId.toLatin1()));
  Q_ASSERT(snapshotNode);
  this->MRMLScene->SaveStateForUndo();
  this->MRMLScene->RemoveNode(snapshotNode);
}

// --------------------------------------------------------------------------
// qMRMLSceneSnapshotMenu methods

// --------------------------------------------------------------------------
qMRMLSceneSnapshotMenu::qMRMLSceneSnapshotMenu(QWidget* newParent) : Superclass(newParent)
  , d_ptr(new qMRMLSceneSnapshotMenuPrivate(*this))
{
  Q_D(qMRMLSceneSnapshotMenu);
  d->NoSnapshotText = tr("No snapshots");
}

// --------------------------------------------------------------------------
qMRMLSceneSnapshotMenu::~qMRMLSceneSnapshotMenu()
{
}

//-----------------------------------------------------------------------------
void qMRMLSceneSnapshotMenu::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLSceneSnapshotMenu);
  if (scene == d->MRMLScene)
    {
    return ;
    }

  qvtkReconnect(d->MRMLScene, scene,
                vtkMRMLScene::NodeAddedEvent, d, SLOT(onMRMLNodeAdded(vtkObject*, vtkObject*)));

  qvtkReconnect(d->MRMLScene, scene,
                vtkMRMLScene::NodeRemovedEvent, d, SLOT(onMRMLNodeRemoved(vtkObject*, vtkObject*)));

  d->MRMLScene = scene;
  emit mrmlSceneChanged(scene);

  if (d->MRMLScene)
    {
    d->resetMenu();
    }
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qMRMLSceneSnapshotMenu::mrmlScene() const
{
  Q_D(const qMRMLSceneSnapshotMenu);
  return d->MRMLScene;
}

//-----------------------------------------------------------------------------
QString qMRMLSceneSnapshotMenu::noSnapshotText()const
{
  Q_D(const qMRMLSceneSnapshotMenu);
  return d->NoSnapshotText;
}

//-----------------------------------------------------------------------------
void qMRMLSceneSnapshotMenu::setNoSnapshotText(const QString& newText)
{
  Q_D(qMRMLSceneSnapshotMenu);
  d->NoSnapshotText = newText;
}
