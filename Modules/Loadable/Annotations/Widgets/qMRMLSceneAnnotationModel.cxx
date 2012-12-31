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
#include <QStringList>

// Annotations includes
#include "qMRMLSceneAnnotationModel_p.h"
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLAnnotationSnapshotNode.h"
#include "vtkSlicerAnnotationModuleLogic.h"

// MRML includes
#include <vtkMRMLScene.h>

//------------------------------------------------------------------------------
qMRMLSceneAnnotationModelPrivate
::qMRMLSceneAnnotationModelPrivate(qMRMLSceneAnnotationModel& object)
  : qMRMLSceneDisplayableModelPrivate(object)
{
  this->LockColumn = -1;
  this->EditColumn = -1;
  this->ValueColumn = -1;
  this->TextColumn = -1;
}

//------------------------------------------------------------------------------
void qMRMLSceneAnnotationModelPrivate::init()
{
  Q_Q(qMRMLSceneAnnotationModel);
  this->Superclass::init();

  q->setCheckableColumn(0);
  q->setVisibilityColumn(1);
  q->setLockColumn(2);
  q->setEditColumn(3);
  q->setValueColumn(4);
  q->setNameColumn(5);
  q->setTextColumn(6);

  q->setHorizontalHeaderLabels(
    QStringList() << "" << "Vis" << "Lock" << "Edit" << "Value" << "Name" << "Description");
}

