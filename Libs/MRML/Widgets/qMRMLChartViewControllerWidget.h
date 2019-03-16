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

#ifndef __qMRMLChartViewControllerWidget_h
#define __qMRMLChartViewControllerWidget_h

// CTK includes
#include <ctkVTKObject.h>

// qMRMLWidget includes
#include "qMRMLViewControllerBar.h"
class qMRMLChartViewControllerWidgetPrivate;
class qMRMLChartView;

// MRML includes
class vtkMRMLChartViewNode;

///
/// qMRMLChartViewControllerWidget offers controls to a chart view
/// (vtkMRMLChartViewNode and vtkMRMLChartNode). This controller
/// allows for the content (data) and style (properties) of a chart to
/// be defined.
class QMRML_WIDGETS_EXPORT qMRMLChartViewControllerWidget
  : public qMRMLViewControllerBar
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Superclass typedef
  typedef qMRMLViewControllerBar Superclass;

  /// Constructors
  explicit qMRMLChartViewControllerWidget(QWidget* parent = nullptr);
  ~qMRMLChartViewControllerWidget() override;

  /// Set the label for the chart view (abbreviation for the view
  /// name)
  void setViewLabel(const QString& newViewLabel);

  /// Get the label for the view (abbreviation for the view name)
  QString viewLabel()const;

public slots:
  /// Set the scene
  void setMRMLScene(vtkMRMLScene* newScene) override;

  /// Set the ChartView with which this controller interacts
  void setChartView(qMRMLChartView* ChartView);

  /// Set the ChartViewNode associated with this ChartViewController.
  /// ChartViewNodes are 1-to-1 with ChartViews
  void setMRMLChartViewNode(vtkMRMLChartViewNode* chartViewNode);

  /// Control whether lines are displayed
  void showLines(bool show);

  /// Control whether point markers are displayed
  void showMarkers(bool show);

  /// Control the display of a grid in the chart
  void showGrid(bool show);

  /// Control the display of the legend in the chart
  void showLegend(bool show);

  /// Control the display of the title for the chart
  /// \sa setTitle
  void showTitle(bool show);

  /// Conrtol the display of a label along the X-Axis
  /// \sa setXAxisLabel
  void showXAxisLabel(bool show);

  /// Control the display of a label along the Y-AXis
  /// \sa setYAxisLabel
  void showYAxisLabel(bool show);

  /// Set the title.
  /// \sa showTitle
  void setTitle(const QString&);

  /// Set the label along the X-Axis.
  /// \sa showXAxisLabel
  void setXAxisLabel(const QString&);

  /// Set the label along the Y-Axis.
  /// \sa showYAxisLabel
  void setYAxisLabel(const QString&);

  /// Show a popup to edit the title
  void editTitle();

  /// Show a popup to edit the label along the X-Axis
  void editXAxisLabel();

  /// Show a popup to edit the label along the Y-Axis
  void editYAxisLabel();

protected slots:
  void updateWidgetFromMRML();

private:
  Q_DECLARE_PRIVATE(qMRMLChartViewControllerWidget);
  Q_DISABLE_COPY(qMRMLChartViewControllerWidget);
};

#endif
