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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLThreeDViewControllerWidget_h
#define __qMRMLThreeDViewControllerWidget_h

// CTK includes
#include <ctkAxesWidget.h>
#include <ctkVTKObject.h>

// qMRMLWidget includes
#include "qMRMLViewControllerBar.h"
class qMRMLThreeDViewControllerWidgetPrivate;
class qMRMLThreeDView;

// MRML includes

// MRMLLogic includes
class vtkMRMLViewLogic;
class vtkMRMLViewNode;

// VTK includes

class QMRML_WIDGETS_EXPORT qMRMLThreeDViewControllerWidget : public qMRMLViewControllerBar
{
  Q_OBJECT
  QVTK_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLViewControllerBar Superclass;

  /// Constructors
  explicit qMRMLThreeDViewControllerWidget(QWidget* parent = nullptr);
  ~qMRMLThreeDViewControllerWidget() override;

  void setQuadBufferStereoSupportEnabled(bool value);

  /// Get ViewLogic
  vtkMRMLViewLogic* viewLogic() const;

  /// Set \a newViewLogic
  /// Use if two instances of the controller need to observe the same logic.
  void setViewLogic(vtkMRMLViewLogic* newViewLogic);

  /// Set the label for the table view (abbreviation for the view name)
  void setViewLabel(const QString& newViewLabel);

  /// Get the label for the view (abbreviation for the view name)
  QString viewLabel() const;

  /// Get 3D view node associated with this ThreeDViewController.
  Q_INVOKABLE vtkMRMLViewNode* mrmlThreeDViewNode() const;

public slots:

  void setMRMLScene(vtkMRMLScene* newScene) override;

  void setThreeDView(qMRMLThreeDView* threeDView);

  /// Link/Unlink the view controls and the cameras across all views
  void setViewLink(bool linked);

  void setOrthographicModeEnabled(bool enabled);

  void lookFromAxis(const ctkAxesWidget::Axis& axis);
  void pitchView();
  void rollView();
  void yawView();
  void zoomIn();
  void zoomOut();
  void spinView(bool enabled);
  void rockView(bool enabled);
  void setAnimationMode(int newAnimationMode);

  void resetFocalPoint();
  void set3DAxisVisible(bool visible);
  void set3DAxisLabelVisible(bool visible);

  /// Use or not depth peeling in the first renderer.
  /// False by default.
  void setUseDepthPeeling(bool use);
  /// Show or hide the FPS in the lower right corner.
  /// False by default.
  void setFPSVisible(bool visible);

  /// Utility function to change the color of the background to blue
  void setLightBlueBackground();

  /// Utility function to change the color of the background to black
  void setBlackBackground();

  /// Utility function to change the color of the background to white
  void setWhiteBackground();

  /// If the second color is not set, the first color is used.
  void setBackgroundColor(const QColor& color, QColor color2 = QColor());

  /// Utility function to change color of the box
  void setBoxColor(const QColor& color);

  void setStereoType(int newStereoType);
  void setOrientationMarkerType(int type);
  void setOrientationMarkerSize(int size);
  void setRulerType(int type);
  void setRulerColor(int color);

  void setShadowsVisibility(bool visibility);
  void setAmbientShadowsSizeScale(double value);
  void setAmbientShadowsVolumeOpacityThresholdPercent(double opacityPercent);

protected slots:
  void updateWidgetFromMRMLViewLogic();
  void updateWidgetFromMRMLView() override;
  void updateViewFromMRMLCamera();

protected:
  void setMRMLViewNode(vtkMRMLAbstractViewNode* viewNode) override;

private:
  Q_DECLARE_PRIVATE(qMRMLThreeDViewControllerWidget);
  Q_DISABLE_COPY(qMRMLThreeDViewControllerWidget);
};

#endif
