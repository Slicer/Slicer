/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkAxesWidget.h>
#include <ctkVTKObject.h>

// qMRMLWidget includes
#include "qMRMLWidgetsExport.h"
class qMRMLThreeDViewControllerWidgetPrivate;
class qMRMLThreeDView;

// MRML includes
class vtkMRMLViewNode;

class QMRML_WIDGETS_EXPORT qMRMLThreeDViewControllerWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Constructors
  explicit qMRMLThreeDViewControllerWidget(QWidget* parent = 0);
  virtual ~qMRMLThreeDViewControllerWidget();

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

  void setLightBlueBackground();
  void setBlackBackground();
  void setWhiteBackground();
  void setBackgroundColor(const QColor& color);
  void setStereoType(int newStereoType);

protected slots:
  void updateWidgetFromMRML();

protected:
  QScopedPointer<qMRMLThreeDViewControllerWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLThreeDViewControllerWidget);
  Q_DISABLE_COPY(qMRMLThreeDViewControllerWidget);
};

#endif
