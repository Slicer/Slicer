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

#include <vtkCallbackCommand.h>

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
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>

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
  QStringList HideChildNodeTypes;
  QString FiberDisplayClass;
  vtkMRMLSelectionNode* SelectionNode;
  vtkSmartPointer<vtkCallbackCommand> CallBack;
};

//-----------------------------------------------------------------------------
// qSlicerModelsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerModelsModuleWidgetPrivate::qSlicerModelsModuleWidgetPrivate()
{
  this->InsertHierarchyAction = 0;
  this->DeleteMultipleNodesAction = 0;
  this->RenameMultipleNodesAction = 0;
  this->HideChildNodeTypes = (QStringList() << "vtkMRMLFiberBundleNode" << "vtkMRMLAnnotationNode");
  this->FiberDisplayClass = "vtkMRMLFiberBundleLineDisplayNode";
  this->CallBack = vtkSmartPointer<vtkCallbackCommand>::New();
  this->SelectionNode = 0;
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
  Q_D(qSlicerModelsModuleWidget);

  // set the mrml scene to null so that stop observing it for events
  if (this->mrmlScene())
    {
    this->mrmlScene()->RemoveObserver(d->CallBack);
    }

  this->setMRMLScene(0);
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::setup()
{
  Q_D(qSlicerModelsModuleWidget);

  d->setupUi(this);

  d->ClipModelsNodeComboBox->setVisible(false);

  d->DisplayClassTabWidget->setVisible(false);

  d->ModelHierarchyTreeView->setSelectionMode(QAbstractItemView::SingleSelection);

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

  connect(d->IncludeFiberBundleCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(includeFiberBundles(bool)));

  connect( d->DisplayClassTabWidget, SIGNAL(currentChanged(int)),
           this, SLOT(onDisplayClassChanged(int)) );

  this->updateTreeViewModel();

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

  d->ModelHierarchyTreeView->sortFilterProxyModel()->setHideChildNodeTypes(d->HideChildNodeTypes);

  d->ModelHierarchyTreeView->sortFilterProxyModel()->invalidate();

    // use lazy update instead of responding to scene import end event
  sceneModel->setLazyUpdate(true);

  // qDebug() << "qSlicerModelsModuleWidget::updateTreeViewModel done";
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::insertHierarchyNode()
{
  Q_D(qSlicerModelsModuleWidget);

  vtkNew<vtkMRMLModelHierarchyNode> modelHierarchyNode;
  modelHierarchyNode->SetName(this->mrmlScene()->GetUniqueNameByString("Model Hierarchy"));

  // also add a display node to the hierarchy node for use when the hierarchy is collapsed
  vtkNew<vtkMRMLModelDisplayNode> modelDisplayNode;
  this->mrmlScene()->AddNode(modelDisplayNode.GetPointer());
  // qDebug() << "insertHierarchyNode: added a display node for hierarchy node, with id = " << modelDisplayNode->GetID();

  this->mrmlScene()->AddNode(modelHierarchyNode.GetPointer());

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
                                    indexFromMRMLNode(modelHierarchyNode.GetPointer());
  d->ModelHierarchyTreeView->expand(modelHierarchyIndex);

  if (modelDisplayNode.GetPointer())
    {
    modelHierarchyNode->SetAndObserveDisplayNodeID(modelDisplayNode->GetID());
    }
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
          vtkNew<vtkMRMLDisplayableHierarchyLogic> hierarchyLogic;
          hierarchyLogic->SetMRMLScene(this->mrmlScene());
          bool retval = hierarchyLogic->DeleteHierarchyNodeAndChildren(vtkMRMLDisplayableHierarchyNode::SafeDownCast(mrmlNode));
          if (!retval)
            {
            qWarning() << "Failed to delete hierarchy and children!";
            }
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
  Q_D(qSlicerModelsModuleWidget);
  //Q_Q(qSlicerModelsModuleWidget);

  if (scene == this->mrmlScene())
    {
    return;
    }
  this->Superclass::setMRMLScene(scene);

  if (scene)
    {
    scene->RemoveObserver(d->CallBack);
    }
  if (scene)
    {
    d->CallBack->SetClientData(this);

    d->CallBack->SetCallback(qSlicerModelsModuleWidget::onMRMLSceneEvent);

    scene->AddObserver(vtkMRMLScene::EndImportEvent, d->CallBack);
    this->onMRMLSceneEvent(scene, vtkMRMLScene::EndImportEvent, this, 0);
    }
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::onMRMLSceneEvent(vtkObject* vtk_obj, unsigned long event,
                                                void* client_data, void* call_data)
{
  vtkMRMLScene* scene = reinterpret_cast<vtkMRMLScene*>(vtk_obj);
  qSlicerModelsModuleWidget* widget = reinterpret_cast<qSlicerModelsModuleWidget*>(client_data);
  vtkMRMLNode* node = reinterpret_cast<vtkMRMLNode*>(call_data);
  Q_UNUSED(node);
  Q_ASSERT(scene);
  Q_UNUSED(scene);
  Q_ASSERT(widget);
  if (event == vtkMRMLScene::EndImportEvent)
    {
    widget->updateWidgetFromSelectionNode();
    }
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

void qSlicerModelsModuleWidget::includeFiberBundles(bool include)
{
  Q_D(qSlicerModelsModuleWidget);

  // update selection node
  vtkMRMLSelectionNode* selectionNode = this->getSelectionNode();
  if (selectionNode)
    {
    selectionNode->ClearModelHierarchyDisplayNodeClassNames();
    if (include)
      {
      selectionNode->AddModelHierarchyDisplayNodeClassName("vtkMRMLFiberBundleNode",
                                                          d->FiberDisplayClass.toStdString());
      }
    }

  this->updateWidgetFromSelectionNode();
}

void qSlicerModelsModuleWidget::onDisplayClassChanged(int index)
{
  Q_D(qSlicerModelsModuleWidget);

  std::string name;
  if (index == 0)
    {
    name = std::string("vtkMRMLFiberBundleLineDisplayNode");
    }
  else if (index == 1)
    {
    name = std::string("vtkMRMLFiberBundleTubeDisplayNode");
    }
  else if (index == 2)
    {
    name = std::string("vtkMRMLFiberBundleGlyphDisplayNode");
    }

  d->FiberDisplayClass.fromStdString(name);

  vtkMRMLSelectionNode* selectionNode = this->getSelectionNode();
  if (selectionNode)
    {
    selectionNode->ClearModelHierarchyDisplayNodeClassNames();
    selectionNode->AddModelHierarchyDisplayNodeClassName("vtkMRMLFiberBundleNode",
                                                         name);
    }
  this->updateWidgetFromSelectionNode();
}

vtkMRMLSelectionNode* qSlicerModelsModuleWidget::getSelectionNode()
{
  Q_D(qSlicerModelsModuleWidget);

  if (d->SelectionNode == 0)
    {
    std::vector<vtkMRMLNode *> selectionNodes;
    if (this->mrmlScene())
      {
      this->mrmlScene()->GetNodesByClass("vtkMRMLSelectionNode", selectionNodes);
      }

    if (selectionNodes.size() > 0)
      {
      d->SelectionNode = vtkMRMLSelectionNode::SafeDownCast(selectionNodes[0]);
      }
    }
  return d->SelectionNode;
}

void qSlicerModelsModuleWidget::updateWidgetFromSelectionNode()
{
  Q_D(qSlicerModelsModuleWidget);

  vtkMRMLSelectionNode* selectionNode = this->getSelectionNode();

  bool include = false;
  std::string displayNodeClass;
  if (selectionNode)
    {
    displayNodeClass = selectionNode->GetModelHierarchyDisplayNodeClassName("vtkMRMLFiberBundleNode");
    include = !displayNodeClass.empty();
    }

  if (include)
    {
    d->HideChildNodeTypes = (QStringList() << "vtkMRMLAnnotationNode");
    d->DisplayClassTabWidget->setVisible(true);
    }
  else
    {
    d->HideChildNodeTypes = (QStringList() << "vtkMRMLFiberBundleNode" << "vtkMRMLAnnotationNode");
    d->DisplayClassTabWidget->setVisible(false);
    }

  if (include)
    {
    d->FiberDisplayClass.fromStdString(displayNodeClass);
    int index = 0;
    if (displayNodeClass == std::string("vtkMRMLFiberBundleTubeDisplayNode"))
      {
      index = 1;
      }
    else if (displayNodeClass == std::string("vtkMRMLFiberBundleGlyphDisplayNode"))
      {
      index = 2;
      }
      d->DisplayClassTabWidget->setCurrentIndex(index);
    }

  d->ModelHierarchyTreeView->sortFilterProxyModel()->setHideChildNodeTypes(d->HideChildNodeTypes);

  d->ModelHierarchyTreeView->sortFilterProxyModel()->invalidate();

  if (include)
    {
    // force update mrml widgets
    std::vector<vtkMRMLNode *> nodes;
    vtkMRMLScene *scene = this->mrmlScene();
    scene->GetNodesByClass(displayNodeClass.c_str(), nodes);
    for (unsigned int i = 0; i < nodes.size(); i++)
      {
      nodes[i]->InvokeEvent(vtkCommand::ModifiedEvent);
      }
    }
  d->ModelDisplayWidget->setMRMLModelOrHierarchyNode(d->ModelDisplayWidget->mrmlDisplayableNode());
}
