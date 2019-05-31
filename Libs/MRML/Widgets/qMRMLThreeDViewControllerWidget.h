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
class vtkMRMLViewNode;

// MRMLLogic includes
class vtkMRMLViewLogic;

// VTK includes
class vtkCollection;

class QMRML_WIDGETS_EXPORT qMRMLThreeDViewControllerWidget
  : public qMRMLViewControllerBar
{
  Q_OBJECT
  QVTK_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLViewControllerBar Superclass;

  /// Constructors
  explicit qMRMLThreeDViewControllerWidget(QWidget* parent = nullptr);
  ~qMRMLThreeDViewControllerWidget() override;

  /// Set the label for the 3D view (abbreviation for the view
  /// name)
  void setViewLabel(const QString& newViewLabel);

  /// Get the label for the view (abbreviation for the view name)
  QString viewLabel()const;

  /// Set the color for the view
  void setViewColor(const QColor& newViewColor);

  /// Get the color for the view
  QColor viewColor()const;

  void setQuadBufferStereoSupportEnabled(bool value);

  /// Get ViewLogic
  vtkMRMLViewLogic* viewLogic()const;

  /// Set \a newViewLogic
  /// Use if two instances of the controller need to observe the same logic.
  void setViewLogic(vtkMRMLViewLogic* newViewLogic);

  /// TODO:
  /// Ideally the view logics should be retrieved by the viewLogic
  /// until then, we manually set them.
  void setViewLogics(vtkCollection* logics);

public slots:

  void setMRMLScene(vtkMRMLScene* newScene) override;

  void setThreeDView(qMRMLThreeDView* threeDView);
  void setMRMLViewNode(vtkMRMLViewNode* viewNode);

  /// Link/Unlink the view controls and the cameras across all viewes
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
  void setBackgroundColor(const QColor& color,
                          QColor color2 = QColor());

  void setStereoType(int newStereoType);
  void setOrientationMarkerType(int type);
  void setOrientationMarkerSize(int size);
  void setRulerType(int type);
  void setRulerColor(int color);

protected slots:
  void updateWidgetFromMRMLView();
  void updateViewFromMRMLCamera();

private:
  Q_DECLARE_PRIVATE(qMRMLThreeDViewControllerWidget);
  Q_DISABLE_COPY(qMRMLThreeDViewControllerWidget);
};

#endif
