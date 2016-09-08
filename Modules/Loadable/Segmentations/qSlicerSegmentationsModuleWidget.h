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

// SlicerQt includes
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
  qSlicerSegmentationsModuleWidget(QWidget *parent=0);
  virtual ~qSlicerSegmentationsModuleWidget();

  virtual void enter();
  virtual void exit();

  /// Support of node editing. Selects node in user interface that the user wants to edit
  virtual bool setEditedNode(vtkMRMLNode* node, QString role=QString(), QString context=QString());

public slots:
  /// Update widget GUI from parameter node
  void updateWidgetFromMRML();

  /// Select segmentation node in module (workaround for issue that newly added nodes are not selected in the module)
  void selectSegmentationNode(vtkMRMLSegmentationNode* segmentationNode);

protected:
  virtual void setup();
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

  /// Copy segment from current segmentation to other segmentation/representation
  /// \param removeFromSource If true, then delete segment from source segmentation after copying. Default value is false.
  /// \return Success flag
  bool copyFromCurrentSegmentation(bool removeFromSource=false);

  /// Update master representation in segmentation to a given representation.
  /// Used before adding a certain segment to a segmentation, making sure the user knows if data loss is possible.
  /// 1. Segmentation is empty or master is unspecified -> Master is changed to the segment's representation type
  /// 2. Segmentation is non-empty and master matches the representation -> No action
  /// 3. Segmentation is non-empty and master differs -> Choice presented to user
  /// \return False only if user chose not to change master representation on option 3, or if error occurred, otherwise true
  bool updateMasterRepresentationInSegmentation(vtkSegmentation* segmentation, QString representation);

protected slots:
  /// Handle change of selected segmentation node
  void onSegmentationNodeChanged(vtkMRMLNode* node);

  /// Handle change of selection for the "other" node in copy/move/import/export
  void setOtherSegmentationOrRepresentationNode(vtkMRMLNode* node);

  /// Update copy/move/import/export buttons based on selection
  void updateCopyMoveButtonStates();

  /// Callback function for selection changes in the main segment table view
  void onSegmentSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

  void onAddSegment();
  void onEditSelectedSegment();
  void onRemoveSelectedSegments();

  void onAddLabelmap();
  void onAddModel();

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
