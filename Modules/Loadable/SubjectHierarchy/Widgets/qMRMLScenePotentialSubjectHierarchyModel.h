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

#ifndef __qMRMLScenePotentialSubjectHierarchyModel_h
#define __qMRMLScenePotentialSubjectHierarchyModel_h

// SubjectHierarchy Widgets includes
#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

// MRMLWidgets includes
#include "qMRMLSceneModel.h"

class qMRMLScenePotentialSubjectHierarchyModelPrivate;

/// \ingroup Slicer_QtModules_SubjectHierarchy
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qMRMLScenePotentialSubjectHierarchyModel : public qMRMLSceneModel
{
  Q_OBJECT

public:
  typedef qMRMLSceneModel Superclass;
  qMRMLScenePotentialSubjectHierarchyModel(QObject *parent=0);
  virtual ~qMRMLScenePotentialSubjectHierarchyModel();

  /// Overridden function telling the view the supported drop actions
  virtual Qt::DropActions supportedDropActions()const;

  /// Function returning the supported MIME types
  virtual QStringList mimeTypes()const;

  /// Function encoding the dragged item to MIME data
  virtual QMimeData* mimeData(const QModelIndexList &indexes)const;

  /// Check the type of the node to know if it can be a child.
  virtual bool canBeAChild(vtkMRMLNode* node)const;

protected:
  /// Overridden function to handle tree view item display from node data
  virtual void updateItemDataFromNode(QStandardItem* item, vtkMRMLNode* node, int column);

  /// Overridden function to handle node update from tree view item
  virtual void updateNodeFromItemData(vtkMRMLNode* node, QStandardItem* item);

protected slots:
  /// Update model when a node is drag&dropped outside
  void onRowsRemoved(const QModelIndex parent, int start, int end);

private:
  Q_DECLARE_PRIVATE(qMRMLScenePotentialSubjectHierarchyModel);
  Q_DISABLE_COPY(qMRMLScenePotentialSubjectHierarchyModel);
};

#endif
