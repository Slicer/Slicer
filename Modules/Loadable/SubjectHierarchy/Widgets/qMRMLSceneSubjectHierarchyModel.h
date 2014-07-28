/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qMRMLSceneSubjectHierarchyModel_h
#define __qMRMLSceneSubjectHierarchyModel_h

// SubjectHierarchy Widgets includes
#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

// MRMLWidgets includes
#include "qMRMLSceneHierarchyModel.h"

class qMRMLSceneSubjectHierarchyModelPrivate;

/// \ingroup Slicer_QtModules_SubjectHierarchy
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qMRMLSceneSubjectHierarchyModel : public qMRMLSceneHierarchyModel
{
  Q_OBJECT

  /// Control in which column vtkMRMLNode::NodeType are displayed (Qt::DecorationRole).
  /// A value of -1 hides it. Hidden by default (value of -1).
  Q_PROPERTY (int nodeTypeColumn READ nodeTypeColumn WRITE setNodeTypeColumn)

  /// Control in which column the parent transforms are displayed
  /// A MRML node combobox is displayed in the row of the transformable nodes, in which
  /// the current transform is selected. The user can change the transform using the combobox.
  /// A value of -1 (default) hides the column
  Q_PROPERTY (int transformColumn READ transformColumn WRITE setTransformColumn)

public:
  typedef qMRMLSceneHierarchyModel Superclass;
  qMRMLSceneSubjectHierarchyModel(QObject *parent=0);
  virtual ~qMRMLSceneSubjectHierarchyModel();

  enum ItemDataRole
  {
    /// MRML node ID of the parent transform
    TransformIDRole = qMRMLSceneModel::LastRole + 1,
    /// Must stay the last enum in the list.
    LastRole
  };

  /// Define drop actions
  virtual Qt::DropActions supportedDropActions()const;

  /// Get parent node
  virtual vtkMRMLNode* parentNode(vtkMRMLNode* node)const;

  /// Provide node index for node
  virtual int nodeIndex(vtkMRMLNode* node)const;

  /// Fast function that only check the type of the node to know if it can be a child.
  virtual bool canBeAChild(vtkMRMLNode* node)const;

  /// Fast function that only check the type of the node to know if it can be a parent.
  virtual bool canBeAParent(vtkMRMLNode* node)const;

  /// Insert/move node in subject hierarchy under newParent
  Q_INVOKABLE virtual bool reparent(vtkMRMLNode* node, vtkMRMLNode* newParent);

  int nodeTypeColumn()const;
  void setNodeTypeColumn(int column);

  int transformColumn()const;
  void setTransformColumn(int column);

public slots:
  /// Remove transforms from nodes in branch of current node
  void onRemoveTransformsFromBranchOfCurrentNode();

  /// Harden transform on branch of current node
  void onHardenTransformOnBranchOfCurrentNode();

signals:
  void saveTreeExpandState();
  void loadTreeExpandState();
  void invalidateModels();

protected:
  /// Get the largest column ID
  virtual int maxColumnId()const;

  /// Overridden function to return flags for custom columns
  virtual QFlags<Qt::ItemFlag> nodeFlags(vtkMRMLNode* node, int column)const;

  /// Overridden function to handle tree view item display from node data
  virtual void updateItemDataFromNode(QStandardItem* item, vtkMRMLNode* node, int column);

  /// Overridden function to handle node update from tree view item
  virtual void updateNodeFromItemData(vtkMRMLNode* node, QStandardItem* item);

private:
  Q_DECLARE_PRIVATE(qMRMLSceneSubjectHierarchyModel);
  Q_DISABLE_COPY(qMRMLSceneSubjectHierarchyModel);
};

#endif
