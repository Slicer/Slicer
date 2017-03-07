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

public:
  typedef ctkComboBox Superclass;
  qMRMLSubjectHierarchyComboBox(QWidget *parent=0);
  virtual ~qMRMLSubjectHierarchyComboBox();

public:
  Q_INVOKABLE vtkMRMLScene* mrmlScene()const;
  Q_INVOKABLE vtkMRMLSubjectHierarchyNode* subjectHierarchyNode()const;

  Q_INVOKABLE vtkIdType currentItem()const;
  Q_INVOKABLE vtkIdType rootItem()const;

  void setShowRootItem(bool show);
  bool showRootItem()const;

  bool highlightReferencedItems()const;
  void setHighlightReferencedItems(bool highlightOn);

  int maximumNumberOfShownItems()const;
  void setMaximumNumberOfShownItems(int maxNumberOfShownItems);

  /// Set attribute filter that allows showing only items that have the specified attribute and their parents.
  /// \param attributeName Name of the attribute by which the items are filtered
  /// \param attributeValue Value of the specified attribute that needs to match this given value in order
  ///   for it to be shown. If empty, then existence of the attribute is enough to show. Empty by default
  Q_INVOKABLE void setAttributeFilter(const QString& attributeName, const QVariant& attributeValue=QVariant());
  /// Remove item attribute filtering \sa setAttribute
  Q_INVOKABLE void removeAttributeFilter();

  /// Set level filter that allows showing only items at a specified level and their parents. Show all items if empty
  Q_INVOKABLE void setLevelFilter(QString &levelFilter);

  Q_INVOKABLE qMRMLSortFilterSubjectHierarchyProxyModel* sortFilterProxyModel()const;
  Q_INVOKABLE qMRMLSubjectHierarchyModel* model()const;

public:
  /// Provides customized popup window for the tree view
  virtual void showPopup();

public slots:
  /// Set MRML scene
  virtual void setMRMLScene(vtkMRMLScene* scene);

  /// Set current (=selected) subject hierarchy item
  virtual void setCurrentItem(vtkIdType itemID);

  /// Set subject hierarchy item to be the root in the shown tree
  virtual void setRootItem(vtkIdType itemID);

signals:
  void currentItemChanged(vtkIdType);

protected slots:
  void updateComboBoxTitleAndIcon(vtkIdType selectedShItemID);

protected:
  /// Handle mouse press event (disable context menu)
  virtual void mousePressEvent(QMouseEvent* event);

protected:
  QScopedPointer<qMRMLSubjectHierarchyComboBoxPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSubjectHierarchyComboBox);
  Q_DISABLE_COPY(qMRMLSubjectHierarchyComboBox);
};

#endif
