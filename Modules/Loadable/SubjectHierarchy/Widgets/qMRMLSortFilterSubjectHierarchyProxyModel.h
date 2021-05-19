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
  /// This property controls whether items unaffiliated with a given subject hierarchy item are hidden or not.
  /// All the nodes are visible (invalid item ID - vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID) by default
  Q_PROPERTY(vtkIdType hideItemsUnaffiliatedWithItemID READ hideItemsUnaffiliatedWithItemID WRITE setHideItemsUnaffiliatedWithItemID)
  /// Show hierarchy items (items without data node such as patient/study/folder) that have no children to show
  /// according to the filters.
  /// This is most useful to in off state keep combobox content reasonable size for selecting data nodes.
  /// True by default.
  Q_PROPERTY(bool showEmptyHierarchyItems READ showEmptyHierarchyItems WRITE setShowEmptyHierarchyItems)
  /// Filter to show only items of a certain level (and their parents). If empty, then show all
  Q_PROPERTY(QStringList levelFilter READ levelFilter WRITE setLevelFilter)

  /// This property controls which items are visible. The MRML node class name must be provided.
  /// An empty list means all the nodes are visible (default).
  Q_PROPERTY(QStringList nodeTypes READ nodeTypes WRITE setNodeTypes)
  /// This property controls the items to hide by MRML node type
  /// Any node of type \a nodeType are visible except the ones
  /// also of type \a hideChildNodeTypes.
  /// e.g.: nodeTypes = vtkMRMLVolumeNode, showChildNodeTypes = true,
  /// hideChildNodeTypes = vtkMRMLDiffusionWeightedVolumeNode
  /// -> all the nodes of type vtkMRMLScalarVolumeNode, vtkMRMLTensorVolumeNode,
  /// vtkMRMLDiffusionImageVolumeNode... (but not vtkMRMLDiffusionWeightedVolumeNode)
  /// will be visible.
  Q_PROPERTY(QStringList hideChildNodeTypes READ hideChildNodeTypes WRITE setHideChildNodeTypes)

  /// Filter to show only items that contain any of the given attributes with this name. Empty by default.
  /// When setting it, all the include filters are overwritten.
  Q_PROPERTY(QStringList includeItemAttributeNamesFilter READ includeItemAttributeNamesFilter WRITE setIncludeItemAttributeNamesFilter)
  /// Filter to show only items for data nodes that contain any of the given attributes with this name. Empty by default.
  /// When setting it, all the include filters are overwritten.
  Q_PROPERTY(QStringList includeNodeAttributeNamesFilter READ includeNodeAttributeNamesFilter WRITE setIncludeNodeAttributeNamesFilter)
  /// Filter to hide items that contain any of the given attributes with this name. Empty by default.
  /// When setting it, all the include filters are overwritten.
  /// Overrides \sa includeItemAttributeNamesFilter
  Q_PROPERTY(QStringList excludeItemAttributeNamesFilter READ excludeItemAttributeNamesFilter WRITE setExcludeItemAttributeNamesFilter)
  /// Filter to hide items for data nodes that contain any of the given attributes with this name. Empty by default.
  /// When setting it, all the include filters are overwritten.
  /// Overrides \sa includeNodeAttributeNamesFilter
  Q_PROPERTY(QStringList excludeNodeAttributeNamesFilter READ excludeNodeAttributeNamesFilter WRITE setExcludeNodeAttributeNamesFilter)

  /// Filter to show only items that contain an attribute with this name. Empty by default
  /// Note: Deprecated, kept only for backwards compatibility. Sets and returns the first attribute in \sa includeNodeAttributeNamesFilter
  Q_PROPERTY(QString attributeNameFilter READ attributeNameFilter WRITE setAttributeNameFilter)
  /// Filter to show only items that contain any attribute given in \sa includeItemAttributeNamesFilter with the value.
  /// If empty, then existence of the attributes is enough to show.
  /// Exact match is required. Empty by default
  /// Note: Deprecated, kept only for backwards compatibility. Works consistently with the previous operation.
  Q_PROPERTY(QString attributeValueFilter READ attributeValueFilter WRITE setAttributeValueFilter)

