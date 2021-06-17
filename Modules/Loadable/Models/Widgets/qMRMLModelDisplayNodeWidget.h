/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under this License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLModelDisplayNodeWidget_h
#define __qMRMLModelDisplayNodeWidget_h

// MRMLWidgets includes
#include "qMRMLWidget.h"
#include "qMRMLScalarsDisplayWidget.h"

// CTK includes
#include <ctkVTKObject.h>

// qMRML includes
#include "qSlicerModelsModuleWidgetsExport.h"

class qMRMLModelDisplayNodeWidgetPrivate;
class vtkMRMLColorNode;
class vtkMRMLDisplayNode;
class vtkMRMLModelDisplayNode;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_MODELS_WIDGETS_EXPORT qMRMLModelDisplayNodeWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

  Q_PROPERTY(bool clippingConfigurationButtonVisible READ clippingConfigurationButtonVisible WRITE setClippingConfigurationButtonVisible)

public:
  typedef qMRMLWidget Superclass;
  qMRMLModelDisplayNodeWidget(QWidget* parent = nullptr);
  ~qMRMLModelDisplayNodeWidget() override;

  /// Get model display node (if model was selected not folder)
  vtkMRMLModelDisplayNode* mrmlModelDisplayNode()const;
  /// Get current display node (may be model or folder display node)
  vtkMRMLDisplayNode* mrmlDisplayNode()const;
  /// Get current item (if single selection)
  vtkIdType currentSubjectHierarchyItemID()const;
  /// Get current items (if multi selection)
  QList<vtkIdType> currentSubjectHierarchyItemIDs()const;

  bool visibility()const;
  bool clipping()const;
  bool sliceIntersectionVisible()const;
  int sliceIntersectionThickness()const;
  double sliceIntersectionOpacity()const;
  bool clippingConfigurationButtonVisible()const;

signals:
  /// Signal sent if the auto/manual value is updated
  void scalarRangeModeValueChanged(vtkMRMLDisplayNode::ScalarRangeFlagType value);
  /// Signal sent if the any property in the display node is changed
  void displayNodeChanged();
  /// Signal sent if user toggles clipping checkbox on the GUI
  void clippingToggled(bool);
  /// Signal sent if clipping configuration button is clicked
  void clippingConfigurationButtonClicked();

public slots:
  /// Set the scene.
  void setMRMLScene(vtkMRMLScene* newScene) override;
  /// Set the model display node
  void setMRMLModelDisplayNode(vtkMRMLModelDisplayNode* node);
  /// Utility function to be connected with generic signals
  void setMRMLModelDisplayNode(vtkMRMLNode* node);
  /// Set display node (may be model or folder display node)
  void setMRMLDisplayNode(vtkMRMLDisplayNode* displayNode);
  /// Utility function to be connected with generic signals,
  /// it internally shows the 1st display node.
  /// can be set from the item of a model node or a folder.
  void setCurrentSubjectHierarchyItemID(vtkIdType currentItemID);
  /// Set the current subject hierarchy items.
  /// Both model and folder items are supported. In case of multi
  /// selection, the first item's display properties are displayed
  /// in the widget, but the changed settings are applied on all
  /// selected items if applicable.
  void setCurrentSubjectHierarchyItemIDs(QList<vtkIdType> currentItemIDs);

  void setVisibility(bool);
  void setClipping(bool);

  void setSliceIntersectionVisible(bool);
  void setSliceDisplayMode(int);
  void setSliceIntersectionThickness(int);
  void setSliceIntersectionOpacity(double);
  void setDistanceToColorNode(vtkMRMLNode*);

  void setRepresentation(int);
  void setPointSize(double);
  void setLineWidth(double);
  void setShowFaces(int);
  void setColor(const QColor&);
  void setBackfaceHueOffset(double newOffset);
  void setBackfaceSaturationOffset(double newOffset);
  void setBackfaceBrightnessOffset(double newOffset);

  void setOpacity(double);
  void setEdgeVisibility(bool);
  void setEdgeColor(const QColor&);
  void setLighting(bool);
  void setInterpolation(int);

  /// Show/hide "Configure..." button for clipping
  void setClippingConfigurationButtonVisible(bool);

protected slots:
  void updateWidgetFromMRML();
  void updateDisplayNodesFromProperty();

protected:
  QScopedPointer<qMRMLModelDisplayNodeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLModelDisplayNodeWidget);
  Q_DISABLE_COPY(qMRMLModelDisplayNodeWidget);
};

#endif
