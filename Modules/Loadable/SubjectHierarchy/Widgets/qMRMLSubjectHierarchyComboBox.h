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

#ifndef __qMRMLSubjectHierarchyComboBox_h
#define __qMRMLSubjectHierarchyComboBox_h

// CTK includes
#include <ctkComboBox.h>
#include <ctkVTKObject.h>

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

#include "vtkMRMLSubjectHierarchyNode.h"

class qMRMLSubjectHierarchyComboBoxPrivate;
class qMRMLSortFilterSubjectHierarchyProxyModel;
class qMRMLSubjectHierarchyModel;
class vtkMRMLScene;

/// \ingroup Slicer_QtModules_SubjectHierarchy
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qMRMLSubjectHierarchyComboBox : public ctkComboBox
{
  Q_OBJECT
  QVTK_OBJECT

  /// This property controls whether the root item (folder, an item for a data node, or the scene itself),
  /// if any is visible. When the root item is visible, it appears as a top-level item,
  /// if it is hidden only its children are top-level items. It doesn't have any effect if \a rootItem() is invalid. Shown by default.
  /// \sa setShowRootItem(), showRootItem(), setRootItem(), setRootIndex()
  Q_PROPERTY(bool showRootItem READ showRootItem WRITE setShowRootItem)
  /// Flag determining whether to highlight items referenced by DICOM. Storing DICOM references:
  ///   Referenced SOP instance UIDs (in attribute named vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName())
  ///   -> SH item instance UIDs (serialized string lists in subject hierarchy UID vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName())
  Q_PROPERTY(bool highlightReferencedItems READ highlightReferencedItems WRITE setHighlightReferencedItems)
  /// Property determining the maximum number of items (rows) shown in the popup tree
  Q_PROPERTY(int maximumNumberOfShownItems READ maximumNumberOfShownItems WRITE setMaximumNumberOfShownItems)
  /// Property determining the vertical alignment of the popup tree with the combobox.
  /// If aligned, the popup will shift vertically so that the selected item overlays above the combobox.
  /// Else, the popup tree appears below the combobox, like for a qMRMLNodeComboBox.
  Q_PROPERTY(bool alignPopupVertically READ alignPopupVertically WRITE setAlignPopupVertically)
  /// This property controls whether an extra item is added before any subject hierarchy item under
  /// the scene item for indicating 'None' selection.
  Q_PROPERTY(bool noneEnabled READ noneEnabled WRITE setNoneEnabled)
  /// This property controls the name that is displayed for the None item.
  /// "None" by default.
  /// \sa noneItemEnabled
  Q_PROPERTY(QString noneDisplay READ noneDisplay WRITE setNoneDisplay)
  /// This property controls whether hierarchy information is included in the current item text (showed when collapsed).
  /// If enabled (which is the default), then the text looks like "[ParentName] / [ParentName] / [SelectedItemName]".
  /// If disabled, the title will only be "[SelectedItemName]".
  Q_PROPERTY(bool showCurrentItemParents READ showCurrentItemParents WRITE setShowCurrentItemParents)
  /// Show hierarchy items (items without data node such as patient/study/folder) that have no children to show
  /// according to the filters.
  /// This is most useful to in off state keep combobox content reasonable size for selecting data nodes.
  /// True by default.
  Q_PROPERTY(bool showEmptyHierarchyItems READ showEmptyHierarchyItems WRITE setShowEmptyHierarchyItems)

  /// Filter to show only items that contain any of the given attributes with this name. Empty by default
  Q_PROPERTY(QStringList includeItemAttributeNamesFilter READ includeItemAttributeNamesFilter WRITE setIncludeItemAttributeNamesFilter)
  /// Filter to show only items for data nodes that contain any of the given attributes with this name. Empty by default
  Q_PROPERTY(QStringList includeNodeAttributeNamesFilter READ includeNodeAttributeNamesFilter WRITE setIncludeNodeAttributeNamesFilter)
  /// Filter to hide items that contain any of the given attributes with this name. Empty by default
  /// Overrides \sa includeItemAttributeNamesFilter
  Q_PROPERTY(QStringList excludeItemAttributeNamesFilter READ excludeItemAttributeNamesFilter WRITE setExcludeItemAttributeNamesFilter)
  /// Filter to hide items for data nodes that contain any of the given attributes with this name. Empty by default
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
  typedef ctkComboBox Superclass;
  qMRMLSubjectHierarchyComboBox(QWidget *parent=nullptr);
  ~qMRMLSubjectHierarchyComboBox() override;

public:
  Q_INVOKABLE vtkMRMLScene* mrmlScene()const;
  Q_INVOKABLE vtkMRMLSubjectHierarchyNode* subjectHierarchyNode()const;

  Q_INVOKABLE void clearSelection();
  Q_INVOKABLE vtkIdType currentItem()const;
  Q_INVOKABLE vtkIdType rootItem()const;

  void setShowRootItem(bool show);
  bool showRootItem()const;

  bool highlightReferencedItems()const;
  void setHighlightReferencedItems(bool highlightOn);

  int maximumNumberOfShownItems()const;
  void setMaximumNumberOfShownItems(int maxNumberOfShownItems);

  bool alignPopupVertically()const;
  void setAlignPopupVertically(bool align);

  bool noneEnabled()const;
  void setNoneEnabled(bool enable);

  QString noneDisplay()const;
  void setNoneDisplay(const QString& displayName);

  bool showCurrentItemParents()const;
  void setShowCurrentItemParents(bool enable);

  bool showEmptyHierarchyItems()const;
  void setShowEmptyHierarchyItems(bool show);

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


  /// Set level filter that allows showing only items at a specified level and their parents. Show all items if empty
  Q_INVOKABLE void setLevelFilter(QStringList &levelFilter);
  /// Set node type filter that allows showing only data nodes of a certain type. Show all data nodes if empty
  Q_INVOKABLE void setNodeTypes(const QStringList& types);
  /// Set child node types filter that allows hiding certain data node subclasses that would otherwise be
  /// accepted by the data node type filter. Show all data nodes if empty
  Q_INVOKABLE void setHideChildNodeTypes(const QStringList& types);

  Q_INVOKABLE qMRMLSortFilterSubjectHierarchyProxyModel* sortFilterProxyModel()const;
  Q_INVOKABLE qMRMLSubjectHierarchyModel* model()const;

public:
  /// Provides customized popup window for the tree view
  void showPopup() override;

public slots:
  /// Set MRML scene
  virtual void setMRMLScene(vtkMRMLScene* scene);

  /// Set current (=selected) subject hierarchy item
  virtual void setCurrentItem(vtkIdType itemID);

  /// Set subject hierarchy item to be the root in the shown tree
  virtual void setRootItem(vtkIdType itemID);

  /// Set list of subject hierarchy plugins that are enabled.
  /// \param allowlist List of allowed subject hierarchy plugin names.
  ///   An empty allowlist means all plugins are enabled. That is the default.
  void setPluginAllowlist(QStringList allowlist);
  /// Set list of subject hierarchy plugins that are disabled.
  /// \param blocklist List of blocked subject hierarchy plugin names.
  ///   An empty blocklist means all plugins are enabled. That is the default.
  void setPluginBlocklist(QStringList blocklist);
  /// Disable subject hierarchy plugin by adding it to the blocklist \sa setPluginBlocklist
  /// \param plugin Name of the plugin to disable
  void disablePlugin(QString plugin);

  /// Deprecated. Use setPluginAllowlist instead.
  void setPluginWhitelist(QStringList allowlist)
    {
    qWarning("qMRMLSubjectHierarchyComboBox::setPluginWhitelist is deprecated. Use setPluginAllowlist instead.");
    this->setPluginAllowlist(allowlist);
    }
  /// Deprecated. Use setPluginBlocklist instead.
  void setPluginBlacklist(QStringList blocklist)
    {
    qWarning("qMRMLSubjectHierarchyComboBox::setPluginBlacklist is deprecated. Use setPluginBlocklist instead.");
    this->setPluginBlocklist(blocklist);
    }

  void setIncludeItemAttributeNamesFilter(QStringList filter);
  void setIncludeNodeAttributeNamesFilter(QStringList filter);
  void setExcludeItemAttributeNamesFilter(QStringList filter);
  void setExcludeNodeAttributeNamesFilter(QStringList filter);
  void setAttributeNameFilter(QString& filter);
  void setAttributeValueFilter(QString& filter);

signals:
  void currentItemChanged(vtkIdType);
  void currentItemModified(vtkIdType);

protected slots:
  void updateComboBoxTitleAndIcon(vtkIdType selectedShItemID);

  void hidePopup() override;

  virtual void onMRMLSceneCloseEnded(vtkObject* sceneObject);

protected:
  /// Handle mouse press event (disable context menu)
  void mousePressEvent(QMouseEvent* event) override;

protected:
  QScopedPointer<qMRMLSubjectHierarchyComboBoxPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSubjectHierarchyComboBox);
  Q_DISABLE_COPY(qMRMLSubjectHierarchyComboBox);
};

#endif
