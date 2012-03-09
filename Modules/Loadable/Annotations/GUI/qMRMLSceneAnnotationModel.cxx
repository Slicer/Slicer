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
#include <QMap>
#include <QMimeData>
#include <QSharedPointer>
#include <QStack>
#include <QStringList>
#include <QVector>

// Annotations includes
#include "qMRMLSceneAnnotationModel.h"
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLAnnotationSnapshotNode.h"
#include "vtkSlicerAnnotationModuleLogic.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLAnnotationNode.h>
#include <vtkMRMLDisplayableHierarchyNode.h>

// VTK includes
#include <vtkVariantArray.h>
#include <typeinfo>

//------------------------------------------------------------------------------
qMRMLSceneAnnotationModel::qMRMLSceneAnnotationModel(QObject *vparent)
  :qMRMLSceneDisplayableModel(vparent)
{
  this->setListenNodeModifiedEvent(true);
  this->setNameColumn(5);
  this->setVisibilityColumn(-1);
  this->setOpacityColumn(-1);
  this->setCheckableColumn(qMRMLSceneAnnotationModel::CheckedColumn);

  this->setColumnCount(7);
  this->setHorizontalHeaderLabels(
    QStringList() << "" << "Vis" << "Lock" << "Edit" << "Value" << "Name" << "Description");
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

  switch (item->column())
    {
    case qMRMLSceneAnnotationModel::TextColumn:
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
      break;
      }
    default:
      break;
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneAnnotationModel::updateItemDataFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  if (!node)
    {
    return;
    }
  this->qMRMLSceneDisplayableModel::updateItemDataFromNode(item, node, column);
  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  vtkMRMLAnnotationHierarchyNode *hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(node);
  switch (column)
    {
    case qMRMLSceneAnnotationModel::VisibilityColumn:
      // the visibility icon
      if (annotationNode)
        {
        if (annotationNode->GetVisible())
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
      break;
    case qMRMLSceneAnnotationModel::LockColumn:
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
      break;
    case qMRMLSceneAnnotationModel::EditColumn:
        // the annotation type icon
        item->setData(QPixmap(this->m_Logic->GetAnnotationIcon(node->GetID())),Qt::DecorationRole);
        break;
    case qMRMLSceneAnnotationModel::ValueColumn:
      if (annotationNode)
        {
        // the annotation measurement
        item->setText(QString(this->m_Logic->GetAnnotationMeasurement(annotationNode->GetID(),false)));
        }
      else if (hnode)
        {
        item->setText(QString(""));
        }
      break;
    case qMRMLSceneAnnotationModel::TextColumn:
      if (annotationNode)
        {
        // the annotation text
        item->setText(QString(this->m_Logic->GetAnnotationText(annotationNode->GetID())));
        }
      else if (hnode)
        {
        item->setText(QString(node->GetDescription()));
        }
      break;
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
      column == qMRMLSceneAnnotationModel::TextColumn)
    {
    flags = flags | Qt::ItemIsEditable;
    }
  return flags;
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
    displayableHierarchyNode = vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(displayableNode->GetScene(), displayableNode->GetID());

    if (displayableHierarchyNode)
      {
      if (displayableHierarchyNode->GetHideFromEditors())
        {
        // this is a hidden hierarchy node, so we do not want to display it
        // instead, we will return the parent of the hidden hierarchy node
        // to be used as the parent for the displayableNode
        return displayableHierarchyNode->GetParentNode();
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


//-----------------------------------------------------------------------------
/// Set and observe the logic
//-----------------------------------------------------------------------------
void qMRMLSceneAnnotationModel::setLogic(vtkSlicerAnnotationModuleLogic* logic)
{
  if (!logic)
    {
    return;
    }

  this->m_Logic = logic;

}
