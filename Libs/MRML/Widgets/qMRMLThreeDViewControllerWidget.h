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

class QMRML_WIDGETS_EXPORT qMRMLThreeDViewControllerWidget
  : public qMRMLViewControllerBar
{
  Q_OBJECT
  QVTK_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLViewControllerBar Superclass;

  /// Constructors
  explicit qMRMLThreeDViewControllerWidget(QWidget* parent = 0);
  virtual ~qMRMLThreeDViewControllerWidget();

  /// Set the label for the 3D view (abbreviation for the view
  /// name)
  void setViewLabel(const QString& newViewLabel);

  /// Get the label for the view (abbreviation for the view name)
  QString viewLabel()const;

  void setQuadBufferStereoSupportEnabled(bool value);

public slots:
  void setThreeDView(qMRMLThreeDView* threeDView);
  void setMRMLViewNode(vtkMRMLViewNode* viewNode);

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

protected slots:
  void updateWidgetFromMRML();

private:
  Q_DECLARE_PRIVATE(qMRMLThreeDViewControllerWidget);
  Q_DISABLE_COPY(qMRMLThreeDViewControllerWidget);
};

#endif