public:
  typedef QSortFilterProxyModel Superclass;
  qMRMLSortFilterSubjectHierarchyProxyModel(QObject *parent=nullptr);
  ~qMRMLSortFilterSubjectHierarchyProxyModel() override;

  Q_INVOKABLE vtkMRMLSubjectHierarchyNode* subjectHierarchyNode()const;
  Q_INVOKABLE vtkMRMLScene* mrmlScene()const;

  QString nameFilter()const;
  QStringList levelFilter()const;
  QStringList nodeTypes()const;
  QStringList hideChildNodeTypes()const;
  QStringList includeItemAttributeNamesFilter()const;
  QStringList includeNodeAttributeNamesFilter()const;
  QStringList excludeItemAttributeNamesFilter()const;
  QStringList excludeNodeAttributeNamesFilter()const;
  QString attributeValueFilter()const;
  QString attributeNameFilter()const;
  /// Add single item attribute filter specifying attribute name, value, include/exclude, and class name
  /// \param attributeName Name of the item attribute to filter
  /// \param attributeValue Value of the item attribute to filter
  /// \param include Flag indicating whether this is an include filter or exclude filter.
  ///   - Include filter means that only the items are shown that match the filter.
  ///   - Exclude filter hides items that match the filter. Overrides include filters.
  ///   True by default (i.e. include filter).
  Q_INVOKABLE void addItemAttributeFilter(QString attributeName, QVariant attributeValue=QString(), bool include=true);
  /// Remove single item attribute filter specifying each attribute \sa addAttributeFilter
  Q_INVOKABLE void removeItemAttributeFilter(QString attributeName, QVariant attributeValue, bool include);
  /// Remove all item attribute filters specifying a given attribute name and include flag
  Q_INVOKABLE void removeItemAttributeFilter(QString attributeName, bool include);
  /// Add single node attribute filter specifying attribute name, value, include/exclude, and class name
  /// \param attributeName Name of the node attribute to filter
  /// \param attributeValue Value of the node attribute to filter
  /// \param include Flag indicating whether this is an include filter or exclude filter.
  ///   - Include filter means that only the items are shown that match the filter.
  ///   - Exclude filter hides items that match the filter. Overrides include filters.
  ///   True by default (i.e. include filter).
  /// \param className Only filter attributes on a certain type. Empty by default (i.e. allow all classes)
  Q_INVOKABLE void addNodeAttributeFilter(QString attributeName, QVariant attributeValue=QString(), bool include=true, QString className=QString());
  /// Remove single node attribute filter specifying each attribute \sa addAttributeFilter
  Q_INVOKABLE void removeNodeAttributeFilter(QString attributeName, QVariant attributeValue, bool include, QString className);
  /// Remove all node attribute filters specifying a given attribute name and include flag
  Q_INVOKABLE void removeNodeAttributeFilter(QString attributeName, bool include);

  vtkIdType hideItemsUnaffiliatedWithItemID()const;
  void setHideItemsUnaffiliatedWithItemID(vtkIdType itemID);

  bool showEmptyHierarchyItems()const;
  void setShowEmptyHierarchyItems(bool show);

  /// Retrieve the index of the MRML scene (the root item) in the subject hierarchy tree
  Q_INVOKABLE QModelIndex subjectHierarchySceneIndex()const;

  /// Retrieve the associated subject hierarchy item ID from a model index
  Q_INVOKABLE vtkIdType subjectHierarchyItemFromIndex(const QModelIndex& index)const;

  /// Retrieve an index for a given a subject hierarchy item ID
  Q_INVOKABLE QModelIndex indexFromSubjectHierarchyItem(vtkIdType itemID, int column=0)const;

  /// Determine the number of accepted (shown) items
  /// \param rootItemID Ancestor item of branch in which the accepted items are counted.
  ///                   If no item is given, then the scene item is used (all items)
  Q_INVOKABLE int acceptedItemCount(vtkIdType rootItemID=0)const;

  /// Returns true if the item in the row indicated by the given sourceRow and
  /// sourceParent should be included in the model; otherwise returns false.
  /// This method test each item via \a filterAcceptsItem
  bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent)const override;

  Qt::ItemFlags flags(const QModelIndex & index)const override;

public slots:
  void setNameFilter(QString filter);
  void setAttributeNameFilter(QString filter);
  void setAttributeValueFilter(QString filter);
  void setLevelFilter(QStringList filter);
  void setNodeTypes(const QStringList& types);
  void setHideChildNodeTypes(const QStringList& types);
  void setIncludeItemAttributeNamesFilter(QStringList filterList);
  void setIncludeNodeAttributeNamesFilter(QStringList filterList);
  void setExcludeItemAttributeNamesFilter(QStringList filterList);
  void setExcludeNodeAttributeNamesFilter(QStringList filterList);

protected:
  /// This enum type is used to describe the behavior of an item with regard to
  /// filtering:
  ///   * Reject if the item should not be visible and has no chance of being
  ///     visible.
  ///   * Accept if the item should be visible and will always be.
  ///   * AcceptDueToBeingParentOfAccepted if the item should not be visible
  ///     based on the applied filters, but it is the parent of an accepted item,
  ///     so the item needs to be shown to indicate hierarchy.
  enum AcceptType
  {
    Reject = 0,
    Accept,
    AcceptDueToBeingParentOfAccepted,
  };

  /// Filters items to decide which to display in the view
  virtual AcceptType filterAcceptsItem(vtkIdType itemID, bool canAcceptIfAnyChildIsAccepted=true)const;

  QStandardItem* sourceItem(const QModelIndex& index)const;

protected:
  QScopedPointer<qMRMLSortFilterSubjectHierarchyProxyModelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSortFilterSubjectHierarchyProxyModel);
  Q_DISABLE_COPY(qMRMLSortFilterSubjectHierarchyProxyModel);
  friend class qMRMLSubjectHierarchyTreeView;
};

#endif
