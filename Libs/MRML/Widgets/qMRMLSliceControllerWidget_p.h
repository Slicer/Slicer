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

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLSliceControllerWidget.h"
#include "qMRMLViewControllerBar_p.h"
#include "ui_qMRMLSliceControllerWidget.h"

// MRMLLogic includes
#include <vtkMRMLSliceLogic.h>

/// VTK includes
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class ctkSignalMapper;
class ctkSpinBox;
class ctkVTKSliceView;
class QSpinBox;
class qMRMLSliderWidget;
class vtkMRMLSliceNode;
class vtkObject;

//-----------------------------------------------------------------------------
struct qMRMLOrientation
{
  QString Prefix;
  QString ToolTip;
};

//-----------------------------------------------------------------------------
class qMRMLSliceControllerWidgetPrivate
  : public qMRMLViewControllerBarPrivate
  , public Ui_qMRMLSliceControllerWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLSliceControllerWidget);

public:
  typedef qMRMLViewControllerBarPrivate Superclass;
  qMRMLSliceControllerWidgetPrivate(qMRMLSliceControllerWidget& object);
  virtual ~qMRMLSliceControllerWidgetPrivate();

  virtual void init();
  virtual void setColor(QColor color);

  void setupLinkedOptionsMenu();
  void setupReformatOptionsMenu();
  void setupLightboxMenu();
  void setupCompositingMenu();
  void setupSliceSpacingMenu();
  void setupSliceModelMenu();
  void setupMoreOptionsMenu();

  vtkSmartPointer<vtkCollection> saveNodesForUndo(const QString& nodeTypes);
  
  void enableLayerWidgets();

  vtkMRMLSliceLogic* compositeNodeLogic(vtkMRMLSliceCompositeNode* node);
  vtkMRMLSliceLogic* sliceNodeLogic(vtkMRMLSliceNode* node);

  void setForegroundInterpolation(vtkMRMLSliceLogic* logic, bool interpolate);
  void setBackgroundInterpolation(vtkMRMLSliceLogic* logic, bool interpolate);

public slots:
  /// Update widget state when the scene is modified
  void updateFromMRMLScene();

  /// Update widget state using the associated MRML slice node
  void updateWidgetFromMRMLSliceNode();

  /// Update widget state using the associated MRML slice composite node
  void updateWidgetFromMRMLSliceCompositeNode();

  /// Called after a foregound layer volume node is selected
  /// using the associated qMRMLNodeComboBox
  void onForegroundLayerNodeSelected(vtkMRMLNode* node);

  /// Called after a backgound layer volume node is selected
  /// using the associated qMRMLNodeComboBox
  void onBackgroundLayerNodeSelected(vtkMRMLNode* node);

  /// Called after a backgound layer volume node is selected
  /// using the associated qMRMLNodeComboBox
  void onLabelMapNodeSelected(vtkMRMLNode* node);

  /// Called after the SliceLogic is modified
  void onSliceLogicModifiedEvent();

  void updateFromForegroundDisplayNode(vtkObject* displayNode);
  void updateFromBackgroundDisplayNode(vtkObject* displayNode);

  void updateFromForegroundVolumeNode(vtkObject* volumeNode);
  void updateFromBackgroundVolumeNode(vtkObject* volumeNode);

  void applyCustomLightbox();

protected:
  virtual void setupPopupUi();
  void setMRMLSliceNodeInternal(vtkMRMLSliceNode* sliceNode);
  void setMRMLSliceCompositeNodeInternal(vtkMRMLSliceCompositeNode* sliceComposite);

public:
  vtkMRMLSliceNode*                   MRMLSliceNode;
  vtkMRMLSliceCompositeNode*          MRMLSliceCompositeNode;
  vtkSmartPointer<vtkMRMLSliceLogic>  SliceLogic;
  vtkCollection*                      SliceLogics;
  vtkWeakPointer<vtkImageData>        ImageData;
  QString                             SliceOrientation;
  QHash<QString, qMRMLOrientation>    SliceOrientationToDescription;
  QString                             SliceViewName;
  QString                             SliceViewLabel;
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

  ctkSpinBox*                         SliceSpacingSpinBox;
  ctkSpinBox*                         SliceFOVSpinBox;
  QSpinBox*                           LightBoxRowsSpinBox;
  QSpinBox*                           LightBoxColumnsSpinBox;

  ctkSpinBox*                         SliceModelFOVXSpinBox;
  ctkSpinBox*                         SliceModelFOVYSpinBox;

  ctkSpinBox*                         SliceModelOriginXSpinBox;
  ctkSpinBox*                         SliceModelOriginYSpinBox;

  QSpinBox*                           SliceModelDimensionXSpinBox;
  QSpinBox*                           SliceModelDimensionYSpinBox;

  QSize                               ViewSize;
};

#endif
