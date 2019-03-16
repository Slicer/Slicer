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

#ifndef __qMRMLSegmentSelectorWidget_h
#define __qMRMLSegmentSelectorWidget_h

// qMRMLWidgets includes
#include "qMRMLWidget.h"
#include "qMRMLWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class vtkMRMLNode;
class qMRMLSegmentSelectorWidgetPrivate;
class QTableWidgetItem;
class QItemSelection;
class QStringList;

/// \brief Qt widget for selecting a single segment from a segmentation.
///   If multiple segments are needed, then use \sa qMRMLSegmentsTableView instead in SimpleListMode
/// \ingroup SlicerRt_QtModules_Segmentations_Widgets
class QMRML_WIDGETS_EXPORT qMRMLSegmentSelectorWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  Q_PROPERTY(bool noneEnabled READ noneEnabled WRITE setNoneEnabled)
  Q_PROPERTY(bool editEnabled READ editEnabled WRITE setEditEnabled)
  Q_PROPERTY(bool segmentationNodeSelectorVisible READ segmentationNodeSelectorVisible WRITE setSegmentationNodeSelectorVisible)
  Q_PROPERTY(bool multiSelection READ multiSelection WRITE setMultiSelection)
  Q_PROPERTY(bool horizontalLayout READ horizontalLayout WRITE setHorizontalLayout)
  Q_PROPERTY(bool selectNodeUponCreation READ selectNodeUponCreation WRITE setSelectNodeUponCreation)

public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;

  /// Constructor
  explicit qMRMLSegmentSelectorWidget(QWidget* parent = nullptr);
  /// Destructor
  ~qMRMLSegmentSelectorWidget() override;

  /// Get currently selected segmentation MRML node
  Q_INVOKABLE vtkMRMLNode* currentNode();
  /// Get currently selected segmentation node's ID
  Q_INVOKABLE QString currentNodeID();

  /// Get segment ID of selected segment.
  /// Returns empty string if multi-selection is enabled (when \sa multiSelection is true)
  Q_INVOKABLE QString currentSegmentID();
  /// Get segment IDs of selected segments.
  /// Returns empty string in case of single selection (when \sa multiSelection is false)
  Q_INVOKABLE QStringList selectedSegmentIDs();

  /// Return true if the "none" is in the segmentation node comboBox list, false otherwise.
  /// \sa noneEnabled, setNoneEnabled()
  bool noneEnabled()const;
  /// Set whether the "none" item should be in the segmentation node comboBox list or not.
  /// \sa noneEnabled, noneEnabled()
  void setNoneEnabled(bool enable);

  /// Return true if the edit option is enabled in the segmentation node combobox
  /// \sa noneEnabled, setEditEnabled()
  bool editEnabled()const;
  /// Set whether the edit option is enabled in the segmentation node combobox
  /// \sa editEnabled, editEnabled()
  void setEditEnabled(bool enable);

  /// Return whether segmentation node selector combobox is visible
  /// \sa segmentationNodeSelectorVisible, setSegmentationNodeSelectorVisible()
  bool segmentationNodeSelectorVisible()const;
  /// Set whether segmentation node selector combobox is visible
  /// \sa segmentationNodeSelectorVisible, segmentationNodeSelectorVisible()
  void setSegmentationNodeSelectorVisible(bool visible);

  /// Return true if the multi-selection of segments is enabled, false otherwise.
  /// \sa multiSelection, setMultiSelection()
  bool multiSelection()const;
  /// Set whether multi-selection of segments is enabled or not. Off by default (single-selection)
  /// \sa multiSelection, multiSelection()
  void setMultiSelection(bool multi);

  /// Return true if the current layout is horizontal
  /// \sa horizontalLayout, setHorizontalLayout()
  bool horizontalLayout()const;
  /// Set whether the widget layout is horizontal (everything in one row) or vertical. False by default
  /// \sa horizontalLayout, horizontalLayout()
  void setHorizontalLayout(bool horizontal);

  /// if true, when the user create a node using "Add node", the node will be
  /// automatically selected. It doesn't apply if the node is programmatically
  /// added (when the combobox is populated by the scene).
  bool selectNodeUponCreation()const;
  void setSelectNodeUponCreation(bool value);

public slots:
  /// Set segmentation MRML node
  Q_INVOKABLE void setCurrentNode(vtkMRMLNode* node);
  /// Set segmentation MRML node by its ID
  Q_INVOKABLE void setCurrentNodeID(const QString& nodeID);

  /// Select segment with given ID.
  /// Does nothing in case multi-selection is enabled (when \sa multiSelection is true)
  Q_INVOKABLE void setCurrentSegmentID(QString segmentID);
  /// Select segments with given IDs.
  /// Does nothing in case of single selection (when \sa multiSelection is false)
  Q_INVOKABLE void setSelectedSegmentIDs(QStringList segmentIDList);

protected slots:
  /// Handles changing of current segmentation MRML node
  Q_INVOKABLE void onCurrentNodeChanged(vtkMRMLNode* node);
  /// Handles changing of selection in the segment combobox
  void onCurrentSegmentChanged(int index);
  /// Handles selection change in case of multi-selection
  void onSegmentMultiSelectionChanged();

  /// Populate segment combobox according to the segmentation node
  void populateSegmentCombobox();

signals:
  void currentNodeChanged(vtkMRMLNode*);
  void currentSegmentChanged(QString);
  void segmentSelectionChanged(QStringList);

  /// Signal emitted when edit node menu action is selected in the segmentation node combobox
  void nodeAboutToBeEdited(vtkMRMLNode* node);

  /// Signal emitted just after currentNodeChanged(vtkMRMLNode*) is.
  /// \param validNode set to True when the current node is changed. Set to False when the list is empty.
  /// Useful to enable/disable/show/hide other widgets depending on the validity of the current node.
  void currentNodeChanged(bool validNode);

protected:
  QScopedPointer<qMRMLSegmentSelectorWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSegmentSelectorWidget);
  Q_DISABLE_COPY(qMRMLSegmentSelectorWidget);
};

#endif
