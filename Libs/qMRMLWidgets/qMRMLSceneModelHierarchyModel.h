/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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

#ifndef __qMRMLSceneModelHierarchyModel_h
#define __qMRMLSceneModelHierarchyModel_h

#include "qMRMLSceneModel.h"

class qMRMLSceneModelHierarchyModelPrivate;

class QMRML_WIDGETS_EXPORT qMRMLSceneModelHierarchyModel : public qMRMLSceneModel
{
  Q_OBJECT
  ///
  /// Control in which column vtkMRMLModelDisplayNode::Color are displayed
  /// (Qt::DecorationRole). Even if a vtkMRMLModelNode doesn't have a color
  /// proper, the color of its display node is used. If the model node has
  /// more than one display node and their colors are different, it uses
  /// an invalid color.
  /// A value of -1 hides it. Hidden by default (value of -1).
  Q_PROPERTY (int colorColumn READ colorColumn WRITE setColorColumn)
  Q_PROPERTY (int opacityColumn READ opacityColumn WRITE setOpacityColumn)

public:
  typedef qMRMLSceneModel Superclass;
  qMRMLSceneModelHierarchyModel(QObject *parent=0);
  virtual ~qMRMLSceneModelHierarchyModel();

  int colorColumn()const;
  void setColorColumn(int column);

  int opacityColumn()const;
  void setOpacityColumn(int column);

  ///
  virtual vtkMRMLNode* parentNode(vtkMRMLNode* node)const;

  virtual int          nodeIndex(vtkMRMLNode* node)const;

  /// Fast function that only check the type of the node to know if it can be a child.
  virtual bool         canBeAChild(vtkMRMLNode* node)const;
  
  /// Fast function that only check the type of the node to know if it can be a parent.
  virtual bool         canBeAParent(vtkMRMLNode* node)const;

  /// If newParent == 0, set the node into the vtkMRMLScene
  virtual bool         reparent(vtkMRMLNode* node, vtkMRMLNode* newParent);

  virtual Qt::DropActions supportedDropActions()const;
  virtual void setMRMLScene(vtkMRMLScene* scene);

  /// 
  /// As we reimplement insertNode, we need don't want to hide the other functions.
  using qMRMLSceneModel::insertNode;
  /// Reimplemented to listen to the displayable DisplayModifiedEvent event for
  /// visibility check state changes.
  virtual QStandardItem* insertNode(vtkMRMLNode* node, QStandardItem* parent, int row);

protected:
  QFlags<Qt::ItemFlag> nodeFlags(vtkMRMLNode* node, int column)const;
  virtual void updateItemDataFromNode(QStandardItem* item, vtkMRMLNode* node, int column);
  virtual void updateNodeFromItemData(vtkMRMLNode* node, QStandardItem* item);

private:
  Q_DECLARE_PRIVATE(qMRMLSceneModelHierarchyModel);
  Q_DISABLE_COPY(qMRMLSceneModelHierarchyModel);
};

#endif
