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
#include "qMRMLSceneViewMenu_p.h"

// MRML includes
#include <vtkMRMLSceneViewNode.h>


//--------------------------------------------------------------------------
// qMRMLSceneViewMenuPrivate methods

//---------------------------------------------------------------------------
qMRMLSceneViewMenuPrivate::qMRMLSceneViewMenuPrivate(qMRMLSceneViewMenu& object)
  : Superclass(&object), q_ptr(&object)
{
  connect(&this->RestoreActionMapper, SIGNAL(mapped(QString)), SLOT(restoreSceneView(QString)));
  connect(&this->DeleteActionMapper, SIGNAL(mapped(QString)), SLOT(deleteSceneView(QString)));
}

// --------------------------------------------------------------------------
void qMRMLSceneViewMenuPrivate::resetMenu()
{
  Q_Q(qMRMLSceneViewMenu);
  Q_ASSERT(this->MRMLScene);

  // Clear menu
  q->clear();

  QAction * noSceneViewAction = q->addAction(this->NoSceneViewText);
  noSceneViewAction->setDisabled(true);

  // Loop over sceneView nodes and associated menu entry
  const char* className = "vtkMRMLSceneViewNode";
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass(className);
  for (int n = 0; n < nnodes; n++)
    {
    this->addMenuItem(this->MRMLScene->GetNthNodeByClass(n, className));
    }
}

// --------------------------------------------------------------------------
void qMRMLSceneViewMenuPrivate::onMRMLNodeAdded(vtkObject* mrmlScene, vtkObject * mrmlNode)
{
  Q_UNUSED(mrmlScene);
  vtkMRMLSceneViewNode * sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(mrmlNode);
  if (!sceneViewNode)
    {
    return;
    }

  this->addMenuItem(sceneViewNode);
}

// --------------------------------------------------------------------------
void qMRMLSceneViewMenuPrivate::addMenuItem(vtkMRMLNode * sceneViewNode)
{
  Q_Q(qMRMLSceneViewMenu);
  vtkMRMLSceneViewNode * node = vtkMRMLSceneViewNode::SafeDownCast(sceneViewNode);
  if (!node)
    {
    return;
    }

  // Reload the menu each time a sceneView node is modified, if there
  // are performance issues, the relation between menu item and sceneView should
  // be tracked either using a QModel or a QHash
  this->qvtkReconnect(sceneViewNode, vtkCommand::ModifiedEvent,
                      this, SLOT(onMRMLSceneViewNodeModified(vtkObject*)));

  if (this->hasNoSceneViewItem())
    {
    q->clear();
    }

  QMenu* sceneViewMenu = q->addMenu(QString::fromUtf8(node->GetName()));
  sceneViewMenu->setObjectName("sceneViewMenu");

  QAction* restoreAction = sceneViewMenu->addAction(QIcon(":/Icons/SnapshotRestore.png"), "Restore",
                                                   &this->RestoreActionMapper, SLOT(map()));
  this->RestoreActionMapper.setMapping(restoreAction, QString::fromUtf8(node->GetID()));

  QAction* deleteAction = sceneViewMenu->addAction(QIcon(":/Icons/SnapshotDelete.png"), "Delete",
                                                  &this->DeleteActionMapper, SLOT(map()));
  this->DeleteActionMapper.setMapping(deleteAction, QString::fromUtf8(node->GetID()));
}

// --------------------------------------------------------------------------
void qMRMLSceneViewMenuPrivate::onMRMLNodeRemoved(vtkObject* mrmlScene, vtkObject * mrmlNode)
{
  Q_UNUSED(mrmlScene);
  vtkMRMLSceneViewNode * sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(mrmlNode);
  if (!sceneViewNode)
    {
    return;
    }

  this->removeMenuItem(sceneViewNode);
}

// --------------------------------------------------------------------------
void qMRMLSceneViewMenuPrivate::removeMenuItem(vtkMRMLNode * sceneViewNode)
{
  Q_Q(qMRMLSceneViewMenu);
  vtkMRMLSceneViewNode * node = vtkMRMLSceneViewNode::SafeDownCast(sceneViewNode);
  if (!node)
    {
    return;
    }

  // Do not listen for ModifiedEvent anymore
  this->qvtkDisconnect(sceneViewNode, vtkCommand::ModifiedEvent,
                       this, SLOT(onMRMLSceneViewNodeModified(vtkObject*)));

  QList<QAction*> actions = q->actions();
  foreach(QAction * action, actions)
    {
    if (action->text().compare(QString::fromUtf8(node->GetName())) == 0)
      {
      q->removeAction(action);
      break;
      }
    }

  if (q->actions().isEmpty())
    {
    QAction * noSceneViewAction = q->addAction(this->NoSceneViewText);
    noSceneViewAction->setDisabled(true);
    }
}

