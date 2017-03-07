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

#ifndef __qMRMLSortFilterSubjectHierarchyProxyModel_h
#define __qMRMLSortFilterSubjectHierarchyProxyModel_h

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

// Qt includes
#include <QSortFilterProxyModel>

// CTK includes
#include <ctkVTKObject.h>
#include <ctkPimpl.h>

class qMRMLSortFilterSubjectHierarchyProxyModelPrivate;
class vtkMRMLSubjectHierarchyNode;
class vtkMRMLScene;
class QStandardItem;

/// \ingroup Slicer_QtModules_SubjectHierarchy
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qMRMLSortFilterSubjectHierarchyProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
  QVTK_OBJECT

  /// Filter to show only items that contain the string in their names. Empty by default
  Q_PROPERTY(QString nameFilter READ nameFilter WRITE setNameFilter)
  /// Filter to show only items that contain an attribute with this name. Empty by default
  Q_PROPERTY(QString attributeNameFilter READ attributeNameFilter WRITE setAttributeNameFilter)
  /// Filter to show only items that contain an attribute with \sa attributeNameFilter (must be set)
  /// with this value. If empty, then existence of the attribute is enough to show
  /// Exact match is required. Empty by default
  Q_PROPERTY(QString attributeValueFilter READ attributeValueFilter WRITE setAttributeValueFilter)
  /// This property controls whether items unaffiliated with a given subject hierarchy item are hidden or not.
  /// All the nodes are visible (invalid item ID - vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID) by default
  Q_PROPERTY(vtkIdType hideItemsUnaffiliatedWithItemID READ hideItemsUnaffiliatedWithItemID WRITE setHideItemsUnaffiliatedWithItemID)
  /// Filter to show only items of a certain level (and their parents). If empty, then show all
  /// Exact match is required. Empty by default
  Q_PROPERTY(QString levelFilter READ levelFilter WRITE setLevelFilter)

public:
  typedef QSortFilterProxyModel Superclass;
  qMRMLSortFilterSubjectHierarchyProxyModel(QObject *parent=0);
  virtual ~qMRMLSortFilterSubjectHierarchyProxyModel();

  Q_INVOKABLE vtkMRMLSubjectHierarchyNode* subjectHierarchyNode()const;
  Q_INVOKABLE vtkMRMLScene* mrmlScene()const;

  QString nameFilter()const;
  void setNameFilter(QString filter);
  QString attributeNameFilter()const;
  void setAttributeNameFilter(QString filter);
  QString attributeValueFilter()const;
  void setAttributeValueFilter(QString filter);
  QString levelFilter()const;
  void setLevelFilter(QString filter);

  vtkIdType hideItemsUnaffiliatedWithItemID();
  void setHideItemsUnaffiliatedWithItemID(vtkIdType itemID);

  /// Retrieve the index of the MRML scene (the root item) in the subject hierarchy tree
  Q_INVOKABLE QModelIndex subjectHierarchySceneIndex()const;

  /// Retrieve the associated subject hierarchy item ID from a model index
  Q_INVOKABLE vtkIdType subjectHierarchyItemFromIndex(const QModelIndex& index)const;

  /// Retrieve an index for a given a subject hierarchy item ID
  Q_INVOKABLE QModelIndex indexFromSubjectHierarchyItem(vtkIdType itemID, int column=0)const;

  /// Determine the number of accepted (shown) items
  /// \param rootItemID Ancestor item of branch in which the accepted items are counted
  Q_INVOKABLE int acceptedItemCount(vtkIdType rootItemID)const;

protected:
  /// Returns true if the item in the row indicated by the given sourceRow and
  /// sourceParent should be included in the model; otherwise returns false.
  /// This method test each item via \a filterAcceptsItem
  virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent)const;

  /// Filters items to decide which to display in the view
  virtual bool filterAcceptsItem(vtkIdType itemID)const;

  QStandardItem* sourceItem(const QModelIndex& index)const;

protected:
  QScopedPointer<qMRMLSortFilterSubjectHierarchyProxyModelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSortFilterSubjectHierarchyProxyModel);
  Q_DISABLE_COPY(qMRMLSortFilterSubjectHierarchyProxyModel);
};

#endif
