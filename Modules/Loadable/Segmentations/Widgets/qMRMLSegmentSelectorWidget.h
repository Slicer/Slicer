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

// MRMLWidgets includes
#include "qMRMLWidget.h"

#include "qSlicerSegmentationsModuleWidgetsExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class vtkMRMLNode;
class qMRMLSegmentSelectorWidgetPrivate;
class QTableWidgetItem;
class QItemSelection;

/// \brief Qt widget for selecting a single segment from a segmentation.
///   If multiple segments are needed, then use \sa qMRMLSegmentsTableView instead in SimpleListMode
/// \ingroup SlicerRt_QtModules_Segmentations_Widgets
class Q_SLICER_MODULE_SEGMENTATIONS_WIDGETS_EXPORT qMRMLSegmentSelectorWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  Q_PROPERTY(bool noneEnabled READ noneEnabled WRITE setNoneEnabled)
  Q_PROPERTY(bool segmentationNodeSelectorVisible READ segmentationNodeSelectorVisible WRITE setSegmentationNodeSelectorVisible)

public:
  /// Constructor
  explicit qMRMLSegmentSelectorWidget(QWidget* parent = 0);
  /// Destructor
  virtual ~qMRMLSegmentSelectorWidget();

  /// Get currently selected segmentation MRML node
  Q_INVOKABLE vtkMRMLNode* currentNode();
  /// Get currently selected segmentation node's ID
  Q_INVOKABLE QString currentNodeID();

  /// Get segment ID of selected segment
  Q_INVOKABLE QString currentSegmentID();

  /// Return true if the "none" is in the segmentation node comboBox list, false otherwise.
  /// \sa noneEnabled, setNoneEnabled()
  bool noneEnabled()const;
  /// Set whether the "none" item should be in the segmentation node comboBox list or not.
  /// \sa noneEnabled, noneEnabled()
  void setNoneEnabled(bool enable);

  /// Return whether segmentation node selector combobox is visible
  /// \sa segmentationNodeSelectorVisible, setSegmentationNodeSelectorVisible()
  bool segmentationNodeSelectorVisible()const;
  /// Set whether segmentation node selector combobox is visible
  /// \sa segmentationNodeSelectorVisible, segmentationNodeSelectorVisible()
  void setSegmentationNodeSelectorVisible(bool visible);

public slots:
  /// Set segmentation MRML node
  Q_INVOKABLE void setCurrentNode(vtkMRMLNode* node);
  /// Set segmentation MRML node by its ID
  Q_INVOKABLE void setCurrentNodeID(const QString& nodeID);

  /// Get segment ID of selected segment
  Q_INVOKABLE void setCurrentSegmentID(QString segmentID);

protected slots:
  /// Handles changing of current segmentation MRML node
  Q_INVOKABLE void onCurrentNodeChanged(vtkMRMLNode* node);
  /// Handles changing of selection in the segment combobox
  void onCurrentSegmentChanged(int index);

  /// Populate segment combobox according to the segmentation node
  void populateSegmentCombobox();

signals:
  void currentNodeChanged(vtkMRMLNode*);
  void currentSegmentChanged(QString);

protected:
  QScopedPointer<qMRMLSegmentSelectorWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSegmentSelectorWidget);
  Q_DISABLE_COPY(qMRMLSegmentSelectorWidget);
};

#endif
