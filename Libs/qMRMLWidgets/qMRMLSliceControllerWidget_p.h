/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include "ui_qMRMLSliceControllerWidget.h"

// MRMLLogic includes
#include <vtkMRMLSliceLogic.h>

/// VTK includes
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class QSpinBox;
class QDoubleSpinBox;
class ctkVTKSliceView;
class vtkMRMLSliceNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qMRMLSliceControllerWidgetPrivate: public QObject,
                                   public Ui_qMRMLSliceControllerWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLSliceControllerWidget);
protected:
  qMRMLSliceControllerWidget* const q_ptr;
public:
  qMRMLSliceControllerWidgetPrivate(qMRMLSliceControllerWidget& object);

  void setupUi(qMRMLWidget* widget);

  void setupMoreOptionsMenu();

  vtkSmartPointer<vtkCollection> saveNodesForUndo(const QString& nodeTypes);

  vtkMRMLSliceLogic* compositeNodeLogic(vtkMRMLSliceCompositeNode* node);
  vtkMRMLSliceLogic* sliceNodeLogic(vtkMRMLSliceNode* node);

  void fitSliceToBackground(vtkMRMLSliceLogic* sliceLogic);
  void setForegroundInterpolation(vtkMRMLSliceLogic* logic, bool interpolate);
  void setBackgroundInterpolation(vtkMRMLSliceLogic* logic, bool interpolate);
public slots:
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

  /// Called after the ImageData associated with the SliceLogic is modified
  void onImageDataModifiedEvent();

  void updateFromForegroundDisplayNode(vtkObject* displayNode);
  void updateFromBackgroundDisplayNode(vtkObject* displayNode);

  void toggleControllerWidgetGroupVisibility();

  void toggleLabelOpacity(bool toggle);

  void applyCustomLightbox();
protected:
  void setMRMLSliceNodeInternal(vtkMRMLSliceNode* sliceNode);
  void setMRMLSliceCompositeNodeInternal(vtkMRMLSliceCompositeNode* sliceComposite);

public:
  vtkMRMLSliceNode*                   MRMLSliceNode;
  vtkMRMLSliceCompositeNode*          MRMLSliceCompositeNode;
  vtkSmartPointer<vtkMRMLSliceLogic>  SliceLogic;
  vtkCollection*                      SliceLogics;
  vtkWeakPointer<vtkImageData>        ImageData;
  QString                             SliceOrientation;
  QHash<QString, QString>             SliceOrientationToDescription;
  QString                             SliceViewName;
  QButtonGroup*                       ControllerButtonGroup;

  ctkSliderWidget*                    LabelOpacitySlider;
  QToolButton*                        LabelOpacityToggleButton;
  double                              LastLabelOpacity;

  QDoubleSpinBox*                     SliceSpacingSpinBox;

  QSpinBox*                           LightBoxRowsSpinBox;
  QSpinBox*                           LightBoxColumnsSpinBox;

  QSize                               ViewSize;
};

#endif
