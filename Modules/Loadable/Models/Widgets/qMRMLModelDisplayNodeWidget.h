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

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// qMRML includes
#include "qSlicerModelsModuleWidgetsExport.h"

class qMRMLModelDisplayNodeWidgetPrivate;
class vtkMRMLScene;
class vtkMRMLNode;
class vtkMRMLModelDisplayNode;
class vtkMRMLColorNode;
class vtkMRMLSelectionNode;

class Q_SLICER_QTMODULES_MODELS_WIDGETS_EXPORT qMRMLModelDisplayNodeWidget : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

  Q_PROPERTY(ControlMode scalarRangeMode READ scalarRangeMode WRITE setScalarRangeMode)
  Q_PROPERTY(double minimumValue READ minimumValue WRITE setMinimumValue)
  Q_PROPERTY(double maximumValue READ maximumValue WRITE setMaximumValue)
  Q_PROPERTY(bool clippingConfigurationButtonVisible READ clippingConfigurationButtonVisible WRITE setClippingConfigurationButtonVisible)
  Q_ENUMS(ControlMode)

public:

  qMRMLModelDisplayNodeWidget(QWidget *parent=nullptr);
  ~qMRMLModelDisplayNodeWidget() override;

  vtkMRMLModelDisplayNode* mrmlModelDisplayNode()const;
  vtkMRMLNode* mrmlDisplayableNode()const;

  bool visibility()const;
  bool clipping()const;
  bool sliceIntersectionVisible()const;
  int sliceIntersectionThickness()const;
  double sliceIntersectionOpacity()const;
  bool clippingConfigurationButtonVisible()const;

  bool scalarsVisibility()const;
  QString activeScalarName()const;
  vtkMRMLColorNode* scalarsColorNode()const;

  enum ControlMode
  {
    Data = 0,
    LUT = 1,
    DataType = 2,
    Manual = 3,
    DirectMapping = 4
  };

  /// Set scalar range mode
  void setScalarRangeMode(ControlMode controlMode);
  ControlMode scalarRangeMode() const;

  /// Get minimum of the scalar display range
  double minimumValue()const;

  /// Get maximum of the scalar display range
  double maximumValue()const;

signals:
  ///
  /// Signal sent if the min/max value is updated
  void minMaxValuesChanged(double min, double max);
  ///
  /// Signal sent if the auto/manual value is updated
  void scalarRangeModeValueChanged(ControlMode value);
  ///
  /// Signal sent if the any property in the display node is changed
  void displayNodeChanged();
  ///
  /// Signal sent if user toggles clipping checkbox on the GUI
  void clippingToggled(bool);
  ///
  /// Signal sent if clipping configuration button is clicked
  void clippingConfigurationButtonClicked();

public slots:
  /// Set the volume node to display
  void setMRMLModelDisplayNode(vtkMRMLModelDisplayNode *node);
  /// Utility function to be connected with generic signals
  void setMRMLModelDisplayNode(vtkMRMLNode *node);
  /// Utility function to be connected with generic signals,
  /// it internally shows the 1st display node.
  /// can be set from a model node or a model hierarchy node
  void setMRMLModelOrHierarchyNode(vtkMRMLNode* modelNode);

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
  void setOpacity(double);
  void setEdgeVisibility(bool);
  void setEdgeColor(const QColor&);
  void setLighting(bool);
  void setInterpolation(int);

  void setScalarsVisibility(bool);
  void setActiveScalarName(const QString&);
  void setScalarsColorNode(vtkMRMLNode*);
  void setScalarsColorNode(vtkMRMLColorNode*);
  void setScalarsDisplayRange(double min, double max);
  void setTresholdEnabled(bool b);
  void setThresholdRange(double min, double max);

  /// Set Auto/Manual mode
  void setScalarRangeMode(int scalarRangeMode);

  /// Set min/max of scalar range
  void setMinimumValue(double min);
  void setMaximumValue(double max);

  /// Show/hide "Configure..." button for clipping
  void setClippingConfigurationButtonVisible(bool);

protected slots:
  void updateWidgetFromMRML();
  void updateNodeFromProperty();
  vtkMRMLSelectionNode* getSelectionNode(vtkMRMLScene *mrmlScene);

protected:
  QScopedPointer<qMRMLModelDisplayNodeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLModelDisplayNodeWidget);
  Q_DISABLE_COPY(qMRMLModelDisplayNodeWidget);
};

#endif
