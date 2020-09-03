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

==============================================================================*/

#ifndef __qMRMLMarkupsDisplayNodeWidget_h
#define __qMRMLMarkupsDisplayNodeWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// qMRML includes
#include "qSlicerMarkupsModuleWidgetsExport.h"

class qMRMLMarkupsDisplayNodeWidgetPrivate;
class vtkMRMLScene;
class vtkMRMLNode;
class vtkMRMLMarkupsDisplayNode;
class vtkMRMLMarkupsNode;
class vtkMRMLColorNode;
class vtkMRMLSelectionNode;

class Q_SLICER_MODULE_MARKUPS_WIDGETS_EXPORT qMRMLMarkupsDisplayNodeWidget : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

/*
  Q_PROPERTY(ControlMode scalarRangeMode READ scalarRangeMode WRITE setScalarRangeMode)
  Q_PROPERTY(double minimumValue READ minimumValue WRITE setMinimumValue)
  Q_PROPERTY(double maximumValue READ maximumValue WRITE setMaximumValue)
  Q_PROPERTY(bool clippingConfigurationButtonVisible READ clippingConfigurationButtonVisible WRITE setClippingConfigurationButtonVisible)
  */
public:

  qMRMLMarkupsDisplayNodeWidget(QWidget *parent=nullptr);
  ~qMRMLMarkupsDisplayNodeWidget() override;

  vtkMRMLMarkupsDisplayNode* mrmlMarkupsDisplayNode()const;

  bool visibility()const;

  bool glyphSizeIsAbsolute()const;
  bool curveLineSizeIsAbsolute()const;

  bool pointLabelsVisibility()const;

signals:
  ///
  /// Signal sent if the any property in the display node is changed
  void displayNodeChanged();

public slots:
  /// Set the markups display node to show edit properties of
  void setMRMLMarkupsDisplayNode(vtkMRMLMarkupsDisplayNode *node);
  /// Utility function to be connected with generic signals
  void setMRMLMarkupsDisplayNode(vtkMRMLNode *node);

  /// Set the markups display node to show edit properties of,
  /// by specifying markups node.
  void setMRMLMarkupsNode(vtkMRMLMarkupsNode* node);
  /// Utility function to be connected with generic signals
  void setMRMLMarkupsNode(vtkMRMLNode* node);

  void setVisibility(bool);

  void setGlyphSizeIsAbsolute(bool absolute);
  void setCurveLineSizeIsAbsolute(bool absolute);

  void setPointLabelsVisibility(bool);

  void setMaximumMarkupsScale(double maxScale);
  void setMaximumMarkupsSize(double maxScale);

  void setFillVisibility(bool visibility);
  void setOutlineVisibility(bool visibility);
  void onFillOpacitySliderWidgetChanged(double opacity);
  void onOutlineOpacitySliderWidgetChanged(double opacity);
  void setOccludedVisibility(bool visibility);
  void setOccludedOpacity(double OccludedOpacity);

protected slots:
  void updateWidgetFromMRML();
  vtkMRMLSelectionNode* getSelectionNode(vtkMRMLScene *mrmlScene);

  void onSelectedColorPickerButtonChanged(QColor qcolor);
  void onUnselectedColorPickerButtonChanged(QColor qcolor);
  void onGlyphTypeComboBoxChanged(QString value);
  void onGlyphScaleSliderWidgetChanged(double value);
  void onGlyphSizeSliderWidgetChanged(double value);
  void onCurveLineThicknessSliderWidgetChanged(double percentValue);
  void onCurveLineDiameterSliderWidgetChanged(double value);
  void onTextScaleSliderWidgetChanged(double value);
  void onOpacitySliderWidgetChanged(double value);

  void setTextProperty();

  void onInteractionCheckBoxChanged(int state);

protected:
  QScopedPointer<qMRMLMarkupsDisplayNodeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLMarkupsDisplayNodeWidget);
  Q_DISABLE_COPY(qMRMLMarkupsDisplayNodeWidget);
};

#endif
