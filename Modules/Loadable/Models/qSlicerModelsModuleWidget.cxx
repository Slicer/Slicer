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
#include <QInputDialog>
#include <QMessageBox>
#include <QModelIndex>

// qMRMLWidgets
#include "qMRMLSceneModelHierarchyModel.h"

// SlicerQt includes
#include "qSlicerModelsModuleWidget.h"
#include "ui_qSlicerModelsModuleWidget.h"

// Logic includes
#include "vtkSlicerModelsLogic.h"

// MRML includes
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLScene.h"

#include <vtkMRMLDisplayableHierarchyLogic.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Models
class qSlicerModelsModuleWidgetPrivate: public Ui_qSlicerModelsModuleWidget
{
public:
  qSlicerModelsModuleWidgetPrivate();
  QAction *InsertHierarchyAction;
  QAction *DeleteMultipleNodesAction;
  QAction *RenameMultipleNodesAction;
};

//-----------------------------------------------------------------------------
// qSlicerModelsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerModelsModuleWidgetPrivate::qSlicerModelsModuleWidgetPrivate()
{
  this->InsertHierarchyAction = 0;
  this->DeleteMultipleNodesAction = 0;
  this->RenameMultipleNodesAction = 0;
}

//-----------------------------------------------------------------------------
// qSlicerModelsModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerModelsModuleWidget::qSlicerModelsModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerModelsModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerModelsModuleWidget::~qSlicerModelsModuleWidget()
{
  // set the mrml scene to null so that stop observing it for events
  this->setMRMLScene(0);
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::setup()
{
  Q_D(qSlicerModelsModuleWidget);

  d->setupUi(this);
  d->ClipModelsNodeComboBox->setVisible(false);

  this->updateTreeViewModel();

  // turn of setting of size to visible indexes to allow drag scrolling
  d->ModelHierarchyTreeView->setFitSizeToVisibleIndexes(false);

  // add an add hierarchy right click action on the scene and hierarchy nodes
  connect(d->ModelHierarchyTreeView,  SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onCurrentNodeChanged(vtkMRMLNode*)) );

  d->InsertHierarchyAction = new QAction(tr("Insert hierarchy"), this);
  d->ModelHierarchyTreeView->prependSceneMenuAction(d->InsertHierarchyAction);
  connect(d->InsertHierarchyAction, SIGNAL(triggered()),
          this, SLOT(insertHierarchyNode()));

  // customize the right click menu to offer a delete multiple nodes option,
  // and deal with the hierarchies associated with model nodes
  d->DeleteMultipleNodesAction = new QAction(tr("Delete Model(s)"), this);
  d->DeleteMultipleNodesAction->setToolTip(tr("Delete one or more models and/or hierarchies, along with the 1:1 hierarchy nodes that may be associated with them"));
  d->ModelHierarchyTreeView->appendNodeMenuAction(d->DeleteMultipleNodesAction);
  connect(d->DeleteMultipleNodesAction, SIGNAL(triggered()),
          this, SLOT(deleteMultipleModels()));

  // customise the right click menu to offer a rename multiple nodes option
  d->RenameMultipleNodesAction = new QAction(tr("Rename Model(s)"), this);
  d->RenameMultipleNodesAction->setToolTip(tr("Rename one or more models and/or hierarchies"));
  d->ModelHierarchyTreeView->appendNodeMenuAction(d->RenameMultipleNodesAction);
  connect(d->RenameMultipleNodesAction, SIGNAL(triggered()),
          this, SLOT(renameMultipleModels()));

  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::updateTreeViewModel()
{
  Q_D(qSlicerModelsModuleWidget);

  d->ModelHierarchyTreeView->setSceneModelType(QString("ModelHierarchy"));
  qMRMLSceneModelHierarchyModel* sceneModel =
    qobject_cast<qMRMLSceneModelHierarchyModel*>(
      d->ModelHierarchyTreeView->sceneModel());
  sceneModel->setIDColumn(-1);
  sceneModel->setExpandColumn(1);
  sceneModel->setColorColumn(2);
  sceneModel->setOpacityColumn(3);

  d->ModelHierarchyTreeView->header()->setStretchLastSection(false);
  d->ModelHierarchyTreeView->header()->setResizeMode(sceneModel->nameColumn(), QHeaderView::Stretch);
  d->ModelHierarchyTreeView->header()->setResizeMode(sceneModel->expandColumn(), QHeaderView::ResizeToContents);
  d->ModelHierarchyTreeView->header()->setResizeMode(sceneModel->colorColumn(), QHeaderView::ResizeToContents);
  d->ModelHierarchyTreeView->header()->setResizeMode(sceneModel->opacityColumn(), QHeaderView::ResizeToContents);

  d->ModelHierarchyTreeView->sortFilterProxyModel()->setHideChildNodeTypes(
    QStringList() << "vtkMRMLFiberBundleNode" << "vtkMRMLAnnotationNode");

  // use lazy update instead of responding to scene import end event
  sceneModel->setLazyUpdate(true);

  // qDebug() << "qSlicerModelsModuleWidget::updateTreeViewModel done";
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::insertHierarchyNode()
{
  Q_D(qSlicerModelsModuleWidget);

  vtkMRMLModelHierarchyNode *modelHierarchyNode = vtkMRMLModelHierarchyNode::New();
  modelHierarchyNode->SetName(this->mrmlScene()->GetUniqueNameByString("Model Hierarchy"));

  // also add a display node to the hierarchy node for use when the hierarchy is collapsed
  vtkMRMLModelDisplayNode *modelDisplayNode = vtkMRMLModelDisplayNode::New();
  if (modelDisplayNode)
    {
    this->mrmlScene()->AddNode(modelDisplayNode);
    // qDebug() << "insertHierarchyNode: added a display node for hierarchy node, with id = " << modelDisplayNode->GetID();
    }

  this->mrmlScene()->AddNode(modelHierarchyNode);

  vtkMRMLNode* parent = vtkMRMLNode::SafeDownCast(d->ModelHierarchyTreeView->currentNode());
  if (parent)
    {
    QModelIndex parentIndex = d->ModelHierarchyTreeView->sortFilterProxyModel()->
                  indexFromMRMLNode(parent);
    bool parentExpanded = d->ModelHierarchyTreeView->isExpanded(parentIndex);
    modelHierarchyNode->SetParentNodeID(parent->GetID());
    if (parentExpanded)
      {
      d->ModelHierarchyTreeView->expand(parentIndex);
      }
    }

  // Expand the newly added hierarchy node
  QModelIndex modelHierarchyIndex = d->ModelHierarchyTreeView->
                                    sortFilterProxyModel()->
                                    indexFromMRMLNode(modelHierarchyNode);
  d->ModelHierarchyTreeView->expand(modelHierarchyIndex);

  if (modelDisplayNode)
    {
    modelHierarchyNode->SetAndObserveDisplayNodeID(modelDisplayNode->GetID());
    modelDisplayNode->Delete();
    }

  modelHierarchyNode->Delete();
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::deleteMultipleModels()
{
  Q_D(qSlicerModelsModuleWidget);

  if (!this->mrmlScene())
    {
    qWarning("No mrml scene set, cannot delete models");
    return;
    }

  // get all rows where column 2, the model name, is selected
  int nameColumn = d->ModelHierarchyTreeView->sceneModel()->nameColumn();
  QModelIndexList indexList = d->ModelHierarchyTreeView->selectionModel()->selectedRows(nameColumn);
  // have to build up a list of nodes to delete, as the node from index call will
  // fail as the tree is updated while the nodes are being deleted.
  QStringList modelIDsToDelete;
  for (int i = 0; i < indexList.size(); ++i)
    {
    QModelIndex index = indexList.at(i);
    vtkMRMLNode *nodeToDelete =  d->ModelHierarchyTreeView->sortFilterProxyModel()->mrmlNodeFromIndex(index);
    if (nodeToDelete && nodeToDelete->GetID())
      {
      modelIDsToDelete << nodeToDelete->GetID();
      }
    else
      {
      qWarning() << "selection index " << i << " has no model";
      }
    }
  for (int i = 0; i < modelIDsToDelete.size(); i++)
    {
    QString modelID = modelIDsToDelete.at(i);
    vtkMRMLNode *mrmlNode = this->mrmlScene()->GetNodeByID(modelID.toLatin1());
    if (mrmlNode && mrmlNode->IsA("vtkMRMLModelNode"))
      {
      // get the model hierarchy node and delete it
      vtkMRMLHierarchyNode *hnode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(mrmlNode->GetScene(), mrmlNode->GetID());
      if (hnode)
        {
        //qDebug() << i << ": removing hierarchy " << (hnode ? hnode->GetID() : "null");
        this->mrmlScene()->RemoveNode(hnode);
        }
      // remove the model node
      //qDebug() << i << ": removing model node " << mrmlNode->GetName() << ", id = " << mrmlNode->GetID();
      this->mrmlScene()->RemoveNode(mrmlNode);
      }
    else if (mrmlNode && mrmlNode->IsA("vtkMRMLModelHierarchyNode"))
      {
      // Deleting a model hierarchy node and everything under it
      // confirm first
      QMessageBox confirmBox;
      QString msg = QString("Delete ") + QString(mrmlNode->GetName()) + QString(" and all children?");
      confirmBox.setText(msg);
      confirmBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
      confirmBox.setDefaultButton(QMessageBox::Cancel);
      int ret = confirmBox.exec();
      switch (ret) {
        case QMessageBox::Ok :
          {
          // delete
          vtkMRMLDisplayableHierarchyLogic *hierarchyLogic = vtkMRMLDisplayableHierarchyLogic::New();
          hierarchyLogic->SetMRMLScene(this->mrmlScene());
          bool retval = hierarchyLogic->DeleteHierarchyNodeAndChildren(vtkMRMLDisplayableHierarchyNode::SafeDownCast(mrmlNode));
          if (!retval)
            {
            qWarning() << "Failed to delete hierarchy and children!";
            }
          hierarchyLogic->Delete();
          break;
          }
        default:
          {
          qWarning() << "Not deleting hierarchy";
          }
        }
      }
    else
      {
      qWarning() << "Unable to delete model using node id" << qPrintable(modelID);
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::renameMultipleModels()
{
  Q_D(qSlicerModelsModuleWidget);

  if (!this->mrmlScene())
    {
    qWarning("No mrml scene set, cannot rename models");
    return;
    }

  // get all rows where column 2, the model name, is selected
  int nameColumn = d->ModelHierarchyTreeView->sceneModel()->nameColumn();
  QModelIndexList indexList = d->ModelHierarchyTreeView->selectionModel()->selectedRows(nameColumn);
  for (int i = 0; i < indexList.size(); ++i)
    {
    QModelIndex index = indexList.at(i);
    vtkMRMLNode *mrmlNode =  d->ModelHierarchyTreeView->sortFilterProxyModel()->mrmlNodeFromIndex(index);
    if (mrmlNode &&
        (mrmlNode->IsA("vtkMRMLModelNode") || mrmlNode->IsA("vtkMRMLModelHierarchyNode")))
      {
      // pop up an entry box for the new name, with the old name as default
      QString oldName = mrmlNode->GetName();
      bool ok = false;
      QString newName = QInputDialog::getText(
                        this, "Rename " + oldName, "Old name: " + oldName + "\nNew name:",
                        QLineEdit::Normal, oldName, &ok);
      if (ok)
        {
        mrmlNode->SetName(newName.toLatin1());
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::onCurrentNodeChanged(vtkMRMLNode* newCurrentNode)
{
  Q_D(qSlicerModelsModuleWidget);

  // only allow adding hierarchies when right click on hierarchies
  vtkMRMLModelHierarchyNode* hierarchyNode =
    vtkMRMLModelHierarchyNode::SafeDownCast(newCurrentNode);
  if (hierarchyNode)
    {
    d->ModelHierarchyTreeView->prependNodeMenuAction(d->InsertHierarchyAction);
    }
  else
    {
    d->ModelHierarchyTreeView->removeNodeMenuAction(d->InsertHierarchyAction);
    }
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  //Q_D(qSlicerModelsModuleWidget);

  if (scene == this->mrmlScene())
    {
    return;
    }
  this->Superclass::setMRMLScene(scene);
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::showAllModels()
{
  if (this->logic() == 0)
    {
    return;
    }
  vtkSlicerModelsLogic *modelsLogic = vtkSlicerModelsLogic::SafeDownCast(this->logic());
  if (modelsLogic)
    {
    modelsLogic->SetAllModelsVisibility(1);
    }
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::hideAllModels()
{
  if (this->logic() == 0)
    {
    return;
    }
  vtkSlicerModelsLogic *modelsLogic = vtkSlicerModelsLogic::SafeDownCast(this->logic());
  if (modelsLogic)
    {
    modelsLogic->SetAllModelsVisibility(0);
    }
}
