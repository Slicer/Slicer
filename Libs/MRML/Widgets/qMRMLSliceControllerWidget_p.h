/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLSliceControllerWidget_p_h
#define __qMRMLSliceControllerWidget_p_h

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

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLSliceControllerWidget.h"
#include "qMRMLViewControllerBar_p.h"
#include "ui_qMRMLSliceControllerWidget.h"

// MRMLLogic includes
#include <vtkMRMLSliceLogic.h>

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class ctkSignalMapper;
class ctkDoubleSpinBox;
class ctkVTKSliceView;
class QSpinBox;
class qMRMLSliderWidget;
class vtkMRMLSliceNode;
class vtkObject;
class vtkMRMLSegmentationDisplayNode;

//-----------------------------------------------------------------------------
struct QMRML_WIDGETS_EXPORT qMRMLOrientation
{
  QString Prefix;
  QString ToolTip;
};

//-----------------------------------------------------------------------------
class QMRML_WIDGETS_EXPORT qMRMLSliceControllerWidgetPrivate
  : public qMRMLViewControllerBarPrivate
  , public Ui_qMRMLSliceControllerWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLSliceControllerWidget);

public:
  typedef qMRMLSliceControllerWidgetPrivate Self;
  typedef qMRMLViewControllerBarPrivate Superclass;
  qMRMLSliceControllerWidgetPrivate(qMRMLSliceControllerWidget& object);
  ~qMRMLSliceControllerWidgetPrivate() override;

  void init() override;
  void setColor(QColor color) override;

  void setupLinkedOptionsMenu();
  void setupReformatOptionsMenu();
  void setupLightboxMenu();
  void setupCompositingMenu();
  void setupSliceSpacingMenu();
  void setupSliceModelMenu();
  void setupSegmentationMenu();
  void setupLabelMapMenu();
  void setupMoreOptionsMenu();
  void setupOrientationMarkerMenu();
  void setupRulerMenu();

  qMRMLOrientation mrmlOrientation(const QString& name);

  vtkSmartPointer<vtkCollection> saveNodesForUndo(const QString& nodeTypes);

  void enableLayerWidgets();

  vtkMRMLSliceLogic* compositeNodeLogic(vtkMRMLSliceCompositeNode* node);
  vtkMRMLSliceLogic* sliceNodeLogic(vtkMRMLSliceNode* node);

  void setForegroundInterpolation(vtkMRMLSliceLogic* logic, bool interpolate);
  void setBackgroundInterpolation(vtkMRMLSliceLogic* logic, bool interpolate);

  /// Create a list of orientation containing the regular presets and also
  /// the "Reformat" string if sliceToRAS is different one of the preset.
  static void updateSliceOrientationSelector(
      vtkMRMLSliceNode* sliceNode, QComboBox *sliceOrientationSelector);

public slots:
  /// Update widget state when the scene is modified
  void updateFromMRMLScene();

  /// Update widget state using the associated MRML slice node
  void updateWidgetFromMRMLSliceNode();

  /// Update widget state using the associated MRML slice composite node
  void updateWidgetFromMRMLSliceCompositeNode();

  /// Called after a foreground layer volume node is selected
  /// using the associated qMRMLNodeComboBox
  void onForegroundLayerNodeSelected(vtkMRMLNode* node);

  /// Called after a background layer volume node is selected
  /// using the associated qMRMLNodeComboBox
  void onBackgroundLayerNodeSelected(vtkMRMLNode* node);

  /// Called after a label layer volume node is selected
  /// using the associated qMRMLNodeComboBox
  void onLabelMapNodeSelected(vtkMRMLNode* node);

  /// Called after a segmentation node is selected in the combobox
  void onSegmentationNodeSelected(vtkMRMLNode* node);

  /// Called after the currently selected segmentation node's display
  /// option is modified
  void onSegmentationNodeDisplayModifiedEvent(vtkObject* nodeObject);
  /// Called when segment visibility is changed from the segment combobox
  void onSegmentVisibilitySelectionChanged(QStringList selectedSegmentIDs);
  /// Update segmentation outline/fill button
  void updateSegmentationOutlineFillButton();
  /// Utility function to get the display node of the current segmentation
  vtkMRMLSegmentationDisplayNode* currentSegmentationDisplayNode();

  void updateFromForegroundDisplayNode(vtkObject* displayNode);
  void updateFromBackgroundDisplayNode(vtkObject* displayNode);

  void updateFromForegroundVolumeNode(vtkObject* volumeNode);
  void updateFromBackgroundVolumeNode(vtkObject* volumeNode);

  /// Called after the SliceLogic is modified
  void onSliceLogicModifiedEvent();

  void applyCustomLightbox();

protected:
  void setupPopupUi() override;
  virtual void setMRMLSliceNodeInternal(vtkMRMLSliceNode* sliceNode);
  void setMRMLSliceCompositeNodeInternal(vtkMRMLSliceCompositeNode* sliceComposite);

public:
  vtkMRMLSliceNode*                   MRMLSliceNode;
  vtkMRMLSliceCompositeNode*          MRMLSliceCompositeNode;
  vtkSmartPointer<vtkMRMLSliceLogic>  SliceLogic;
  vtkCollection*                      SliceLogics;
  vtkWeakPointer<vtkAlgorithmOutput>  ImageDataConnection;
  QHash<QString, qMRMLOrientation>    SliceOrientationToDescription;
  QString                             SliceViewName;
  QButtonGroup*                       ControllerButtonGroup;

  QToolButton*                        FitToWindowToolButton;
  qMRMLSliderWidget*                  SliceOffsetSlider;
  double                              LastLabelMapOpacity;
  double                              LastForegroundOpacity;
  double                              LastBackgroundOpacity;

  QMenu*                              LightboxMenu;
  QMenu*                              CompositingMenu;
  QMenu*                              SliceSpacingMenu;
  QMenu*                              SliceModelMenu;
  QMenu*                              SegmentationMenu;
  QMenu*                              LabelMapMenu;
  QMenu*                              OrientationMarkerMenu;
  QMenu*                              RulerMenu;

  ctkDoubleSpinBox*                   SliceSpacingSpinBox;
  ctkDoubleSpinBox*                   SliceFOVSpinBox;
  QSpinBox*                           LightBoxRowsSpinBox;
  QSpinBox*                           LightBoxColumnsSpinBox;

  ctkDoubleSpinBox*                   SliceModelFOVXSpinBox;
  ctkDoubleSpinBox*                   SliceModelFOVYSpinBox;

  ctkDoubleSpinBox*                   SliceModelOriginXSpinBox;
  ctkDoubleSpinBox*                   SliceModelOriginYSpinBox;

  QSpinBox*                           SliceModelDimensionXSpinBox;
  QSpinBox*                           SliceModelDimensionYSpinBox;

  QSize                               ViewSize;

  ctkSignalMapper*                    OrientationMarkerTypesMapper;
  ctkSignalMapper*                    OrientationMarkerSizesMapper;

  ctkSignalMapper*                    RulerTypesMapper;
  ctkSignalMapper*                    RulerColorMapper;
};

#endif
