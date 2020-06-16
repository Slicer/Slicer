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

#ifndef __qMRMLSubjectHierarchyModel_p_h
#define __qMRMLSubjectHierarchyModel_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Qt includes
#include <QFlags>
#include <QMap>

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

#include "qMRMLSubjectHierarchyModel.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSubjectHierarchyNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>

class QStandardItemModel;
class vtkSlicerTerminologiesModuleLogic;

//------------------------------------------------------------------------------
// qMRMLSubjectHierarchyModelPrivate
//------------------------------------------------------------------------------
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qMRMLSubjectHierarchyModelPrivate
{
  Q_DECLARE_PUBLIC(qMRMLSubjectHierarchyModel);

protected:
  qMRMLSubjectHierarchyModel* const q_ptr;
public:
  qMRMLSubjectHierarchyModelPrivate(qMRMLSubjectHierarchyModel& object);
  virtual ~qMRMLSubjectHierarchyModelPrivate();
  void init();

  /// This method is called by qMRMLSubjectHierarchyModel::updateFromSubjectHierarchy() to speed up
  /// the loading. By explicitly specifying the \a index, it skips item lookup within their parents
  /// happening in qMRMLSubjectHierarchyModel::subjectHierarchyItemIndex(vtkIdType).
  virtual QStandardItem* insertSubjectHierarchyItem(vtkIdType itemID, int index);

  /// Convenience function to get name for subject hierarchy item
  QString subjectHierarchyItemName(vtkIdType itemID);

  /// Get terminologies module logic. If not found in cache get from module object
  vtkSlicerTerminologiesModuleLogic* terminologiesModuleLogic();

public:
  vtkSmartPointer<vtkCallbackCommand> CallBack;
  int PendingItemModified;

  int NameColumn;
  int IDColumn;
  int VisibilityColumn;
  int ColorColumn;
  int TransformColumn;
  int DescriptionColumn;

  QIcon VisibleIcon;
  QIcon HiddenIcon;
  QIcon PartiallyVisibleIcon;

  QIcon UnknownIcon;
  QIcon WarningIcon;

  QIcon NoTransformIcon;
  QIcon FolderTransformIcon;
  QIcon LinearTransformIcon;
  QIcon DeformableTransformIcon;

  /// Subject hierarchy node
  vtkWeakPointer<vtkMRMLSubjectHierarchyNode> SubjectHierarchyNode;
  /// MRML scene (to get new subject hierarchy node if the stored one is deleted)
  vtkWeakPointer<vtkMRMLScene> MRMLScene;
  /// Terminology module logic. Needed to generate the terminology tooltip in the color column
  vtkSlicerTerminologiesModuleLogic* TerminologiesModuleLogic;

  mutable QList<vtkIdType> DraggedSubjectHierarchyItems;
  bool DelayedItemChangedInvoked;

  // Keep a list of QStandardItem instead of subject hierarchy item because they are likely to be
  // unreachable when browsing the model
  QList<QList<QStandardItem*> > Orphans;

  // Map from subject hierarchy item to row.
  // It just stores the result of the latest lookup by \sa indexFromSubjectHierarchyItem,
  // not guaranteed to contain up-to-date information, should be just used as a search hint.
  // If the item cannot be found at the given index then we need to browse through all model items.
  mutable QMap<vtkIdType, QPersistentModelIndex> RowCache;
};

#endif
