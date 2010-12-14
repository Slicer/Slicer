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

// qMRML includes
#include "qMRMLSceneAnnotationModel.h"
#include "qMRMLSceneDisplayableModel.h"
//#include "qMRMLUtils.h"

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
  this->setColumnCount(6);
  this->setHorizontalHeaderLabels(
    QStringList() << "" << "Vis" << "Lock" << "Edit" << "Value" << "Text");
}

//------------------------------------------------------------------------------
qMRMLSceneAnnotationModel::~qMRMLSceneAnnotationModel()
{
}

//------------------------------------------------------------------------------
void qMRMLSceneAnnotationModel::updateNodeFromItemData(vtkMRMLNode* node, QStandardItem* item)
{
  if (item->column() == qMRMLSceneAnnotationModel::TextColumn)
    {
    vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
    vtkMRMLAnnotationHierarchyNode* hierarchyNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(node);
    vtkMRMLAnnotationSnapshotNode* snapshotNode = vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);

    if (annotationNode)
      {
      // if we have an annotation node, the text can be changed by editing the textcolumn
      annotationNode->SetText(0,item->text().toLatin1(),0,1);
      }
    else if (hierarchyNode)
      {
      // if we have a hierarchy node, the name can be changed by editing the textcolumn
      hierarchyNode->SetName(item->text().toLatin1());
      }
    else if (snapshotNode)
      {
      // if we have a snapshot node, the name can be changed by editing the textcolumn
      snapshotNode->SetName(item->text().toLatin1());
      }
    }
  //this->m_Widget->refreshTree();
}

//------------------------------------------------------------------------------
void qMRMLSceneAnnotationModel::updateItemDataFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);

  switch (column)
    {
    case qMRMLSceneAnnotationModel::VisibilityColumn:
      // the visibility icon

      if (annotationNode)
        {
        if (annotationNode->GetVisible())
          {
          item->setData(QPixmap(":/Icons/AnnotationVisibility.png"),Qt::DecorationRole);
          }
        else
          {
          item->setData(QPixmap(":/Icons/AnnotationInvisible.png"),Qt::DecorationRole);
          }
        break;
        }
      if (node->IsA("vtkMRMLAnnotationHierarchyNode"))
        {
        item->setData(QPixmap(":/Icons/AnnotationVisibility.png"),Qt::DecorationRole);
        }
      break;
    case qMRMLSceneAnnotationModel::LockColumn:
      // the lock/unlock icon

      if (annotationNode)
        {
        if (annotationNode->GetLocked())
          {
          item->setData(QPixmap(":/Icons/AnnotationLock.png"),Qt::DecorationRole);
          }
        else
          {
          item->setData(QPixmap(":/Icons/AnnotationUnlock.png"),Qt::DecorationRole);
          }
        break;
        }
      if (node->IsA("vtkMRMLAnnotationHierarchyNode"))
        {
        item->setData(QPixmap(":/Icons/AnnotationUnlock.png"),Qt::DecorationRole);
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
        break;
        }
      else if (node->IsA("vtkMRMLAnnotationHierarchyNode"))
        {
        item->setText(QString(""));
        }
      break;
    case qMRMLSceneAnnotationModel::TextColumn:
      if (annotationNode)
        {
        // the annotation text
        item->setText(QString(this->m_Logic->GetAnnotationText(annotationNode->GetID())));
        break;
        }
      else if (node->IsA("vtkMRMLAnnotationHierarchyNode"))
        {
        item->setText(QString(node->GetName()));
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
  // and set it to the right column
  switch(column)
    {
    case qMRMLSceneAnnotationModel::TextColumn:
      flags = flags | Qt::ItemIsEditable;
      break;
    default:
      break;
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
/// Set and observe the GUI widget
//-----------------------------------------------------------------------------
void qMRMLSceneAnnotationModel::setAndObserveWidget(qSlicerAnnotationModuleWidget* widget)
{
  if (!widget)
    {
    return;
    }

  this->m_Widget = widget;

}

//-----------------------------------------------------------------------------
/// Set and observe the logic
//-----------------------------------------------------------------------------
void qMRMLSceneAnnotationModel::setAndObserveLogic(vtkSlicerAnnotationModuleLogic* logic)
{
  if (!logic)
    {
    return;
    }

  this->m_Logic = logic;

}