//------------------------------------------------------------------------------
qMRMLSceneAnnotationModel::qMRMLSceneAnnotationModel(QObject *vparent)
  : Superclass(new qMRMLSceneAnnotationModelPrivate(*this), vparent)
{
  Q_D(qMRMLSceneAnnotationModel);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLSceneAnnotationModel::qMRMLSceneAnnotationModel(
  qMRMLSceneAnnotationModelPrivate* pimpl, QObject *vparent)
  : Superclass(pimpl, vparent)
{
  // init() is called by derived class.
}

//------------------------------------------------------------------------------
qMRMLSceneAnnotationModel::~qMRMLSceneAnnotationModel()
{
}

//------------------------------------------------------------------------------
void qMRMLSceneAnnotationModel::updateNodeFromItemData(vtkMRMLNode* node, QStandardItem* item)
{
  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  vtkMRMLAnnotationHierarchyNode* hierarchyNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(node);
  vtkMRMLAnnotationSnapshotNode* snapshotNode = vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);

  int oldChecked = node->GetSelected();

  this->qMRMLSceneDisplayableModel::updateNodeFromItemData(node, item);

  // TODO move to logic ?
  if (hierarchyNode && oldChecked != hierarchyNode->GetSelected())
    {
    int newChecked = hierarchyNode->GetSelected();
    vtkCollection* children = vtkCollection::New();
    hierarchyNode->GetChildrenDisplayableNodes(children);

    children->InitTraversal();
    for (int i=0; i<children->GetNumberOfItems(); ++i)
      {
      vtkMRMLAnnotationNode* childNode = vtkMRMLAnnotationNode::SafeDownCast(children->GetItemAsObject(i));
      if (childNode)
        {
        // this is a valid annotation child node
        // set all children to have same selected as the hierarchy
        childNode->SetSelected(newChecked);
        }
      } // for loop
    }// if hierarchyNode

  if (item->column() == this->textColumn())
    {
    if (annotationNode)
      {
      // if we have an annotation node, the text can be changed by editing the textcolumn
      annotationNode->SetText(0,item->text().toLatin1(),0,1);
      if (annotationNode->IsA("vtkMRMLAnnotationFiducialNode"))
        {
        // also set the name
        //annotationNode->SetName(item->text().toLatin1());
        }
      }
    else if (hierarchyNode)
      {
      // if we have a hierarchy node, the description can be changed by editing the textcolumn
      hierarchyNode->SetDescription(item->text().toLatin1());
      }
    else if (snapshotNode)
      {
      // if we have a snapshot node, the name can be changed by editing the textcolumn
      snapshotNode->SetName(item->text().toLatin1());
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneAnnotationModel::updateItemDataFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  Q_D(qMRMLSceneAnnotationModel);
  if (!node)
    {
    return;
    }
  this->Superclass::updateItemDataFromNode(item, node, column);
  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  vtkMRMLAnnotationHierarchyNode *hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(node);
  if (column == this->visibilityColumn())
    {
    // the visibility icon
    if (annotationNode)
      {
      if (annotationNode->GetDisplayVisibility())
        {
        item->setData(QPixmap(":/Icons/Small/SlicerVisible.png"),Qt::DecorationRole);
        }
      else
        {
        item->setData(QPixmap(":/Icons/Small/SlicerInvisible.png"),Qt::DecorationRole);
        }
      }
    else if (hnode)
      {
      // don't show anything, handle it in property dialogue
      }
    }
  if (column == this->lockColumn())
    {
    // the lock/unlock icon
    if (annotationNode)
      {
      if (annotationNode->GetLocked())
        {
        item->setData(QPixmap(":/Icons/Small/SlicerLock.png"),Qt::DecorationRole);
        }
      else
        {
        item->setData(QPixmap(":/Icons/Small/SlicerUnlock.png"),Qt::DecorationRole);
        }
      }
    else if (hnode)
      {
      // don't show anything, handle it in property dialogue
      }
    }
  if (column == this->editColumn())
    {
    // the annotation type icon
    item->setData(
      QPixmap(d->AnnotationLogic->GetAnnotationIcon(node)), Qt::DecorationRole);
    }
  if (column == this->valueColumn())
    {
    if (annotationNode)
      {
      // the annotation measurement
      item->setText(
        QString(d->AnnotationLogic->GetAnnotationMeasurement(
                  annotationNode->GetID(),false)));
      }
    else if (hnode)
      {
      item->setText(QString(""));
      }
    }
  if (column == this->textColumn())
    {
    if (annotationNode)
      {
      // the annotation text
      item->setText(
        QString(d->AnnotationLogic->GetAnnotationText(
                  annotationNode->GetID())));
      }
    else if (hnode)
      {
      item->setText(QString(node->GetDescription()));
      }
    }
}

//------------------------------------------------------------------------------
QFlags<Qt::ItemFlag> qMRMLSceneAnnotationModel::nodeFlags(vtkMRMLNode* node, int column)const
{
  QFlags<Qt::ItemFlag> flags = this->qMRMLSceneDisplayableModel::nodeFlags(node, column);
  // remove the ItemIsEditable flag from any possible item (typically at column 0)
  flags = flags & ~Qt::ItemIsEditable;
  // and set it to the text and names columns
  if (column == this->nameColumn() ||
      column == this->textColumn())
    {
    flags = flags | Qt::ItemIsEditable;
    }
  // if this is an annotation with a hierarchy node that it's a 1:1 node, don't allow
  // dropping
  vtkMRMLDisplayableHierarchyNode *displayableHierarchyNode =
    vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(
      node->GetScene(), node->GetID());
  if (displayableHierarchyNode  &&
      !displayableHierarchyNode->GetAllowMultipleChildren())
    {
    flags = flags & ~Qt::ItemIsDropEnabled;
    }
  return flags;
}

//------------------------------------------------------------------------------
bool qMRMLSceneAnnotationModel::canBeAParent(vtkMRMLNode* node)const
{
  bool res = this->Superclass::canBeAParent(node) ||
    (node && node->IsA("vtkMRMLAnnotationNode"));
  return res;
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLSceneAnnotationModel::parentNode(vtkMRMLNode* node)const
{
  if (node == NULL)
    {
    return 0;
    }

  // MRML Displayable nodes (inherits from transformable)
  vtkMRMLDisplayableNode *displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
  vtkMRMLDisplayableHierarchyNode * displayableHierarchyNode = NULL;
  if (displayableNode &&
      displayableNode->GetScene() &&
      displayableNode->GetID())
    {
    // get the displayable hierarchy node associated with this displayable node
    displayableHierarchyNode = vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(
      displayableNode->GetScene(), displayableNode->GetID());

    if (displayableHierarchyNode)
      {
      if (displayableHierarchyNode->GetHideFromEditors())
        {
        // this is a hidden hierarchy node, so we do not want to display it
        // instead, we will return the parent of the hidden hierarchy node
        // to be used as the parent for the displayableNode
        vtkMRMLDisplayableHierarchyNode* parent =
          vtkMRMLDisplayableHierarchyNode::SafeDownCast(
            displayableHierarchyNode->GetParentNode());
        if (parent && parent->GetHideFromEditors() &&
            parent->GetDisplayableNode())
          {
          return parent->GetDisplayableNode();
          }
        return parent;
        }
      return displayableHierarchyNode;
      }
    }
  if (displayableHierarchyNode == NULL)
    {
    // the passed in node might have been a hierarchy node instead, try to
    // cast it
    displayableHierarchyNode = vtkMRMLDisplayableHierarchyNode::SafeDownCast(node);
    }
  if (displayableHierarchyNode)
    {
    // return it's parent
    return displayableHierarchyNode->GetParentNode();
    }
  return 0;
}

//------------------------------------------------------------------------------
int qMRMLSceneAnnotationModel::lockColumn()const
{
  Q_D(const qMRMLSceneAnnotationModel);
  return d->LockColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneAnnotationModel::setLockColumn(int column)
{
  Q_D(qMRMLSceneAnnotationModel);
  d->LockColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSceneAnnotationModel::editColumn()const
{
  Q_D(const qMRMLSceneAnnotationModel);
  return d->EditColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneAnnotationModel::setEditColumn(int column)
{
  Q_D(qMRMLSceneAnnotationModel);
  d->EditColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSceneAnnotationModel::valueColumn()const
{
  Q_D(const qMRMLSceneAnnotationModel);
  return d->ValueColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneAnnotationModel::setValueColumn(int column)
{
  Q_D(qMRMLSceneAnnotationModel);
  d->ValueColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSceneAnnotationModel::textColumn()const
{
  Q_D(const qMRMLSceneAnnotationModel);
  return d->TextColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneAnnotationModel::setTextColumn(int column)
{
  Q_D(qMRMLSceneAnnotationModel);
  d->TextColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSceneAnnotationModel::maxColumnId()const
{
  Q_D(const qMRMLSceneAnnotationModel);
  int maxId = this->Superclass::maxColumnId();
  maxId = qMax(maxId, d->LockColumn);
  maxId = qMax(maxId, d->EditColumn);
  maxId = qMax(maxId, d->ValueColumn);
  maxId = qMax(maxId, d->TextColumn);
  return maxId;
}

//-----------------------------------------------------------------------------
/// Set and observe the logic
//-----------------------------------------------------------------------------
void qMRMLSceneAnnotationModel::setLogic(vtkSlicerAnnotationModuleLogic* logic)
{
  Q_D(qMRMLSceneAnnotationModel);
  if (!logic)
    {
    return;
    }

  d->AnnotationLogic = logic;
}
