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
}

//------------------------------------------------------------------------------
qMRMLSceneAnnotationModel::~qMRMLSceneAnnotationModel()
{
}
/*
//------------------------------------------------------------------------------
void qMRMLSceneAnnotationModel::updateScene()
{

  // save extra items
  QStringList oldPreItems = this->preItems(0);
  QStringList oldPostItems = this->postItems(0);

  QStringList oldScenePreItems, oldScenePostItems;
  QList<QStandardItem*> oldSceneItem = this->findItems("Scene");
  if (oldSceneItem.size())
    {
    oldScenePreItems = this->preItems(oldSceneItem[0]);
    oldScenePostItems = this->postItems(oldSceneItem[0]);
    }

  int oldColumnCount = this->columnCount();
  this->clear();
  this->invisibleRootItem()->setFlags(Qt::ItemIsEnabled);
  this->setColumnCount(oldColumnCount);

  // restore extra items
  this->setPreItems(oldPreItems, 0);
  this->setPostItems(oldPostItems, 0);
  if (this->mrmlScene == 0)
    {
    return;
    }

  // Add scene item
  QList<QStandardItem*> sceneItems;
  QStandardItem* sceneItem = new QStandardItem;
  sceneItem->setFlags(Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
  sceneItem->setText("Scene");
  sceneItem->setData("scene", qMRML::UIDRole);
  sceneItem->setData(QVariant::fromValue(reinterpret_cast<long long>(d->MRMLScene)), qMRML::PointerRole);
  sceneItems << sceneItem;
  sceneItems << new QStandardItem;
  sceneItems[1]->setFlags(0);
  this->insertRow(oldPreItems.count(), sceneItems);
  this->setPreItems(oldScenePreItems, sceneItem);
  this->setPostItems(oldScenePostItems, sceneItem);

  // Populate scene with nodes
  this->populateScene();
}

//------------------------------------------------------------------------------
void qMRMLSceneAnnotationModel::populateScene()
{
  // Add nodes
  vtkMRMLNode *node = 0;
  vtkCollectionSimpleIterator it;
  int row = 0;
  for (this->mrmlScene->GetCurrentScene()->InitTraversal(it);
       (node = (vtkMRMLNode*)this->mrmlScene->GetCurrentScene()->GetNextItemAsObject(it)) ;)
    {
    if (!node->IsA("vtkMRMLAnnotationNode") && !node->IsA("vtkMRMLAnnotationHierarchyNode"))
      {
      break;
      }

    this->insertNode(node, this->invisibleRootItem(), row++);
    }
}
*/
//------------------------------------------------------------------------------
/*vtkMRMLNode* qMRMLSceneAnnotationModel::parentNode(vtkMRMLNode* node)
{
  if (node == NULL)
    {
    return 0;
    }

  // MRML Displayable nodes (inherits from transformable)
  vtkMRMLAnnotationNode *displayableNode = vtkMRMLAnnotationNode::SafeDownCast(node);

  if (!displayableNode->IsA("vtkMRMLAnnotationLinesNode"))
    {
    return 0;
    }
  if (!displayableNode->IsA("vtkMRMLAnnotationControlPointsNode"))
    {
    return 0;
    }
  if (!displayableNode->IsA("vtkMRMLAnnotationTextNode"))
    {
    return 0;
    }


  vtkMRMLDisplayableHierarchyNode * displayableHierarchyNode = NULL;
  if (displayableNode &&
      displayableNode->GetScene() &&
      displayableNode->GetID())
    {
    // get the displayable hierarchy node associated with this displayable node
    displayableHierarchyNode = vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(displayableNode->GetScene(), displayableNode->GetID());
    if (displayableHierarchyNode)
      {
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
*/
