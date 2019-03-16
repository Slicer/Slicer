/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qMRMLChartWidget_h
#define __qMRMLChartWidget_h

// Qt includes
#include <QWidget>
class QResizeEvent;

// qMRMLWidget includes
#include "qMRMLWidget.h"
class qMRMLChartViewControllerWidget;
class qMRMLChartView;
class qMRMLChartWidgetPrivate;

// MRML includes
class vtkMRMLChartViewNode;
class vtkMRMLColorLogic;
class vtkMRMLScene;

/// \brief qMRMLChartWidget is the toplevel charting widget that can be
/// packed in a layout.
///
/// qMRMLChartWidget provides charting capabilities with a display
/// canvas for the chart and a controller widget to control the
/// content and properties of the chart.
class QMRML_WIDGETS_EXPORT qMRMLChartWidget : public qMRMLWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;

  /// Constructors
  explicit qMRMLChartWidget(QWidget* parent = nullptr);
  ~qMRMLChartWidget() override;

  /// Get the chart node observed by view.
  vtkMRMLChartViewNode* mrmlChartViewNode()const;

  /// Get a reference to the underlying Chart View
  /// Becareful if you change the ChartView, you might
  /// unsynchronize the view from the nodes/logics.
  Q_INVOKABLE qMRMLChartView* chartView()const;

  /// Get the view label for the chart.
  /// \sa qMRMLChartControllerWidget::chartViewLabel()
  /// \sa setChartViewLabel()
  QString viewLabel()const;

  /// Set the view label for the chart.
  /// \sa qMRMLChartControllerWidget::chartViewLabel()
  /// \sa chartViewLabel()
  void setViewLabel(const QString& newChartViewLabel);

  /// Set the color logic that is used by the view.
  void setColorLogic(vtkMRMLColorLogic* colorLogic);
  vtkMRMLColorLogic* colorLogic()const;

public slots:
  /// Set the current \a viewNode to observe
  void setMRMLChartViewNode(vtkMRMLChartViewNode* newChartViewNode);

protected slots:
  void onAppAboutToQuit();

protected:
  QScopedPointer<qMRMLChartWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLChartWidget);
  Q_DISABLE_COPY(qMRMLChartWidget);
};

#endif
