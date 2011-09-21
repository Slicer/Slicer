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
#include <QModelIndex>

// CTK includes
#include <ctkLogger.h>

// qMRMLWidgets
#include "qMRMLSceneModelHierarchyModel.h"

// SlicerQt includes
#include "qSlicerModelsModuleWidget.h"
#include "ui_qSlicerModelsModule.h"

// MRML includes
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLScene.h"



static ctkLogger logger("org.slicer.qtmodules.models.qSlicerModelsModule");

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Models
class qSlicerModelsModuleWidgetPrivate: public Ui_qSlicerModelsModule
{
public:
  qSlicerModelsModuleWidgetPrivate();
  QAction *InsertHierarchyAction;
};

//-----------------------------------------------------------------------------
// qSlicerModelsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerModelsModuleWidgetPrivate::qSlicerModelsModuleWidgetPrivate()
{
  this->InsertHierarchyAction = 0;
}

//-----------------------------------------------------------------------------
// qSlicerModelsModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerModelsModuleWidget::qSlicerModelsModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerModelsModuleWidgetPrivate )
{
  logger.setOff();
}

//-----------------------------------------------------------------------------
qSlicerModelsModuleWidget::~qSlicerModelsModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::setup()
{
  Q_D(qSlicerModelsModuleWidget);

  logger.trace("setup");
  
  d->setupUi(this);
  d->ClipModelsNodeComboBox->setVisible(false);

  qobject_cast<qMRMLSceneModelHierarchyModel*>(
    d->ModelHierarchyTreeView->sceneModel())->setIDColumn(-1);
  qobject_cast<qMRMLSceneModelHierarchyModel*>(
    d->ModelHierarchyTreeView->sceneModel())->setColorColumn(1);
  qobject_cast<qMRMLSceneModelHierarchyModel*>(
    d->ModelHierarchyTreeView->sceneModel())->setOpacityColumn(2);

  qobject_cast<qMRMLSceneModelHierarchyModel*>(
    d->ModelHierarchyTreeView->sceneModel())->setColumnCount(3);
  d->ModelHierarchyTreeView->header()->setStretchLastSection(false);
  d->ModelHierarchyTreeView->header()->setResizeMode(0, QHeaderView::Stretch);
  d->ModelHierarchyTreeView->header()->setResizeMode(1, QHeaderView::ResizeToContents);
  d->ModelHierarchyTreeView->header()->setResizeMode(2, QHeaderView::ResizeToContents);

  d->ModelHierarchyTreeView->sortFilterProxyModel()->setHideChildNodeTypes(
    QStringList() << "vtkMRMLFiberBundleNode" << "vtkMRMLAnnotationNode");
  d->ModelHierarchyTreeView->sortFilterProxyModel()->setShowHiddenForTypes(
    QStringList() << "vtkMRMLModelHierarchyNode");

  // add an add hierarchy right click action on the scene and hierarchy nodes
  connect(d->ModelHierarchyTreeView,  SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onCurrentNodeChanged(vtkMRMLNode*)) );
  bool expRet = connect(d->ModelHierarchyTreeView, SIGNAL(expanded(QModelIndex)),
          this, SLOT(onExpanded(QModelIndex)));
  bool colRet = connect(d->ModelHierarchyTreeView, SIGNAL(collapsed(QModelIndex)),
          this, SLOT(onCollapsed(QModelIndex)));
  if (!expRet)
    {
    logger.trace("Connecting expanded failed");
    }
  if (!colRet)
    {
    logger.trace("Connecting collapsed failed");
    }

  d->InsertHierarchyAction = new QAction(tr("Insert hierarchy"), this);
  d->ModelHierarchyTreeView->prependSceneMenuAction(d->InsertHierarchyAction);
  connect(d->InsertHierarchyAction, SIGNAL(triggered()),
          this, SLOT(insertHierarchyNode()));
  

  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::insertHierarchyNode()
{
  Q_D(qSlicerModelsModuleWidget);
  
  vtkMRMLModelHierarchyNode *modelHierarchyNode = vtkMRMLModelHierarchyNode::New();

  vtkMRMLNode* parent = vtkMRMLNode::SafeDownCast(d->ModelHierarchyTreeView->currentNode());
  if (parent)
    {
    modelHierarchyNode->SetParentNodeID(parent->GetID());
    }
  this->mrmlScene()->AddNode(modelHierarchyNode);
  modelHierarchyNode->SetName(this->mrmlScene()->GetUniqueNameByString("Model Hierarchy"));

  // also add a display node to the hierarchy node for use when the hierarchy is collapsed
  vtkMRMLModelDisplayNode *modelDisplayNode = vtkMRMLModelDisplayNode::New();
  if (modelDisplayNode)
    {
    this->mrmlScene()->AddNode(modelDisplayNode);
    modelHierarchyNode->SetAndObserveDisplayNodeID(modelDisplayNode->GetID());

    modelDisplayNode->Delete();
    }
  modelHierarchyNode->Delete();
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
void qSlicerModelsModuleWidget::onCollapsed(const QModelIndex & index)
{
  Q_D(qSlicerModelsModuleWidget);
  logger.trace("onCollapsed");
  
  vtkMRMLNode *node = d->ModelHierarchyTreeView->sortFilterProxyModel()->mrmlNodeFromIndex(index);
  if (!node)
    {
    logger.trace("onCollapsed: node not found for this index");
    return;
    }
  vtkMRMLDisplayableHierarchyNode *hnode = vtkMRMLDisplayableHierarchyNode::SafeDownCast(node);
  if (!hnode)
    {
    logger.warn("onCollapsed: node is not a displayable hierarchy node");
    return;
    }
  logger.trace(QString("onCollapsed: found displayable hierarchy node ") + QString(hnode->GetID()));

  hnode->SetExpanded(0);
  logger.trace("onCollapsed: set hierarchy node to collapsed");

}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::onExpanded(const QModelIndex & index)
{
  Q_D(qSlicerModelsModuleWidget);
  logger.trace("onExpanded");
  
  vtkMRMLNode *node = d->ModelHierarchyTreeView->sortFilterProxyModel()->mrmlNodeFromIndex(index);
  if (!node)
    {
    logger.trace("onExpanded: node not found for this index");
    return;
    }
  vtkMRMLDisplayableHierarchyNode *hnode = vtkMRMLDisplayableHierarchyNode::SafeDownCast(node);
  if (!hnode)
    {
    logger.warn("onExpanded: node is not a displayable hierarchy node");
    return;
    }
  logger.trace(QString("onExpanded: found displayable hierarchy node ") + QString(hnode->GetID()));

  hnode->SetExpanded(1);
  logger.trace("onExpanded: set hierarchy node to expanded");
}
/*
//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerModelsModuleWidget);
  this->Superclass::setMRMLScene(scene);
}
*/