// --------------------------------------------------------------------------
void qMRMLSceneViewMenuPrivate::onMRMLSceneViewNodeModified(vtkObject * sceneViewNode)
{
  vtkMRMLSceneViewNode * node = vtkMRMLSceneViewNode::SafeDownCast(sceneViewNode);
  if (!node)
    {
    return;
    }

  this->resetMenu();
}

// --------------------------------------------------------------------------
bool qMRMLSceneViewMenuPrivate::hasNoSceneViewItem()const
{
  Q_Q(const qMRMLSceneViewMenu);
  QList<QAction*> actions = q->actions();
  Q_ASSERT(actions.count() > 0); // At least one item is expected
  return (actions.at(0)->text().compare(this->NoSceneViewText) == 0);
}

// --------------------------------------------------------------------------
void qMRMLSceneViewMenuPrivate::restoreSceneView(const QString& sceneViewNodeId)
{
  vtkMRMLSceneViewNode * sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(
      this->MRMLScene->GetNodeByID(sceneViewNodeId.toUtf8()));
  Q_ASSERT(sceneViewNode);
  this->MRMLScene->SaveStateForUndo();
  // pass false to not delete nodes from the scene
  sceneViewNode->RestoreScene(false);
  if (this->MRMLScene->GetErrorCode() != 0)
    {
    QString errorMsg = QString(this->MRMLScene->GetErrorMessage().c_str());
    // reset the error state
    this->MRMLScene->SetErrorCode(0);
    this->MRMLScene->SetErrorMessage("");

    // ask the user if they wish to continue removing the node(s) or
    // add the missing nodes to the scene view
    ctkMessageBox missingNodesMsgBox;
    missingNodesMsgBox.setWindowTitle("Data missing from Scene View");
    QString sceneViewName = QString(sceneViewNode->GetName());
    QString labelText = QString("Add data to scene view \"")
      + sceneViewName
      + QString("\" before restoring?\n"
                "\n");
    QString infoText = QString(
      "Data is present in the current scene but not in the scene view.\n"
      "\n"
      "If you don't add and restore, data not already saved to disk"
      ", or saved in another scene view,"
      " will be permanently lost!\n");
    missingNodesMsgBox.setText(labelText + infoText);
    // until CTK bug is fixed, informative text will overlap the don't show
    // again message so put it all in the label text
    // missingNodesMsgBox.setInformativeText(infoText);
    QPushButton *continueButton = missingNodesMsgBox.addButton(QMessageBox::Discard);
    continueButton->setText("Restore without saving");
    QPushButton *addButton = missingNodesMsgBox.addButton(QMessageBox::Save);
    addButton->setText("Add and Restore");
    missingNodesMsgBox.addButton(QMessageBox::Cancel);

    missingNodesMsgBox.setIcon(QMessageBox::Warning);
    missingNodesMsgBox.setDontShowAgainVisible(true);
    missingNodesMsgBox.setDontShowAgainSettingsKey("SceneViewMenu/AlwaysRemoveNodes");
    int ret = missingNodesMsgBox.exec();
    switch (ret)
      {
      case QMessageBox::Discard:
        sceneViewNode->RestoreScene(true);
        break;
      case QMessageBox::Save:
        sceneViewNode->AddMissingNodes();
        // try to restore again
        this->restoreSceneView(sceneViewNode->GetID());
        break;
      case QMessageBox::Cancel:
      default:
        break;
      }
    }
}

// --------------------------------------------------------------------------
void qMRMLSceneViewMenuPrivate::deleteSceneView(const QString& sceneViewNodeId)
{
  vtkMRMLSceneViewNode * sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(
      this->MRMLScene->GetNodeByID(sceneViewNodeId.toUtf8()));
  Q_ASSERT(sceneViewNode);
  this->MRMLScene->SaveStateForUndo();
  this->MRMLScene->RemoveNode(sceneViewNode);
}

// --------------------------------------------------------------------------
// qMRMLSceneViewMenu methods

// --------------------------------------------------------------------------
qMRMLSceneViewMenu::qMRMLSceneViewMenu(QWidget* newParent) : Superclass(newParent)
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
    return ;
    }

  qvtkReconnect(d->MRMLScene, scene,
                vtkMRMLScene::NodeAddedEvent, d, SLOT(onMRMLNodeAdded(vtkObject*,vtkObject*)));

  qvtkReconnect(d->MRMLScene, scene,
                vtkMRMLScene::NodeRemovedEvent, d, SLOT(onMRMLNodeRemoved(vtkObject*,vtkObject*)));

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
QString qMRMLSceneViewMenu::noSceneViewText()const
{
  Q_D(const qMRMLSceneViewMenu);
  return d->NoSceneViewText;
}

//-----------------------------------------------------------------------------
void qMRMLSceneViewMenu::setNoSceneViewText(const QString& newText)
{
  Q_D(qMRMLSceneViewMenu);
  d->NoSceneViewText = newText;
}
