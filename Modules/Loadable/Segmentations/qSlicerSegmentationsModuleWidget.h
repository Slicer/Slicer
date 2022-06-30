/*==============================================================================

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

#ifndef __qSlicerSegmentationsModuleWidget_h
#define __qSlicerSegmentationsModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerSegmentationsModuleExport.h"

// SlicerRtCommon includes
#include "vtkMRMLSegmentationNode.h"

// CTK includes
#include <ctkVTKObject.h>

class qSlicerSegmentationsModuleWidgetPrivate;
class vtkMRMLSegmentationNode;
class vtkMRMLSegmentationDisplayNode;
class vtkMRMLSubjectHierarchyNode;
class vtkMRMLNodeReference;
class vtkMRMLNode;
class QItemSelection;
class Ui_qSlicerSegmentationsModule;

/// \ingroup SlicerRt_QtModules_Segmentations
class Q_SLICER_QTMODULES_SEGMENTATIONS_EXPORT qSlicerSegmentationsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerSegmentationsModuleWidget(QWidget *parent=nullptr);
  ~qSlicerSegmentationsModuleWidget() override;

  void enter() override;
  void exit() override;

  /// Support of node editing. Selects node in user interface that the user wants to edit
  bool setEditedNode(vtkMRMLNode* node, QString role=QString(), QString context=QString()) override;

public slots:
  /// Update widget GUI from parameter node
  void updateWidgetFromMRML();

  /// Select segmentation node in module (workaround for issue that newly added nodes are not selected in the module)
  void selectSegmentationNode(vtkMRMLSegmentationNode* segmentationNode);

protected:
  void setup() override;
  void onEnter();

  /// Initialization function to enable automatic testing
  void init();

  /// Get display node of current segmentation node
  /// \param create If on, then create a default display node if missing. False by default
  vtkMRMLSegmentationDisplayNode* segmentationDisplayNode(bool create=false);

  /// Copy segment from one segmentation to another
  /// \param fromSegmentation Source segmentation
  /// \param toSegmentation Target segmentation
  /// \param segmentId ID of segment to copy
  /// \param removeFromSource If true, then delete segment from source segmentation after copying. Default value is false.
  /// \return Success flag
  bool copySegmentBetweenSegmentations(vtkSegmentation* fromSegmentation,
    vtkSegmentation* toSegmentation, QString segmentId, bool removeFromSource=false);

  /// Copy segments to/from current segmentation from/to other segmentation.
  /// \param copyFromCurrentSegmentation If true, then copy current->other; otherwise other->current.
  /// \param removeFromSource If true, then delete segment from source segmentation after copying. Default value is false.
  /// \return Success flag
  bool copySegmentsBetweenSegmentations(bool copyFromCurrentSegmentation, bool removeFromSource = false);

  bool exportFromCurrentSegmentation();
  bool importToCurrentSegmentation();

protected slots:
  /// Handle change of selected segmentation node
  void onSegmentationNodeChanged(vtkMRMLNode* node);

  /// Handle change of selection for the "other" node in copy/move/import/export
  void setOtherSegmentationOrRepresentationNode(vtkMRMLNode* node);

  /// Update copy/move/import/export buttons based on selection
  void updateCopyMoveButtonStates();

  /// Callback function for selection changes in the main segment table view
  void onSegmentSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

  /// Populate source volume label and combobox for export
  void onSegmentationNodeReferenceChanged();

  void onAddSegment();
  void onEditSegmentation();
  void onRemoveSelectedSegments();

  void updateLayerWidgets();
  void collapseLabelmapLayers();

  void updateImportExportWidgets();
  void onImportExportApply();
  void onImportExportClearSelection();
  void updateExportColorWidgets();
  void onExportColorTableChanged();

  void onMoveFromCurrentSegmentation();
  void onCopyFromCurrentSegmentation();
  void onCopyToCurrentSegmentation();
  void onMoveToCurrentSegmentation();

  void onMRMLSceneEndImportEvent();
  void onMRMLSceneEndRestoreEvent();
  void onMRMLSceneEndBatchProcessEvent();
  void onMRMLSceneEndCloseEvent();

protected:
  QScopedPointer<qSlicerSegmentationsModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSegmentationsModuleWidget);
  Q_DISABLE_COPY(qSlicerSegmentationsModuleWidget);
};
#endif
