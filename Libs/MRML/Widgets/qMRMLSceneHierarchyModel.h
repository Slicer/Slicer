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

#ifndef __qMRMLSceneHierarchyModel_h
#define __qMRMLSceneHierarchyModel_h

// MRMLWidgets includes
#include "qMRMLSceneModel.h"
class qMRMLSceneHierarchyModelPrivate;

class QMRML_WIDGETS_EXPORT qMRMLSceneHierarchyModel : public qMRMLSceneModel
{
  Q_OBJECT
  /// Control in which column vtkMRMLHierarchyNode is displayed (Qt::CheckStateRole)
  /// A value of -1 hides it. Hidden (-1) by default.
  /// Note that the expand column should be different than checkableColumn if
  /// not both set to -1.
  /// The Expanded property is on vtkMRMLDisplayableHierarchyNode.
  /// \todo Move the Expanded property to vtkMRMLHierarchyNode.
  /// \sa nameColumn, idColumn, checkableColumn, visibilityColumn...
  Q_PROPERTY (int expandColumn READ expandColumn WRITE setExpandColumn)

public:
  typedef qMRMLSceneModel Superclass;
  qMRMLSceneHierarchyModel(QObject *parent=0);
  virtual ~qMRMLSceneHierarchyModel();

  int expandColumn()const;
  void setExpandColumn(int column);

  virtual Qt::DropActions supportedDropActions()const;

  ///
  virtual vtkMRMLNode* parentNode(vtkMRMLNode* node)const;
  virtual int          nodeIndex(vtkMRMLNode* node)const;
  /// fast function that only check the type of the node to know if it can be a child.
  virtual bool         canBeAChild(vtkMRMLNode* node)const;
  /// fast function that only check the type of the node to know if it can be a parent.
  virtual bool         canBeAParent(vtkMRMLNode* node)const;
  /// if newParent == 0, set the node into the vtkMRMLScene
  virtual bool         reparent(vtkMRMLNode* node, vtkMRMLNode* newParent);

protected:
  qMRMLSceneHierarchyModel(qMRMLSceneHierarchyModelPrivate* pimpl,
                           QObject *parent=0);
  virtual QFlags<Qt::ItemFlag> nodeFlags(vtkMRMLNode* node, int column)const;

  virtual void observeNode(vtkMRMLNode* node);

  /// Reimplemented to add expandColumn support
  void updateItemDataFromNode(QStandardItem* item, vtkMRMLNode* node, int column);

  /// Reimplemented to add expandColumn support
  void updateNodeFromItemData(vtkMRMLNode* node, QStandardItem* item);

  /// Must be reimplemented in subclasses that add new column types
  virtual int maxColumnId()const;

private:
  Q_DECLARE_PRIVATE(qMRMLSceneHierarchyModel);
  Q_DISABLE_COPY(qMRMLSceneHierarchyModel);
};

#endif
